#include "Graph.h"
#include<iostream>
#include<fstream>
#include<string>
#include<time.h>
#include<stdio.h>
#include <stdlib.h>

using namespace std;


int compare (const void * a, const void * b)
{
	if( *(float*)a - *(float*)b >=0.0 )
		return 1;
	else
		return -1;
}

Graph::Graph(void)
{
	insideNodeNum =0;
	leafNodeNum =0;
	sizeofLeafNode=0;

	treeInside=0;
	referenceNode=0;
	createIdTree = 0;

	grid_root =0;
	clique_size_i = CLIQUE_SIZE;

}

Graph::~Graph(void)
{

 
	delete [] edge;
	delete [] noden;
	delete [] edge_node;
//	delete [] grid_root;
//	delete [] order_node;
//	delete [] path;
//	delete [] clique;
	
}
void Graph::preprocess(const char* fileName)
{
	ofstream ofile("test.txt");
	ifstream file(fileName);
	string str;
	getline(file,str,'\n');
	while(file.good())
	{
		getline(file,str,'\t');
		if(str =="")
			break;
		getline(file,str,'\t'); 
		ofile<<str<<'\t';
		//ofile<<str<<'\t';

		getline(file,str,'\t');
		ofile<<str<<'\t';

		getline(file,str,'\t');
		getline(file,str,'\n');

		ofile<<str<<'\n';

		//ofile<<str<<'\n';

		// file.ignore(1000000,'\n');

	}
	file.clear();
	file.seekg(0,ios::beg);
	file.close();
	ofile.close();
}

bool Graph::loadFile(const char *fileName)
{
	ifstream file(fileName);
	//ofstream ofile("output.txt");
	string str;
	set<string> string_node;
	set<string>::iterator it; 

	//	getline(file,str,'\n');;// skip the first line;
	numEdge =0;

//	getline(file,str,'\n');
	while(file.good())
	{
		numEdge++;
//		getline(file,str,'\t');
		getline(file,str,'\t'); 
		if(str =="")
			break;
		string_node.insert(str);
		getline(file,str,'\t');
		string_node.insert(str);
		getline(file,str,'\n');
	}
	numEdge--;
	//store the node information to a array of node
	int index_node=0;
	numNode = string_node.size();
	node = new Node[numNode]();
	for(it = string_node.begin();it!=string_node.end();it++)
		node[index_node++].name =(*it);

	file.clear();
	file.seekg(0,ios::beg);
	//	file.ignore(10000,'\n');// skip the first line;

	edge = new Edge[numEdge];
	int index_edge =0;
	int id;
	int index_node2 =0; 
//	getline(file,str,'\n'); 
	while(file.good())
	{	 
//		getline(file,str,'\t');
		getline(file,str,'\t');

		index_node = findNode(str);
		if(index_node == -1)
			break;
		else
		{
			edge[index_edge].node1 = index_node;
			node[index_node].weight++;
			edge[index_edge].weight =1.0;
		}

		getline(file,str,'\t');
		index_node2 = findNode(str);
		if(index_node == -1)
			return false;
		else
		{
			edge[index_edge].node2 = index_node2;
			node[index_node2].weight++; 
		}

		getline(file,str,'\t');
		edge[index_edge].kind = atoi(str.c_str());

		getline(file,str,'\t');

		getline(file,str,'\n');
		edge[index_edge].str = str;
		index_edge++;  
	}
	float density;

	for(int i=0;i<numEdge;i++)
	{
		density = 1.0/(node[edge[i].node1].weight*node[edge[i].node2].weight);

		if(node[edge[i].node1].numEdge == 0)
		{
			node[edge[i].node1].edge = new Edge_Node[(int)(node[edge[i].node1].weight)];
			node[edge[i].node1].numEdge =1;
			node[edge[i].node1].edge[0].node = edge[i].node2;
			node[edge[i].node1].edge[0].weight = 1.0;
			node[edge[i].node1].edge[0].edge_id = i;
			//		 node[edge[i].node1].edge[0].density = density;
		}
		else
		{
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].node = edge[i].node2;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].weight = 1.0;
			//		 node[edge[i].node1].edge[node[edge[i].node1].numEdge].density = density;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].edge_id = i;
			node[edge[i].node1].numEdge++;
		}

		if(node[edge[i].node2].numEdge == 0)
		{
			node[edge[i].node2].edge = new Edge_Node[(int)(node[edge[i].node2].weight)];
			node[edge[i].node2].numEdge =1;
			node[edge[i].node2].edge[0].node = edge[i].node1;
			node[edge[i].node2].edge[0].weight = 1.0;
			node[edge[i].node2].edge[0].edge_id = i;
			//			 node[edge[i].node2].edge[0].density = density;
		}
		else
		{
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].node = edge[i].node1;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].weight = 1.0;
			//			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].density = density;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].edge_id = i;
			node[edge[i].node2].numEdge++;
		}


	}

	int tempInt =0;
	edge_node = new Edge_Node[2*numEdge];

	for(int i=0;i<numNode;i++)
	{
		node[i].begin = tempInt;
		for(int j=0;j<node[i].numEdge;j++)
		{
			edge_node[tempInt].node = node[i].edge[j].node;
			edge_node[tempInt].weight = node[i].edge[j].weight;
			edge_node[tempInt].edge_id = node[i].edge[j].edge_id;
			tempInt++;
		}
		delete [] node[i].edge;
	}
	srand(time(0));
	noden = new NodeN[numNode];
	for(int i=0;i<numNode;i++)
	{
		strcpy(noden[i].name,node[i].name.c_str()); 
		noden[i].begin = node[i].begin;
		noden[i].numEdge = node[i].numEdge;
		noden[i].weight = node[i].weight;
		noden[i].pos[0] =node[i].pos[0];
		noden[i].pos[1] =node[i].pos[1];
		noden[i].pos[0]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
		noden[i].pos[1]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
		noden[i].stable = false;
		noden[i].gridId = 0;
		noden[i].id =i;
	}
	delete [] node;

	path = new int[numNode];
	path_num =0;
	clique = new int[numNode];
	clique_num=0;

	return true;

}

bool Graph::loadFile_2(const char *fileName)
{
	ifstream file(fileName);
	string str;
	set<string> string_node;
	set<string>::iterator it; 

	numEdge =0;

	while(file.good())
	{
		numEdge++;
		getline(file,str,'\t'); 
		if(str =="")
			break;
		string_node.insert(str);
		getline(file,str,'\t');
		string_node.insert(str);
		getline(file,str,'\n');
	}
	numEdge--;
	
	int index_node=0;
	numNode = string_node.size();
	node = new Node[numNode]();
	for(it = string_node.begin();it!=string_node.end();it++)
		node[index_node++].name =(*it);

	file.clear();
	file.seekg(0,ios::beg);
	 

	edge = new Edge[numEdge];
	int index_edge =0;
	int id;
	int index_node2 =0; 
	 
	while(file.good())
	{	 
	 
		getline(file,str,'\t');

		index_node = findNode(str);
		if(index_node == -1)
			break;
		else
		{

			edge[index_edge].node1 = index_node;
			node[index_node].weight++;
		}

		getline(file,str,'\t');
		index_node2 = findNode(str);
		if(index_node == -1)
			return false;
		else
		{
			edge[index_edge].node2 = index_node2;
			node[index_node2].weight++; 
		}
		str ="";
		getline(file,str,'\n');
		edge[index_edge].str = str;
		edge[index_edge].weight = atof(str.c_str());
	 
		index_edge++;  
	}
	float density;

	for(int i=0;i<numEdge;i++)
	{
		density = 1.0/(node[edge[i].node1].weight*node[edge[i].node2].weight);

		if(node[edge[i].node1].numEdge == 0)
		{
			node[edge[i].node1].edge = new Edge_Node[(int)(node[edge[i].node1].weight)];
			node[edge[i].node1].numEdge =1;
			node[edge[i].node1].edge[0].node = edge[i].node2;
			node[edge[i].node1].edge[0].weight = edge[i].weight;
			node[edge[i].node1].edge[0].edge_id = i;
		}
		else
		{
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].node = edge[i].node2;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].weight = edge[i].weight;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].edge_id = i;
			node[edge[i].node1].numEdge++;
		}

		if(node[edge[i].node2].numEdge == 0)
		{
			node[edge[i].node2].edge = new Edge_Node[(int)(node[edge[i].node2].weight)];
			node[edge[i].node2].numEdge =1;
			node[edge[i].node2].edge[0].node = edge[i].node1;
			node[edge[i].node2].edge[0].weight = edge[i].weight;
			node[edge[i].node2].edge[0].edge_id = i;
			}
		else
		{
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].node = edge[i].node1;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].weight = edge[i].weight;
			//			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].density = density;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].edge_id = i;
			node[edge[i].node2].numEdge++;
		}


	}

	int tempInt =0;
	edge_node = new Edge_Node[2*numEdge];

	for(int i=0;i<numNode;i++)
	{
		node[i].begin = tempInt;
		for(int j=0;j<node[i].numEdge;j++)
		{
			edge_node[tempInt].node = node[i].edge[j].node;
			edge_node[tempInt].weight =sqrt(node[i].edge[j].weight)*0.1;
			edge_node[tempInt].edge_id = node[i].edge[j].edge_id;
			tempInt++;
		}
		delete [] node[i].edge;
	}
	srand(time(0));
	noden = new NodeN[numNode];
	for(int i=0;i<numNode;i++)
	{
		strcpy(noden[i].name,node[i].name.c_str()); 
		noden[i].begin = node[i].begin;
		noden[i].numEdge = node[i].numEdge;
		noden[i].weight = node[i].weight;
		noden[i].pos[0] =node[i].pos[0];
		noden[i].pos[1] =node[i].pos[1];
		noden[i].pos[0]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
		noden[i].pos[1]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
		noden[i].stable = false;
		noden[i].gridId = 0;
		noden[i].id =i;
	}
	delete [] node;

	path = new int[numNode];
	path_num =0;
	clique = new int[numNode];
	clique_num=0;

	return true;

}


bool Graph::loadFile_3(const char *fileName)
{
	ifstream file(fileName);
	string str;
	set<string> string_node;
	set<string>::iterator it; 

	numEdge =0;

	while(file.good())
	{
		numEdge++;
		getline(file,str,'\t'); 
		if(str =="")
			break;
		string_node.insert(str);
		getline(file,str,'\t');
		string_node.insert(str);
		getline(file,str,'\n');
		 
	}
	numEdge--;

	int index_node=0;
	numNode = string_node.size();
	node = new Node[numNode]();
	for(it = string_node.begin();it!=string_node.end();it++)
		node[index_node++].name =(*it);

	file.clear();
	file.seekg(0,ios::beg);


	edge = new Edge[numEdge];
	int index_edge =0;
	int id;
	int index_node2 =0; 

	while(file.good())
	{	 

		getline(file,str,'\t');

		index_node = findNode(str);
		if(index_node == -1)
			break;
		else
		{

			edge[index_edge].node1 = index_node;
			node[index_node].weight++;
		}

		getline(file,str,'\t');
		index_node2 = findNode(str);
		if(index_node == -1)
			return false;
		else
		{
			edge[index_edge].node2 = index_node2;
			node[index_node2].weight++; 
		}

		//	getline(file,str,'\t');
		//	edge[index_edge].kind = atoi(str.c_str());

		str ="";
		getline(file,str,'\t');
		edge[index_edge].str = str;
		edge[index_edge].weight = atof(str.c_str());

		getline(file,str,'\t');
		edge[index_edge].str = str; 
		node[edge[index_edge].node1].pos[0] = atof(str.c_str());
		getline(file,str,'\t');
		edge[index_edge].str = str; 
		node[edge[index_edge].node1].pos[1] = atof(str.c_str());

		getline(file,str,'\t');
		edge[index_edge].str = str; 
		node[edge[index_edge].node2].pos[0] = atof(str.c_str());
		getline(file,str,'\n');
		edge[index_edge].str = str; 
		node[edge[index_edge].node2].pos[1] = atof(str.c_str());


		//	getline(file,str,'\n');





		index_edge++;  
	}
	float density;

	for(int i=0;i<numEdge;i++)
	{
		density = 1.0/(node[edge[i].node1].weight*node[edge[i].node2].weight);

		if(node[edge[i].node1].numEdge == 0)
		{
			node[edge[i].node1].edge = new Edge_Node[(int)(node[edge[i].node1].weight)];
			node[edge[i].node1].numEdge =1;
			node[edge[i].node1].edge[0].node = edge[i].node2;
			node[edge[i].node1].edge[0].weight = edge[i].weight;
			node[edge[i].node1].edge[0].edge_id = i;
			//		 node[edge[i].node1].edge[0].density = density;
		}
		else
		{
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].node = edge[i].node2;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].weight = edge[i].weight;
			//		 node[edge[i].node1].edge[node[edge[i].node1].numEdge].density = density;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].edge_id = i;
			node[edge[i].node1].numEdge++;
		}

		if(node[edge[i].node2].numEdge == 0)
		{
			node[edge[i].node2].edge = new Edge_Node[(int)(node[edge[i].node2].weight)];
			node[edge[i].node2].numEdge =1;
			node[edge[i].node2].edge[0].node = edge[i].node1;
			node[edge[i].node2].edge[0].weight = edge[i].weight;
			node[edge[i].node2].edge[0].edge_id = i;
			//			 node[edge[i].node2].edge[0].density = density;
		}
		else
		{
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].node = edge[i].node1;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].weight = edge[i].weight;
			//			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].density = density;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].edge_id = i;
			node[edge[i].node2].numEdge++;
		}


	}

	int tempInt =0;
	edge_node = new Edge_Node[2*numEdge];

	for(int i=0;i<numNode;i++)
	{
		node[i].begin = tempInt;
		for(int j=0;j<node[i].numEdge;j++)
		{
			edge_node[tempInt].node = node[i].edge[j].node;
			edge_node[tempInt].weight =sqrt(node[i].edge[j].weight)*0.1;
			edge_node[tempInt].edge_id = node[i].edge[j].edge_id;
			tempInt++;
		}
		delete [] node[i].edge;
	}
	srand(time(0));
	noden = new NodeN[numNode];
	for(int i=0;i<numNode;i++)
	{
		strcpy(noden[i].name,node[i].name.c_str()); 
		noden[i].begin = node[i].begin;
		noden[i].numEdge = node[i].numEdge;
		noden[i].weight = node[i].weight;
		noden[i].pos[0] =node[i].pos[0];
		noden[i].pos[1] =node[i].pos[1];
	//	noden[i].pos[0]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
	//	noden[i].pos[1]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
		noden[i].stable = false;
		noden[i].gridId = 0;
		noden[i].id =i;
	}
	delete [] node;

	path = new int[numNode];
	path_num =0;
	clique = new int[numNode];
	clique_num=0;

	return true;

}


bool Graph::loadFile3(const char *fileName)
{
	ifstream file(fileName);
	//ofstream ofile("output.txt");
	string str;
	set<string> string_node;
	set<string>::iterator it; 
	 
 //	getline(file,str,'\n');;// skip the first line;
	numEdge =0;
	
	getline(file,str,'\n');
	while(file.good())
	{
		numEdge++;
		getline(file,str,'\t');
		getline(file,str,'\t'); 
		if(str =="")
			 break;
		string_node.insert(str);
		getline(file,str,'\t');
	   	string_node.insert(str);
		getline(file,str,'\n');
	}
	numEdge--;
	//store the node information to a array of node
	int index_node=0;
	numNode = string_node.size();
	node = new Node[numNode]();
	for(it = string_node.begin();it!=string_node.end();it++)
		node[index_node++].name =(*it);

	file.clear();
	file.seekg(0,ios::beg);
 //	file.ignore(10000,'\n');// skip the first line;

	edge = new Edge[numEdge];
	int index_edge =0;
	int id;
	int index_node2 =0; 
	getline(file,str,'\n'); 
	while(file.good())
	{	 
		getline(file,str,'\t');
		getline(file,str,'\t');
		
		index_node = findNode(str);
		if(index_node == -1)
			 break;
		else
		{
			edge[index_edge].node1 = index_node;
			node[index_node].weight++;
			edge[index_edge].weight =1.0;
		}
		
		getline(file,str,'\t');
	    index_node2 = findNode(str);
		if(index_node == -1)
			return false;
		else
		{
			edge[index_edge].node2 = index_node2;
			node[index_node2].weight++; 
		}

		getline(file,str,'\t');
		edge[index_edge].kind = atoi(str.c_str());

		getline(file,str,'\n');
		edge[index_edge].str = str;



//		file>>edge[index_edge].subgraph;
//		edge[index_edge-1].subgraph = edge[index_edge].subgraph;
		index_edge++;  
	}
	float density;
	
	for(int i=0;i<numEdge;i++)
	 {
		 density = 1.0/(node[edge[i].node1].weight*node[edge[i].node2].weight);
		 
		 if(node[edge[i].node1].numEdge == 0)
		 {
			 node[edge[i].node1].edge = new Edge_Node[(int)(node[edge[i].node1].weight)];
			 node[edge[i].node1].numEdge =1;
			 node[edge[i].node1].edge[0].node = edge[i].node2;
			 node[edge[i].node1].edge[0].weight = 1.0;
			 node[edge[i].node1].edge[0].edge_id = i;
	//		 node[edge[i].node1].edge[0].density = density;
		 }
		 else
		 {
			 node[edge[i].node1].edge[node[edge[i].node1].numEdge].node = edge[i].node2;
			 node[edge[i].node1].edge[node[edge[i].node1].numEdge].weight = 1.0;
	//		 node[edge[i].node1].edge[node[edge[i].node1].numEdge].density = density;
			 node[edge[i].node1].edge[node[edge[i].node1].numEdge].edge_id = i;
			 node[edge[i].node1].numEdge++;
		 }

		 if(node[edge[i].node2].numEdge == 0)
		 {
			 node[edge[i].node2].edge = new Edge_Node[(int)(node[edge[i].node2].weight)];
			 node[edge[i].node2].numEdge =1;
			 node[edge[i].node2].edge[0].node = edge[i].node1;
			 node[edge[i].node2].edge[0].weight = 1.0;
			 node[edge[i].node2].edge[0].edge_id = i;
//			 node[edge[i].node2].edge[0].density = density;
		 }
		 else
		 {
			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].node = edge[i].node1;
			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].weight = 1.0;
//			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].density = density;
			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].edge_id = i;
			 node[edge[i].node2].numEdge++;
		 }


	 }

	int tempInt =0;
	edge_node = new Edge_Node[2*numEdge];
 
	for(int i=0;i<numNode;i++)
	{
		node[i].begin = tempInt;
		for(int j=0;j<node[i].numEdge;j++)
		{
			edge_node[tempInt].node = node[i].edge[j].node;
			edge_node[tempInt].weight = node[i].edge[j].weight;
			edge_node[tempInt].edge_id = node[i].edge[j].edge_id;
			tempInt++;
		}
	 	delete [] node[i].edge;
	}
	srand(time(0));
	noden = new NodeN[numNode];
	for(int i=0;i<numNode;i++)
	{
		strcpy(noden[i].name,node[i].name.c_str()); 
		noden[i].begin = node[i].begin;
		noden[i].numEdge = node[i].numEdge;
		noden[i].weight = node[i].weight;
		noden[i].pos[0] =node[i].pos[0];
		noden[i].pos[1] =node[i].pos[1];
	//	noden[i].pos[0]  = (float)(rand()/RAND_MAX)*300 - 0.5;
	//	noden[i].pos[1]  = (float)(rand()/RAND_MAX)*300 - 0.5;

		noden[i].pos[0]  = (float)(rand()/RAND_MAX)-0.5;
		noden[i].pos[1]  = (float)(rand()/RAND_MAX)-0.5;
		noden[i].stable = false;
		noden[i].id = i;
	}
	delete [] node;

	path = new int[numNode];
	path_num =0;
	clique = new int[numNode];
	clique_num =0;
 
	return true;

}

bool Graph::loadFile2(const char *fileName)
{
	ifstream file(fileName);
	//ofstream ofile("output.txt");
	string str;
	set<string> string_node;
	set<string>::iterator it; 

	//	getline(file,str,'\n');;// skip the first line;
	numEdge =0;

	//	getline(file,str,'\n');
	while(file.good())
	{
		numEdge++;
		//		getline(file,str,'\t');
		getline(file,str,'\t'); 
		if(str =="")
			break;
		string_node.insert(str);
		getline(file,str,'\n');
		string_node.insert(str);
		//getline(file,str,'\n');
	}
	numEdge--;
	//store the node information to a array of node
	int index_node=0;
	numNode = string_node.size();
	node = new Node[numNode]();
	for(it = string_node.begin();it!=string_node.end();it++)
		node[index_node++].name =(*it);

	file.clear();
	file.seekg(0,ios::beg);
	//	file.ignore(10000,'\n');// skip the first line;

	edge = new Edge[numEdge];
	int index_edge =0;
	int id;
	int index_node2 =0; 
	//	getline(file,str,'\n'); 
	while(file.good())
	{	 
		//		getline(file,str,'\t');
		getline(file,str,'\t');

		index_node = findNode(str);
		if(index_node == -1)
			break;
		else
		{
			edge[index_edge].node1 = index_node;
			node[index_node].weight++;
			edge[index_edge].weight =1.0;
		}

		getline(file,str,'\n');
		index_node2 = findNode(str);
		if(index_node == -1)
			return false;
		else
		{
			edge[index_edge].node2 = index_node2;
			node[index_node2].weight++; 
		}

// 		getline(file,str,'\t');
// 		edge[index_edge].kind = atoi(str.c_str());
// 
// 		getline(file,str,'\t');
// 
// 		getline(file,str,'\n');
// 		edge[index_edge].str = str;



		//		file>>edge[index_edge].subgraph;
		//		edge[index_edge-1].subgraph = edge[index_edge].subgraph;
		index_edge++;  
	}
	float density;

	for(int i=0;i<numEdge;i++)
	{
		density = 1.0/(node[edge[i].node1].weight*node[edge[i].node2].weight);

		if(node[edge[i].node1].numEdge == 0)
		{
			node[edge[i].node1].edge = new Edge_Node[(int)(node[edge[i].node1].weight)];
			node[edge[i].node1].numEdge =1;
			node[edge[i].node1].edge[0].node = edge[i].node2;
			node[edge[i].node1].edge[0].weight = 1.0;
			node[edge[i].node1].edge[0].edge_id = i;
			//		 node[edge[i].node1].edge[0].density = density;
		}
		else
		{
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].node = edge[i].node2;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].weight = 1.0;
			//		 node[edge[i].node1].edge[node[edge[i].node1].numEdge].density = density;
			node[edge[i].node1].edge[node[edge[i].node1].numEdge].edge_id = i;
			node[edge[i].node1].numEdge++;
		}

		if(node[edge[i].node2].numEdge == 0)
		{
			node[edge[i].node2].edge = new Edge_Node[(int)(node[edge[i].node2].weight)];
			node[edge[i].node2].numEdge =1;
			node[edge[i].node2].edge[0].node = edge[i].node1;
			node[edge[i].node2].edge[0].weight = 1.0;
			node[edge[i].node2].edge[0].edge_id = i;
			//			 node[edge[i].node2].edge[0].density = density;
		}
		else
		{
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].node = edge[i].node1;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].weight = 1.0;
			//			 node[edge[i].node2].edge[node[edge[i].node2].numEdge].density = density;
			node[edge[i].node2].edge[node[edge[i].node2].numEdge].edge_id = i;
			node[edge[i].node2].numEdge++;
		}


	}

	int tempInt =0;
	edge_node = new Edge_Node[2*numEdge];

	for(int i=0;i<numNode;i++)
	{
		node[i].begin = tempInt;
		for(int j=0;j<node[i].numEdge;j++)
		{
			edge_node[tempInt].node = node[i].edge[j].node;
			edge_node[tempInt].weight = node[i].edge[j].weight;
			edge_node[tempInt].edge_id = node[i].edge[j].edge_id;
			tempInt++;
		}
		delete [] node[i].edge;
	}
	srand(time(0));
	noden = new NodeN[numNode];
	for(int i=0;i<numNode;i++)
	{
		strcpy(noden[i].name,node[i].name.c_str()); 
		noden[i].begin = node[i].begin;
		noden[i].numEdge = node[i].numEdge;
		noden[i].weight = node[i].weight;
		noden[i].pos[0] =node[i].pos[0];
		noden[i].pos[1] =node[i].pos[1];
		noden[i].id = i;
//		noden[i].pos[0]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;
//		noden[i].pos[1]  = (float)(rand()/RAND_MAX)*(FORCE/2) - 0.5;

		noden[i].pos[0]  = (float)(rand()/RAND_MAX);
		noden[i].pos[1]  = (float)(rand()/RAND_MAX);
		noden[i].stable = false;
		noden[i].gridId = 0;
		noden[i].visited = true;
	}
	delete [] node;

	path = new int[numNode];
	path_num =0;
	clique = new int[numNode];
	clique_num =0;

	return true;
}


 
int Graph::findNode(string name)
{
	int begin, end;
	begin =0;
	end = numNode-1;
	int result;
	int data;

	while(begin<end)
	{
		data = (begin+end)/2;
		result = node[data].name.compare(name);
		if(result ==0)
			return data;
		else if(result<0)
			begin = data+1;
		else if(result>0)
			end = data-1;
		if(begin == end)
		{
			result = node[begin].name.compare(name);
			if(result ==0)
				return begin;
			else
				return -1;
		}
	}
	return -1;

}



void Graph::posInitia()
{
	srand(time(0));
	for(int i=0;i<numNode;i++)
	{
		noden[i].index = i;
		noden[i].pos[0]  = (float)rand()/RAND_MAX;
		noden[i].pos[1]  = (float)rand()/RAND_MAX;
		noden[i].pos[0]*= FORCE;
		noden[i].pos[1]*= FORCE;
		noden[i].pos[0]-=(FORCE/2);
		noden[i].pos[1]-=(FORCE/2);
	}
	//	BuildTree();

}





void Graph::GetAABB()
{

	float min_data =100000000000;
	float max_data =-100000000000;
	for(int i=0;i<numNode;i++)
	{
		if(noden[i].pos[0]>=max_data)
			max_data =noden[i].pos[0];

		if(noden[i].pos[0]<=min_data)
			min_data =noden[i].pos[0]; 
	}


	aabb.minPos[0] = min_data;
	aabb.maxPos[0] = max_data;

	min_data =10000000;
	max_data =-10000000;

	for(int i=0;i<numNode;i++)
	{
		if(noden[i].pos[1]>=max_data)
			max_data =noden[i].pos[1];

		if(noden[i].pos[1]<=min_data)
			min_data =noden[i].pos[1]; 
	}

	aabb.minPos[1] = min_data;
	aabb.maxPos[1] = max_data;

	aabb.minPos[0] -=0.01;
	aabb.minPos[1] -=0.01;


	aabb.maxPos[0] +=0.01;
	aabb.maxPos[1] +=0.01;



}

void Graph::BuildTree()
{
	insideNodeNum = 0;
	leafNodeNum =0;
	sizeofLeafNode=0;



	if(tree_root!=NULL)
		return;

	tree_root = new QuaTree;
	tree_root->weight = noden[0].weight;
	tree_root->pos[0] = noden[0].pos[0];
	tree_root->pos[1] = noden[0].pos[1];
	tree_root->childCount =0;
	tree_root->depth =0;
	tree_root->aabb.maxPos[0] = aabb.maxPos[0];
	tree_root->aabb.maxPos[1] = aabb.maxPos[1];
	tree_root->creatID= createIdTree;
	createIdTree++;

	tree_root->aabb.minPos[0] = aabb.minPos[0];
	tree_root->aabb.minPos[1] = aabb.minPos[1];
	tree_root->isLeaf = false;
	insideNodeNum++;

	int cur_num =  numNode;
	int *temp_node_0,*temp_node_1,*temp_node_2,*temp_node_3;
	temp_node_0 = new int[numNode];
	temp_node_1 = new int[numNode];
	temp_node_2 = new int[numNode];
	temp_node_3 = new int[numNode];

	int child_Num[4];
	child_Num[0]=child_Num[1]=child_Num[2]=child_Num[3]=0;

	float middle_x;
	float middle_y;

	middle_x = (aabb.minPos[0] + aabb.maxPos[0])*0.5;
	middle_y = (aabb.minPos[1] + aabb.maxPos[1])*0.5;

	if (noden[0].pos[1]<=middle_y)
	{
		if (noden[0].pos[0]<= middle_x)
		{
			temp_node_0[0] = 0;
			child_Num[0]++;
		}
		else if(noden[0].pos[0]>= middle_x)
		{
			temp_node_1[0] = 0;
			child_Num[1]++;
		}
	}
	else if(noden[0].pos[1]>=middle_y)
	{
		if (noden[0].pos[0]<= middle_x)
		{
			temp_node_2[0] = 0;
			child_Num[2]++;
		}
		else if(noden[0].pos[0]>= middle_x)
		{
			temp_node_3[0] = 0;
			child_Num[3]++;
		}
	}

	for (int i=1;i<numNode;i++)
	{
		tree_root->pos[0] = (tree_root->weight*tree_root->pos[0] + noden[i].weight*noden[i].pos[0])/(tree_root->weight + noden[i].weight);
		tree_root->pos[1] = (tree_root->weight*tree_root->pos[1] + noden[i].weight*noden[i].pos[1])/(tree_root->weight + noden[i].weight);
		tree_root->weight += noden[i].weight;
		 
		if (noden[i].pos[1]<=middle_y)
		{
			if (noden[i].pos[0]<= middle_x)
			{
				temp_node_0[child_Num[0]] = i;
				child_Num[0]++;
			}
			else if(noden[i].pos[0]>= middle_x)
			{
				temp_node_1[child_Num[1]] = i;
				child_Num[1]++;
			}
		}
		else if(noden[i].pos[1]>=middle_y)
		{
			if (noden[i].pos[0]<= middle_x)
			{
				temp_node_2[child_Num[2]] = i;
				child_Num[2]++;
			}
			else if(noden[i].pos[0]>= middle_x)
			{
				temp_node_3[child_Num[3]] = i;
				child_Num[3]++;
			}
		} 
	}

	if(child_Num[0]>0)
	{
		BuildTree2(tree_root,0,temp_node_0,child_Num[0]);
		if(tree_root->child[0]->isLeaf==false)
			delete [] temp_node_0;
	}

	if(child_Num[1]>0)
	{
		BuildTree2(tree_root,1,temp_node_1,child_Num[1]);
		if(tree_root->child[1]->isLeaf==false)
			delete [] temp_node_1;
	}

	if(child_Num[2]>0)
	{
		BuildTree2(tree_root,2,temp_node_2,child_Num[2]);
		if(tree_root->child[2]->isLeaf==false)
			delete [] temp_node_2;
	}

	if(child_Num[3]>0)
	{
		BuildTree2(tree_root,3,temp_node_3,child_Num[3]);
		if(tree_root->child[3]->isLeaf==false)
			delete [] temp_node_3;
	}
} 

void Graph::BuildTree2(QuaTree* parent, int index, int* nodelist, int num_Nodes)
{
	if(num_Nodes>=1)
	{
		QuaTree* child =  new QuaTree;
		child->parent = parent;
		child->child[0]=child->child[1]=child->child[2]=child->child[3]=0;
		parent->child[index] = child;
		child->depth = parent->depth+1;
		child->childCount=0;
		child->creatID=createIdTree;
		createIdTree++;
		parent->childCount++;

		child->weight = (noden[nodelist[0]]).weight;
		child->pos[0] = (noden[nodelist[0]]).pos[0];
		child->pos[1] = (noden[nodelist[0]]).pos[1];
		child->isLeaf = false;

		 


		if(index<2)
		{
			child->aabb.minPos[1]= parent->aabb.minPos[1];
			child->aabb.maxPos[1]= (parent->aabb.maxPos[1]+parent->aabb.minPos[1])*0.5;
		}
		else
		{
			child->aabb.minPos[1]= (parent->aabb.maxPos[1]+parent->aabb.minPos[1])*0.5;
			child->aabb.maxPos[1]= parent->aabb.maxPos[1];
		}
		if(index%2==0)
		{
			child->aabb.minPos[0] = parent->aabb.minPos[0];
			child->aabb.maxPos[0] = (parent->aabb.minPos[0]+parent->aabb.maxPos[0])*0.5;
		}
		else
		{
			child->aabb.maxPos[0] = parent->aabb.maxPos[0];
			child->aabb.minPos[0] =(parent->aabb.maxPos[0]+parent->aabb.minPos[0])*0.5;
		}

		if(child->depth>=MAX_DEPTH || num_Nodes<=SMALL_CIRTERION)
		{
			child->isLeaf = true;
			child->num_nodes = num_Nodes;
			child->list = nodelist;
			leafNodeNum++;

			for (int i=1;i<num_Nodes;i++)
			{

				child->pos[0] = (child->weight*child->pos[0] + noden[nodelist[i]].weight*noden[nodelist[i]].pos[0])/(child->weight + noden[nodelist[i]].weight);
				child->pos[1] = (child->weight*child->pos[1] + noden[nodelist[i]].weight*noden[nodelist[i]].pos[1])/(child->weight + noden[nodelist[i]].weight);
				child->weight += noden[nodelist[i]].weight;
			}
		}

		else{


			insideNodeNum++;
			float middle_x = (child->aabb.minPos[0] + child->aabb.maxPos[0])*0.5f;
			float middle_y = (child->aabb.minPos[1] + child->aabb.maxPos[1])*0.5f;

			int** temp_node = new int*[4];
			int* temp_node_0, *temp_node_1, *temp_node_2, *temp_node_3;
			temp_node_0 = new int[num_Nodes];
			temp_node_1 = new int[num_Nodes];
			temp_node_2 = new int[num_Nodes];
			temp_node_3 = new int[num_Nodes];

			int child_Num[4];
			child_Num[0]=child_Num[1]=child_Num[2]=child_Num[3]=0;

			if (noden[nodelist[0]].pos[1]<=middle_y)
			{
				if (noden[nodelist[0]].pos[0]<= middle_x)
			 {
				 temp_node_0[0] = nodelist[0];
				 child_Num[0]++;
			 }
				else if(noden[nodelist[0]].pos[0]>= middle_x)
			 {
				 temp_node_1[0] = nodelist[0];
				 child_Num[1]++;
			 }
			}
			else if(noden[nodelist[0]].pos[1]>=middle_y)
			{
				if (noden[nodelist[0]].pos[0]<= middle_x)
			 {
				 temp_node_2[0] = nodelist[0];
				 child_Num[2]++;
			 }
				else if(noden[nodelist[0]].pos[0]>= middle_x)
			 {
				 temp_node_3[0] = nodelist[0];
				 child_Num[3]++;
			 }
			}

			for (int i=1;i<num_Nodes;i++)
			{

				child->pos[0] = (child->weight*child->pos[0] + noden[nodelist[i]].weight*noden[nodelist[i]].pos[0])/(child->weight + noden[nodelist[i]].weight);
				child->pos[1] = (child->weight*child->pos[1] + noden[nodelist[i]].weight*noden[nodelist[i]].pos[1])/(child->weight + noden[nodelist[i]].weight);
				child->weight += noden[nodelist[i]].weight;

				if (noden[nodelist[i]].pos[1]<=middle_y)
				{
					if (noden[nodelist[i]].pos[0]<= middle_x)
				 {
					 temp_node_0[child_Num[0]] = nodelist[i];
					 child_Num[0]++;
				 }
					else if(noden[nodelist[i]].pos[0]>= middle_x)
				 {
					 temp_node_1[child_Num[1]] = nodelist[i];
					 child_Num[1]++;
				 }
			 }
				else if(noden[nodelist[i]].pos[1]>=middle_y)
			 {
				 if (noden[nodelist[i]].pos[0]<= middle_x)
				 {
					 temp_node_2[child_Num[2]] = nodelist[i];
					 child_Num[2]++;
				 }
				 else if(noden[nodelist[i]].pos[0]>= middle_x)
				 {
					 temp_node_3[child_Num[3]] = nodelist[i];
					 child_Num[3]++;
				 }
				} 
			}


			if(child_Num[0]>0)
			{
				BuildTree2(child,0,temp_node_0,child_Num[0]);
				if(child->child[0]->isLeaf==false)
					delete [] temp_node_0;
			}

			if(child_Num[1]>0)
			{
				BuildTree2(child,1,temp_node_1,child_Num[1]);
				if(child->child[1]->isLeaf==false)
					delete [] temp_node_1;
			}

			if(child_Num[2]>0)
			{
				BuildTree2(child,2,temp_node_2,child_Num[2]);
				if(child->child[2]->isLeaf==false)
					delete [] temp_node_2;
			}

			if(child_Num[3]>0)
			{
				BuildTree2(child,3,temp_node_3,child_Num[3]);
				if(child->child[3]->isLeaf==false)
					delete [] temp_node_3;
			}

		 


		}

	}
	else
	{
		//zero nodes in this part and how to deal with this situation
	}


}

void Graph::Cleanup()
{
	createIdTree = 0;
	delete []treeInside;
	treeInside=0;
	delete []referenceNode;
	referenceNode = 0;

	delete [] grid_root;
	grid_root =0;
	delete order_node;
	order_node =0;
}

void Graph::PrepareGPU()
 {
	 
	 int insideIndex=1;
	 int leafIndex=0;
	 int parentIndex=0;
	 QuaTree* current;
	 current = tree_root;

	 if(treeInside==0)
		treeInside = new TreeNodeInside[insideNodeNum + leafNodeNum];
	 else
	 {
		 printf("WRONG ABOUT TREENODEINSIDE/N");
		 getchar();
	 }
	// treeLeafNode = new TreeLeafNode[leafNodeNum];
	 if(referenceNode==0)
		referenceNode = new int[numNode];
	 else
	 {
		 printf("WRONG ABOUT REFERENCENODE/N");
		 getchar();
	 }

	 treeInside[0].pos[0] = current->pos[0];
	 treeInside[0].pos[1] = current->pos[1];
	 treeInside[0].weight = current->weight;
	 treeInside[0].width = (current->aabb.maxPos[0]) -(current->aabb.minPos[0]);
	 treeInside[0].isLeaf = false;
	 treeInside[0].depth=current->depth;
	 treeInside[0].parent =-1;
	 treeInside[0].order = 0 ;
	 treeInside[0].childNum = current->childCount;
	 treeInside[0].brother = -1;

	 for (int i=0;i<4;i++)
	 {
		 if(current->child[i]!=0)
		 {
			 if(i==3)
				PrepareGPU2(parentIndex,current->child[i],insideIndex,i,-1);
			 else
			 {
				 int j=i+1;
				 while((current->child[j]==0)&&j<4)
					 j++;
				 if(j==4)
					PrepareGPU2(parentIndex,current->child[i],insideIndex,i,-1);
				 else
					PrepareGPU2(parentIndex,current->child[i],insideIndex,i,current->child[j]->creatID);
			 }

		 }
		 else
			 treeInside[parentIndex].children[i] = -1;
	 }
	 delete tree_root;
	 tree_root =0;
 }

 void Graph::PrepareGPU2(int parentID,QuaTree* current,int& insideIndex, int index, int brother)
 {
	 if(current->isLeaf == false)
	 {

		 int id_par = insideIndex;
		
		 treeInside[parentID].children[index] = insideIndex;
		 treeInside[insideIndex].isLeaf = false;
		 treeInside[insideIndex].pos[0] = current->pos[0];
		 treeInside[insideIndex].pos[1] = current->pos[1];
		 treeInside[insideIndex].weight = current->weight;
		 treeInside[insideIndex].width = current->aabb.maxPos[0] - current->aabb.minPos[0];
		 treeInside[insideIndex].depth=current->depth;
		 treeInside[insideIndex].parent = parentID;
		 treeInside[insideIndex].order = 0;
		 treeInside[insideIndex].childNum = current->childCount;
		 treeInside[insideIndex].brother = brother;
			 

		 
		 insideIndex++;

		 for (int i=0;i<4;i++)
		 {
// 			 if(current->child[i]!=0)
// 				 PrepareGPU2(id_par,current->child[i],insideIndex,i);
// 			 else
// 				 treeInside[insideIndex-1].children[i] = -1;

			 if(current->child[i]!=0)
			 {
				 if(i==3)
					 PrepareGPU2(id_par,current->child[i],insideIndex,i,-1);
				 else
				 {
					 int j=i+1;
					 while((current->child[j]==0)&&j<4)
						 j++;
					 if(j==4)
						 PrepareGPU2(id_par,current->child[i],insideIndex,i,-1);
					 else
						 PrepareGPU2(id_par,current->child[i],insideIndex,i,current->child[j]->creatID);
				 }

			 }
			 else
				 treeInside[insideIndex-1].children[i] = -1;
		 }
		 delete current;
		 current =0;
	 }
	 else
	 {
		 
		 treeInside[parentID].children[index] = insideIndex;
		 treeInside[insideIndex].isLeaf = true;
		 treeInside[insideIndex].pos[0] = current->pos[0];
		 treeInside[insideIndex].pos[1] = current->pos[1];
		 treeInside[insideIndex].weight = current->weight;
		 treeInside[insideIndex].width = current->aabb.maxPos[0] - current->aabb.minPos[0];
		 treeInside[insideIndex].begin = sizeofLeafNode;
		 treeInside[insideIndex].numNode = current->num_nodes;
		 treeInside[insideIndex].depth=current->depth;
		 treeInside[insideIndex].parent = parentID;
		 treeInside[insideIndex].order =-1;
		 treeInside[insideIndex].childNum = current->childCount;
		 treeInside[insideIndex].brother = brother;
		 insideIndex++;

		 for (int i=0;i<current->num_nodes;i++)
		 {
			 referenceNode[sizeofLeafNode] = current->list[i];
			 sizeofLeafNode++;
		 }
		 delete [] current->list;
		 delete current;
		 current =0;
	 }

 }


 int Graph::TestTraverseTree()
 {
	 int i=1;

	 int stack[1000];
	 stack[0] =0;
	 int index =1;
	 int current;

	 float dist;
	 int energy=0;
	 int parent;

	  
	 current = treeInside[0].children[treeInside[0].order];
	 treeInside[0].order++;

	 while(current>0)
	 {
		 dist  = Distance(i,current);
		 printf("current%d\n",current);

		 printf("Node ID:%d\n",current);

		 if(dist<TREE_CIRTERION*treeInside[current].width)
		 {
			 if(treeInside[current].isLeaf)
			 {
				energy+=treeInside[current].numNode;

				if(treeInside[current].brother>0)
					current  = treeInside[current].brother;
				else{
					current  = treeInside[current].parent;

					if(treeInside[current].brother<0){
					while(treeInside[current].brother<0)
					{
						current = treeInside[current].parent;
						if(current<0)
							return 0;
					}
					}
						current = treeInside[current].brother;
				}
				
// 				while(treeInside[parent].order == treeInside[parent].childNum)
// 				{
// 					parent = treeInside[parent].parent;
// 					if(parent==-1)
// 						return energy;
// 				}
// 
// 				index = treeInside[parent].order;
// 				while(treeInside[parent].children[index]<0)
// 					index++;
// 				current = treeInside[parent].children[index];
// 				treeInside[parent].order++;
			 }
			 else
			 {
// 				 treeInside[current].order++;
// 				 index =0;
// 				 while(treeInside[current].children[index]<0)
// 					 index++;
// 				 current = treeInside[current].children[index];

				 int i=0;
				 while(treeInside[current].children[i]<0)
					 i++;
				 current  = treeInside[current].children[i];
//  

			 }
		 }
		 else
		 {
			 energy +=1;

// 			 parent =  treeInside[current].parent;
// 			 while(treeInside[parent].order == treeInside[parent].childNum)
// 				 parent = treeInside[parent].parent;
// 
// 			 index = treeInside[parent].order;
// 			 while(treeInside[parent].children[index]<0)
// 				 index++;
// 			 current = treeInside[parent].children[index];
// 			 treeInside[parent].order++;

			 if(treeInside[current].brother>0)
				 current  = treeInside[current].brother;
			 else{
				 current  = treeInside[current].parent;

				 if(treeInside[current].brother<0){
					 while(treeInside[current].brother<0)
					 {
						 current = treeInside[current].parent;
						 if(current<0)
							 return 0;
					 }
					}
				 
					 current = treeInside[current].brother;
				}


		 }



	 }
	return energy;
 }

 float Graph::Distance(int id1, int id2)
 {
	 float dist=0;
	 float diff;
	 diff = noden[id1].pos[0] - treeInside[id2].pos[0];
	 dist += diff*diff;
	 diff = noden[id1].pos[1] - treeInside[id2].pos[1];
	 dist += diff*diff;
	 return sqrt(dist);
 }

 void Graph::GridBuild()
 {

	 if (grid_root ==0)
	 {
		 grid_root = new grid[GIRD_SIZE*GIRD_SIZE];
		 order_node= new int[numNode];

		 for (int i=0;i<GIRD_SIZE*GIRD_SIZE;i++)
		 {
			 grid_root[i].begin = 0;
			 grid_root[i].id = 0;
			 grid_root[i].numPoints = 0;
		 }

	 }
	 else 
		 return;



	 int x, y;

	 float strip  = FORCE/GIRD_SIZE;

	 for (int i=0;i<numNode;i++)
	 {
		 x = (noden[i].pos[0]+FORCE/2)/strip;
		 y = (noden[i].pos[1]+FORCE/2)/strip;
		 if(x>=GIRD_SIZE)
			 x = x-1;
		 if(y>=GIRD_SIZE)
			 y = y-1;
		 grid_root[x*GIRD_SIZE + y].numPoints++;
		 noden[i].gridId = x*GIRD_SIZE + y;
		 order_node[i] =0;
	 }

	 //pre-sum
	//  printf("%d	",grid_root[0].begin); 
	 for (int i=1;i<GIRD_SIZE*GIRD_SIZE;i++)
	 {
		 grid_root[i].begin = grid_root[i-1].begin + grid_root[i-1].numPoints; 
		 grid_root[i].id = grid_root[i].begin;
	//	 printf("%d	",grid_root[i].begin); 
	 }

// 	 printf("\n");
// 	 for (int i=0;i<GIRD_SIZE*GIRD_SIZE;i++)
// 	 {
// 		 printf("%d	",grid_root[i].numPoints);
// 	 }
// 	 printf("\n");




	 for (int i=0;i<numNode;i++)
	 {
		 int index = grid_root[noden[i].gridId].id;
		 order_node[index] = i;
		 grid_root[noden[i].gridId].id++;
	 }
 }