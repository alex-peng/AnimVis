#pragma once
#include <string>
#include<set>
#include<math.h>
using namespace std;

#define SIZE_GRID 50
#define MAX_DEPTH 2
#define SMALL_CIRTERION 2
#define TREE_CIRTERION 2

#define  GIRD_SIZE 30
#define  STEP 4
#define  DISTANCE ((float)70.0) 

#define  FORCE ((float)1400.0)
 
#define CLIQUE_SIZE 4



struct AABB
{
	float maxPos[2];
	float minPos[2];
};

struct grid
{
	int id;
	int numPoints;
	int begin;
};
struct leaf
{
	AABB aabb;
	int numPoints;
	int begin;
};

struct Parameter
{
	float attExponent;
	float graFactor;
	float repFactor;
	float repExponent;
	int numNode;
	int insideNodeNum;
	int leafNodeNum;
};


struct Edge_Node
 {
 public:
	 int node;
	 float weight; 
	 int edge_id;
 };


 class Node
 {
 public:
	 string name;
	 float weight;
	 float pos[2];
//	 float velocity[3];
	 
	 Edge_Node *edge;
	 int numEdge;
	 int begin;
	 
	 
	 Node(void)
	 {
//		 name ="";
		 weight =0.0;
		 pos[0] = pos[1] = 0.0;//pos[2] =0.0;
//		 velocity[0] = velocity[1] = velocity[2] =0.0;
		 numEdge =0; 
	 }
 };
 
 struct NodeN
 {
	 char name[50];
	 float weight;
	 float pos[2];
//	 float velocity[3];
	 int index;
	 int numEdge;
	 int begin;
	 bool stable;
	 int gridId;

	 //need initialize;
	 int id;
	 float gval;
	 float hval;
	 float fval;
	 bool visited;
	 bool in_open;
	 bool in_close;
 };

 struct Circle
 {
	 float dir;
	 float pos[2];
 };
 

struct Edge
{
	int node1;
	int node2;
//	int subgraph;
	float weight;
//	float density;
//	string name;
	string str;
	int kind;
};

struct QuaTree
{
	AABB aabb;
	float weight;
	float pos[2];
	int childCount;
	NodeN* node;
	QuaTree* child[4];
	QuaTree* parent;
	int depth;
	bool isLeaf;
	int num_nodes;
	int* list;
	int creatID;
};

struct TreeNodeInside
{
	float children[4];
	bool isLeaf;
	float pos[2];
	float weight;
	float width;
	int numNode;
	int begin;
	int depth;
	int parent;
	int order;
	int childNum;
	int brother;
};

struct TreeLeafNode
{
	float weight;
	float pos[2];
	int numNode;
	int begin;
};

class Graph
{
public:
	AABB aabb;
	Node* node;
	NodeN* noden;
	NodeN* new_node;
	Edge* edge;
	bool loadFile(const char* fileName);
	bool loadFile2(const char* fileName);
	bool loadFile3(const char* fileName);
	bool loadFile_2(const char *fileName);
	bool loadFile_3(const char *fileName);
	int findNode(string name);
	Graph(void);
	~Graph(void);
	int numNode;
	int numEdge;
	int *old_edge_order;
	int *new_pos;

	QuaTree* tree_root;
	TreeNodeInside *treeInside;
	//TreeLeafNode* treeLeafNode;
	int* referenceNode;

	int insideNodeNum;
	int leafNodeNum;
	int sizeofLeafNode;

	int createIdTree;


	grid* grid_root;
	int* order_node;

	Edge_Node* edge_node;
	Edge_Node* edge_node2;
	int* box;

	Parameter param;

	void posInitia();
	void preprocess(const char* fileName);
	void BuildTree();
	void BuildTree2(QuaTree* parent, int index, int* nodelist, int num_Nodes);
	void GetAABB();
	void PrepareGPU();
	void PrepareGPU2(int parentID,QuaTree* current,int& insideIndex, int index, int brother);
	int  TestTraverseTree();
	float Distance(int id1, int id2);
	void Cleanup();
	void GridBuild();

	int* path;
	int* clique;
	int clique_num;
	int path_num;
	int clique_size_i;
};


 
 