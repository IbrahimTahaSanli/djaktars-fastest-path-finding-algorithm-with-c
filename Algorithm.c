//stdio header used for standart input output jobs 
#include <stdio.h>

//stdarg used at one function irrelevant from all work
#include <stdarg.h>

//stdlib header used for memory allocation
#include <stdlib.h>

//Indicates how much nodes in this file for general information
#define NODE_COUNT 5
//Quick fix for an error. I explained it where it makes sense
#define NODES_EDGE_COUNT 40

//For 3th question. Starts nodes ID
#define START_NODE 0
//For 3th question. Final nodes ID
#define FINAL_NODE 4

//Node Files
#define NODE_FILE "example_graph_nodes.csv"
//Edge Files
#define EDGE_FILE "example_graph_edges.csv"


//This signature is that function used with stdarg and it is irrelevant. I explained it in function 
int readSTR(FILE*, char**, ...);
//I dont know is compiler add all header or just function i use so i didnt include all header and write string comparer which i guess similar from header.
int strComp(char*, char*);

//Signatures for functions. I explained it below why they are here.
int dijkstraAlgo(struct Node*, struct Node*);
void step(struct Path*, struct Node*, int);

//Same
void stepDia(struct Path*, struct Node*, int);
int myAlgo(struct Node*, struct Node*);


//Garbage pointer
void* _;

//Global variables
//nodeList is array of Node*. All nodes we use stored here for easy access
struct Node* nodeList[NODE_COUNT];

//distance is array of int. Used for storeing distances between nodes.
int distance[NODE_COUNT];
//distance is array of char(i used it as boolean array). For indicate nodes passed before.
char nodesPassed[NODE_COUNT];

//FastetPath(There is a typo Sorry) is Path*. Variable for storing fastest path.
struct Path* FastetPath;
//CurrentPath is Path* for indicate current path we working on.
struct Path* CurrentPath;


//Structer for Nodes. Basic Node for storing nodes.
typedef struct Node {
	//ID: Unique, int for Identifying
	int ID;
	//Name: Unique, char*(Used as string) for represent better
	char* Name;

	//Edges: Edge** for storing Edges where it is starts
	struct Edge** Edges;
	//EdgesCount: int for storing EdgeCount in Edges pointer
	int EdgesCount;
};

//Function for adding easly edge to node
//This is where i need to NODES_EDGE_COUNT. At start i use dynamic memory allocation but it was throwing an error. I ran away with hard fix. 
//Node* node: indicating where edge adding to
//Edge* edge: which edge going to add
void addEdgeToNode(struct Node* node, struct Edge* edge) {
	// I leave this line for maybe later i can fix.
	//	node->Edges = (struct Edge**)realloc(node->Edges,++node->EdgesCount);
	
	//Adding edge to nodes Edges field. Then increment EdgesCount.
	node->Edges[node->EdgesCount++] = edge;
}

//For debug purpose. Printing all nodes in nodeList.
void printAllNodes() {
	//Loop around all nodes
	for (int i = 0; i < NODE_COUNT; i++)
		//Printing nodes fields with style
		printf("%i\t->%s\t->\t%i\n", nodeList[i]->ID, nodeList[i]->Name, nodeList[i]->EdgesCount);
}



//Path structer used in purpose of storing nodes between start and end
//Its basicly a one-way linked list
typedef struct Path {
	//Cuurent node
	struct Node* node;
	//Next path object for linked list
	struct Path* next;
};

//For adding node to path
//Its add directly to paths next field can cause memory leak
//Path* path: where node going to add
//Node* node: Which Node going to add
struct Path* addPathNext(struct Path* path, struct Node* node) {
	//Allocate memory to set node
	path->next = (struct Path*)calloc(1 , sizeof(struct Path));
	//Set nodes address to path
	path->next->node = node;
	//return New path
	return path->next;
}

//addPathLast: Add node to directly end of path
//I write it because i think i need it but i neednt. Its a little waste
struct Path* addPathLast(struct Path* path, struct Node* node) {
	while (path->next) path = path->next; return addPathNext(path, node);
}

//copyPath: Make new path linked list from other path and return new path
//Path* path: Which path going to get coppied
struct Path* copyPath(struct Path* path) {
	//Start new path Starts with retVal variable. retVal is Concat(Kinda of) of return and Value so it going to store for end of function
	struct Path* retVal = (struct Path*)malloc(1 * sizeof(struct Path));
	//tmp indicates last element of Path and it initialize with same address of retVal 
	struct Path* tmp = retVal;

	//Seting first node of list
	tmp->node = path->node;

	//Preaparing to copy other nodes
	path = path->next;

	//Loop until path equal to null
	while (path) {
		//If path isnt null add node to new path
		tmp = addPathNext(tmp, path->node);
		//Set path to next element for increment
		path = path->next;
	}

	//return new copied Path
	return retVal;
}

//freePath: Clear/Free path in front of
//Path* path: Which path gointo be cleaned/freed 
int freePath(struct Path* path) {
	//I use algorith i called monkey climb(I dont know real term) for linked list
	//Path* tmp for storing next pointer
	struct Path* tmp = 0;
	
	//if tmp(Which is next pointer/current path) or path(current path/Which is next pointer) gets true 
	//else current and next pointer gets null meaning all linked list cleared
	while (path || tmp) {
		//if tmp is not null, tmp stores current Path
		if (tmp) {
			//Set path to next so we dont lose all list
			path = tmp->next;
			//Free current Path which is tmp
			free(tmp);
			//Set current Path to null for indicating we freed  
			tmp = 0;
		}
		//if tmp is null, path stores current Path
		else {
			//Set tmp to next so we dont lose all list
			tmp = path->next;
			//Free current Path which is path
			free(path);
			//Set current Path to null for indicating we freed  
			path = 0;
		}
	}

	//Indicates operations is succeed(First i planned that way)
	return 0;
}

//printPath: Print given path
//Path* path: Given path
void printPath(struct Path* path) {
	//Loop until path equels to null. print nodes name with arrow indicates way. Go to next path
	while (path) { printf("%s -> ", path->node->Name); path = path->next; }
}

//countHop: Count how many edges passed on this path
//Path* path: Path needs to count
int countHop(struct Path* path) {
	//Counter of hops
	int i = 0;
	//Loop until path gets null. Increment counter get next path
	while (path) { i++; path = path->next; }
	//return decrement of counter because of we need count of edges not nodes
	return i-1;
}


//edgeType: Indicates edges type better than string
enum edgeType {
	Undirected,
	Directed
};

//Edge: Infinite point between two nodes
//All edges is Directed
//Undirected edges is two edges with same weight with opposite source and target
//It seems like memory waste. It is but less than duplicated Edges. Every reference is same if isnt need to. Duplicate Waste is for N Node like 17 * N byte
typedef struct Edge {
	//ID: Unique, int for Identifying
	int ID;

	//Source: Start of edge
	struct Node* Source;
	//Target: End of edge
	struct Node* Target;

	//type: directed or undirected
	enum edgeType type;
	//Label: ?
	char* Label;

	//TimeSet: ?
	int TimeSet;
	//Weight: int for storing weight of edge
	int Weight;
};




//main
int main() {
	_ = malloc(1);

	//FILE* fptr: for file operations
	FILE* fptr= 0;
	//Get address of nodes file
	fptr = fopen( NODE_FILE, "r");

	//Read first row where is headers
	readSTR(fptr, "%s,%s\n",_,_);

	//Initiliaze first node nulled 
	struct Node* node = (struct Node*)calloc(1,sizeof(struct Node));

	//Loop until End Of File. From file read a number write it to nodes ID field after comma read string write it to name then jump over new line start loop 
	while (EOF != readSTR(fptr, "%n,%s\n", &node->ID, &node->Name)) {
		//Add node to nodeList
		nodeList[node->ID] = node;
		//Start Non dynamic Edges Array
		nodeList[node->ID]->Edges = (struct Edge**)calloc(NODES_EDGE_COUNT, sizeof(struct Edge*));
//	this too waiting later;	node->Edges = (struct Edge**)calloc(1, sizeof(struct Edge*));
		//Now allocate new nodes memory
		node = (struct Node*)calloc(1, sizeof(struct Node));
	}

	//Free last null filled node
	free(node);

	//Close link to nodes file
	fclose(fptr);
	//Link to Edges file
	fptr = fopen(EDGE_FILE, "r");

	//Read first row where is headers
	readSTR(fptr, "%s,%s,%s,%s,%s,%s,%s\n", _, _, _, _, _, _, _);

	//edge to store edge read from file
	struct Edge* edge = (struct Edge*)malloc(1 * sizeof(struct Edge));
	//edge1 to store opposite of edge if edge is undirected
	struct Edge* edge1;
	//Read edge type as string because i cant just read as enum
	char* type;

	//Loop until End Of File. Read from file next edge write it to adressed memory
	//Its read source and target as intager hold it at pointer field it doesnt throw any error because of havent used as pointer
	while (EOF != readSTR(fptr, "%n,%n,%s,%n,%s,%s,%n\n", &edge->Source, &edge->Target, &type, &edge->ID, &edge->Label, &edge->TimeSet, &edge->Weight)) {
		//Add current edge to node with ID of node
		addEdgeToNode(nodeList[(int)(edge->Source)], edge);
		//Set edges source to address of source node
		edge->Source = nodeList[(int)(edge->Source)];
		//Set edges target to address of target node
		edge->Target = nodeList[(int)(edge->Target)];

		//if edge is Undirected as i explained in Edges add opposite edge
		if (!strComp(type, "Undirected")) {
			edge->type = Undirected;

			//Initiliaze next edge1
			edge1 = (struct Edge*)malloc(1 * sizeof(struct Edge));

			//Set edge1 values with same and opposite of edge
			edge1->ID = edge->ID;
			edge1->type = Undirected;
			edge1->Label = edge->Label;
			edge1->Target = edge->Source;
			edge1->Source = edge->Target;
			edge1->TimeSet = edge->TimeSet;
			edge1->Weight = edge->Weight;

			//Add opposite edge to node
			addEdgeToNode(nodeList[edge1->Source->ID], edge1);
		}
		//if edge is Directed
		else {
			//Just set edges type. Theres no need to create new edge
			edge->type = Directed;
		}

		//Free type because it will be unlinked next read
		free(type);

		//Take new edge
		edge = (struct Edge*)malloc(1 * sizeof(struct Edge));
	}

	//Free empty edges
	free(edge);

	//Close file pointer to unnecessary pointer
	fclose(fptr);


	//3 Lines below this comment: To calculate fastest path between points
	//Start of algorith. Calculates fastest path between START_NODE and FINAL_NODE
	dijkstraAlgo(nodeList[START_NODE], nodeList[FINAL_NODE]);

	//Print fastest path with node names
	printPath(FastetPath);

	//distance between START_NODE and FINAL_NODE at fastest path
	printf("%i\n", distance[FINAL_NODE]);


	//Below here to calculate diameter of network or node map
	
	//Because diameter calculations made on non directed node map, i need to find solution to directed maps.
	//The solution i found is adding temporary edges to network
	//in this loop i look around to all nodes
	for (int i = 0; i < NODE_COUNT; i++)
		//Look around all edges in node
		for(int j = 0; j < nodeList[i]->EdgesCount;j++)
			//if node is directed
			if (nodeList[i]->Edges[j]->type == Directed) {
				//we need to make tmp opposite edge to network
				edge1 = (struct Edge*)malloc(1 * sizeof(struct Edge));

				//for indicating this is tmp i created edges with negative ID				
				edge1->ID = -nodeList[i]->Edges[j]->ID;
				//Other lines same with undirected edge creation
				edge1->type = Directed;
				edge1->Label = nodeList[i]->Edges[j]->Label;
				edge1->Target = nodeList[i]->Edges[j]->Source;
				edge1->Source = nodeList[i]->Edges[j]->Target;
				edge1->TimeSet = nodeList[i]->Edges[j]->TimeSet;
				edge1->Weight = nodeList[i]->Edges[j]->Weight;

				addEdgeToNode(nodeList[edge1->Source->ID], edge1);
			}
	
	//tmp
	int tmp;

	//
	int diameterHop = 0;

	myAlgo(nodeList[START_NODE], nodeList[FINAL_NODE]);

	for (int i = 0; i < NODE_COUNT; i++)
		printf("%s->%i\n", nodeList[i]->Name, distance[i]);

	for(int i =0; i < NODE_COUNT; i++)
		for (int j = i+1; j < NODE_COUNT; j++) {
			myAlgo(nodeList[i], nodeList[j]);
			tmp = countHop(FastetPath);

			if (tmp > diameterHop) 
				diameterHop = tmp;
		}

	printf("%i", diameterHop);

	return 0;
}


int myAlgo(struct Node* from, struct Node* to) {
	for (int i = 0; i < NODE_COUNT; i++)
		distance[i] = INT_MAX;

	distance[from->ID] = 0;

	for (int i = 0; i < NODE_COUNT; i++)
		nodesPassed[i] = 1;

	CurrentPath = (struct Path*)calloc(1, sizeof(struct Path));
	CurrentPath->node = from;

	nodesPassed[from->ID] = 0;
	nodesPassed[to->ID] = 0;

	freePath(FastetPath);
	FastetPath = 0;

	stepDia(CurrentPath, to, 0);

	return 0;
}

void stepDia(struct Path* path, struct Node* FinalNode, int hop) {
	for (int i = 0; i < path->node->EdgesCount; i++) {
		if (!path->node->Edges[i]) continue;

		if (hop + 1 < distance[path->node->Edges[i]->Target->ID]) {
			distance[path->node->Edges[i]->Target->ID] = hop + 1;
			nodesPassed[path->node->Edges[i]->Target->ID] = 1;

			if (path->node->Edges[i]->Target->ID == FinalNode->ID) {
				addPathNext(path, path->node->Edges[i]->Target);

				freePath(FastetPath);
				FastetPath = copyPath(CurrentPath);

				freePath(path->next);
				path->next = 0;
			}
		}

		if (nodesPassed[path->node->Edges[i]->Target->ID]) {
			addPathNext(path, path->node->Edges[i]->Target);

			nodesPassed[path->next->node->ID] = 0;

			stepDia(path->next, FinalNode, distance[path->node->Edges[i]->Target->ID]);

			freePath(path->next);
			path->next = 0;
		}
	}
}


int dijkstraAlgo(struct Node* from, struct Node* to){
	for (int i = 0; i < NODE_COUNT; i++) 
		distance[i] = INT_MAX;
	
	distance[from->ID] = 0;

	for (int i = 0; i < NODE_COUNT; i++)
		nodesPassed[i] = 1;

	CurrentPath = (struct Path*)calloc(1 , sizeof(struct Path));
	CurrentPath->node = from;

	nodesPassed[from->ID] = 0;
	nodesPassed[to->ID] = 0;

	freePath(FastetPath);
	FastetPath = 0;

	step(CurrentPath, to, 0);
	
	return 0;
}

void step(struct Path* path, struct Node* FinalNode, int dist) {
	for (int i = 0; i < path->node->EdgesCount; i++) {
		if (!path->node->Edges[i]) continue;

		if (dist + path->node->Edges[i]->Weight < distance[path->node->Edges[i]->Target->ID]) {
			distance[path->node->Edges[i]->Target->ID] = dist + path->node->Edges[i]->Weight;

			nodesPassed[path->node->Edges[i]->Target->ID] = 1;

			if (path->node->Edges[i]->Target->ID == FinalNode->ID) {
				addPathNext(path, path->node->Edges[i]->Target);

				freePath(FastetPath);
				FastetPath = copyPath(CurrentPath);

				freePath(path->next);
				path->next = 0;
			}
		}

		if (nodesPassed[path->node->Edges[i]->Target->ID]) {
			addPathNext(path, path->node->Edges[i]->Target);
			
			nodesPassed[path->next->node->ID] = 0;

			step(path->next, FinalNode, distance[path->node->Edges[i]->Target->ID]);

			freePath(path->next);
			path->next = 0;
		}
	}
}


int strToInt(char* str) {
	int i = 0;
	int retVal = 0;

	while (str[i]) {
		retVal *= 10;

		retVal += str[i] - 48;

		i++;
	}

	return retVal;

}

int strComp(char* ch1, char* ch2) {
	while (*ch1 && *ch2) {
		if (*ch1 - *ch2)
			return *ch1 - *ch2;
		ch1++; ch2++;
	}
	return *ch1 || *ch2;
}

char peek(FILE* fp) {
	const int c = getc(fp);
	return c == EOF ? EOF : ungetc(c, fp);
}

int readSTR(FILE* fptr, const char* template, ...) {
	va_list list;

	va_start(list, template);

	void* ptr;

	char tmp;
	int size;

	int retVal=0;

	int i = 0;

	while (template[i]) {
		if (template[i] == '%') {
			i++;
			switch (template[i]) {
			case('n'):
				i++;
				size = 1;

				ptr = calloc(size, sizeof(char));

				while (1) {
					tmp = fgetc(fptr);
					if (template[i] == tmp) {
						i++;
						break;
					}
					else if (tmp == EOF) {
						retVal = EOF;
						i++;
						goto readSTR_N;
					}
					ptr = realloc(ptr, ++size * sizeof(char));

					((char*)ptr)[size - 2] = tmp;
				}
				
				readSTR_N:

				((char*)ptr)[size - 1] = '\0';


				*(va_arg(list, int*)) = strToInt(ptr);

				break;

			case('s'):
				i++;
				size = 1;

				
				ptr = calloc(size,sizeof(char));
				
				while (1) {
					tmp = fgetc(fptr);
					if (template[i] == tmp) {
						i++;
						break;
					}
					else if (tmp == EOF) {
						retVal = EOF;
						i++;
						goto readSTR_S;
					}
					ptr = realloc(ptr, ++size * sizeof(char));

					((char*)ptr)[size - 2] = tmp;
				}

				((char*)ptr)[size - 1] = '\0';
				readSTR_S:
				*(va_arg(list, char**)) = ptr ;

				break;
			}
		}

	}

	va_end(list);

	return retVal;
}