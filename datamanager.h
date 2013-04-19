#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <vector>
#include <map>
#include <set>
#include <string>
using namespace std;

#define TIMESTEP 60
#define RANGESTEP 1
#define RANGERATIO 0.05f

namespace AMT
{

typedef vector<unsigned int> t_uintvector;
typedef map<t_uintvector, unsigned int> t_recordset;
typedef set<pair<unsigned int, unsigned int>> t_pickset;
typedef pair<unsigned int, unsigned int> t_intpair;
typedef map<t_intpair,unsigned int> t_edge;
typedef map<unsigned int, unsigned int> t_ref;


typedef struct{
	string name;
	unsigned int num_bytes;
	unsigned int offset;
	unsigned int num_values;
	vector<string> value_names;
	t_uintvector value_counts;
} t_Item;

typedef struct{
	t_uintvector selectedItems;
	t_recordset records;
	float *vertexdata;
	unsigned char *colordata;
	unsigned int *indexdata;

} t_ParaCoordData;

typedef vector<t_Item> t_Item_Desc;

typedef struct{
	float	weight;
	float	pos[2];
	unsigned int		edge_ref;
	bool	stable;
	//	char	name[30];
	unsigned int index;
	unsigned int kind;
	unsigned int begin;
	unsigned int numEdge;
}t_GraphNode;

typedef struct{
	//unsigned int node1;
	//unsigned int node2;
	int node;
	float weight;
}t_GraphEdge;

typedef struct
{
	unsigned int graph_node_num;
	unsigned int graph_edge_num;
	unsigned int graph_node_currentNum;
	unsigned int graph_edge_currentNum;
	//unsigned int graph_node_current_num;
	t_GraphNode* graph_node;
	t_GraphEdge* graph_edge;
	t_GraphNode* graph_currentNode;
	t_GraphEdge* graph_currentEdge;
	//t_GraphNode* current_node;
	unsigned int selected_item[2];
	unsigned int kind;//pay attention to the ip graph
	bool	     if_layout;
	float		 boundary[3];// this variable used for the node in [0-1]
	bool		m_if_animate;
	bool        edge_show;
	bool		node_pin;
}t_Graph;

typedef struct  
{
	float attExponent;
	float graFactor;
	float graFactor_previous;//before calculation
	float repFactor;
	float repExponent;
	unsigned int numNode;
	unsigned int numEdge;
	float baryCenter[2];
	float range;//show the size of all the nodes
	float nodeSize;
}t_LayoutParamter;

class CDataManager
{
public:
	t_Item_Desc m_item_desc;

	t_pickset m_pickset;

	bool m_ifDataReady;
	bool m_ifGraphReady;

	unsigned int Get_Num_Items() { return (unsigned int) m_item_desc.size(); }

    CDataManager();
	~CDataManager();

	void LoadData( char *filename );
	void PrepareHistogramData( unsigned int _selection );
	void PrepareParaCoordData();
	unsigned int *GetHistogramData() { return m_HistogramData; }

	void PrepareGraphData();
	void CreateGraph();
	void GraphLayout();
	bool LoadGraph(char *filename);
	bool SaveGraph(char *filename);
	void PrepareEdgeLayoutEdgeNode();
	
	t_ParaCoordData m_ParaCoordData;
	t_Graph m_Graph;
	t_edge m_edge_prepare;
	t_LayoutParamter m_LayoutParameter;

	time_t m_MinTime, m_MaxTime;
	time_t m_CurTime, m_TimeRange;

	time_t m_TimeStep;
	time_t m_RangeStep;
	float m_RangeRatio;

	clock_t m_histoTime; // in millisecond 
	clock_t m_paraCoordTime; // in millisecond 
	clock_t m_graphTime;

	map<unsigned int, unsigned int> map_x;
	map<unsigned int, unsigned int> map_y;



	void SetCurrentItem( int _item ) { m_CurrentItem = _item; }
	unsigned int GetCurrentItem() { return m_CurrentItem; }
	unsigned int GetNumberofQueriedRecords() { return m_endidx - m_startidx + 1; } 

	void SelectTimeSpanIndex();
	void ParaCoord_PrepareRenderColorData();
private:
	static CDataManager *s_datamanager;

	unsigned int m_NumberOfRecords;
	unsigned int m_TotalBytes;
	unsigned char *m_d_DataBuffer;
	unsigned char *m_DataBuffer;
	unsigned int m_startidx, m_endidx;
	unsigned int m_CurrentItem;

	t_ref  m_source_ref;
	t_ref  m_destination_ref;

	unsigned int *m_HistogramData;
	
	void Load_Item_Desc_File( char *filename );
	
	unsigned int binarytimesearch( unsigned int _first, unsigned int _last, time_t _key );
	void ParaCoord_PrepareRenderData();



};

}


#endif 