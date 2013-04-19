#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <time.h>

#include <glColor.h>
#include "filebuf.h"
#include "datamanager.h"


#include <windows.h>
#include <iostream>

#include <windows.h>
#include <iostream>
#include <sstream>

 #define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}


namespace AMT
{
extern "C" void kernel_function(t_LayoutParamter* layoutParameter,t_GraphNode* graph_node,t_GraphEdge* graph_edge);

int gpuDeviceInit(int devID)
{
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);

    if (deviceCount == 0)
    {
        wxLogError( "gpuDeviceInit() CUDA error: no devices supporting CUDA.\n" );
        exit(-1);
    }

    if (devID < 0)
       devID = 0;
        
    if (devID > deviceCount-1)
    {
        wxLogError( "\n");
        wxLogError( ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
        wxLogError( ">> gpuDeviceInit (-device=%d) is not a valid GPU device. <<\n", devID);
        wxLogError( "\n");
        return -devID;
    }

    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, devID);

    if (deviceProp.major < 1)
    {
        wxLogError( "gpuDeviceInit(): GPU device does not support CUDA.\n");
		return -1;
    }
    
    cudaSetDevice(devID);
    wxLogMessage("gpuDeviceInit() CUDA Device [%d]: %s\n", devID, deviceProp.name);

    return devID;
}

unsigned int get_char_end(const char *str, unsigned int start, char _c )
{
	unsigned int idx = start;
	while (str[idx] != _c && str[idx] != 0)
		idx++;
	return idx;
}

unsigned int get_char_end_last(const char *str, unsigned int start, char _c )
{
	unsigned int len = strlen( str+start );
	unsigned int idx = start+len-1;
	while (str[idx] != _c && str[idx] != 0)
		idx--;
	return idx;
}

unsigned int get_end_string(const char *str, unsigned int start )
{
	unsigned int idx = start;
	while (str[idx] != ',' && str[idx] != 10 && str[idx] != 13 && str[idx] != 0 )
		idx++;
	return idx;
}

CDataManager::CDataManager() : 	m_ifDataReady(false),m_NumberOfRecords(0),m_d_DataBuffer(NULL),m_TotalBytes(0),m_MinTime(0), m_MaxTime(0), m_CurTime(0),m_TimeRange(0), m_HistogramData(NULL), \
								m_CurrentItem(1), m_DataBuffer(NULL), m_startidx(0), m_endidx(0), m_TimeStep(TIMESTEP), m_RangeStep(RANGESTEP), m_RangeRatio(RANGERATIO), \
								m_histoTime(0.0), m_paraCoordTime(0.0),m_ifGraphReady(false)
{
	gpuDeviceInit(0);
	m_ParaCoordData.vertexdata = NULL;
	m_ParaCoordData.colordata = NULL;
	m_ParaCoordData.indexdata = NULL;

	m_Graph.graph_node = NULL;
	m_Graph.graph_edge = NULL;
	m_Graph.graph_currentNode = NULL;
	m_Graph.graph_currentEdge = NULL;
	m_Graph.if_layout = false;
	m_Graph.m_if_animate = false;
	m_Graph.edge_show = true;
	m_Graph.node_pin = false;
	m_Graph.graph_edge_num =0;
	m_Graph.graph_node_num =0;
	m_Graph.graph_node_currentNum = 0;
	m_Graph.graph_edge_currentNum = 0;
	m_Graph.boundary[0]=-1.0;
	m_Graph.boundary[1]=1.0;
	m_Graph.boundary[2]=1.0;//range
}

CDataManager::~CDataManager()
{
	cudaDeviceReset();
	if ( m_d_DataBuffer != NULL )
		cudaFree( m_d_DataBuffer );
	m_d_DataBuffer = NULL;
	if ( m_DataBuffer != NULL )
		free( m_DataBuffer );
	m_DataBuffer = NULL;
	if ( m_HistogramData != NULL )
		cudaFree( m_HistogramData );
	m_HistogramData = NULL;

	if(m_Graph.graph_edge!=NULL)
		cudaFree( m_Graph.graph_edge);
	if(m_Graph.graph_node!=NULL)
		cudaFree(m_Graph.graph_node);
	if(m_Graph.graph_currentNode != NULL)
		cudaFree(m_Graph.graph_currentNode);
	if(m_Graph.graph_currentEdge != NULL)
		cudaFree(m_Graph.graph_currentEdge);
	m_Graph.graph_edge =NULL;
	m_Graph.graph_node =NULL;
	m_Graph.graph_currentNode = NULL;
	m_Graph.graph_currentEdge = NULL;
}

void CDataManager::Load_Item_Desc_File( char *filename )
{
	CFileBuf infile;
	unsigned int start = 0;
	unsigned int end;
	char buffer[4096], tag[1024];

	infile.open(filename, "r");

	infile.ReadNextLine(buffer);
	end = get_char_end( buffer, 0, ':');
	m_NumberOfRecords = atoi( buffer+end+1 );

	infile.ReadNextLine(buffer);
	end = get_char_end( buffer, 0, ':');
	unsigned int num_items = atoi( buffer+end+1 );
	m_item_desc.resize( num_items );

	for ( int i = 0; i < num_items; i++ )
	{
		infile.ReadNextLine(buffer);
		start = 0;
		end = get_end_string( buffer, start);
		strncpy( tag, buffer+start, end-start );
		tag[end-start] = 0;
		m_item_desc[i].name = tag;
	}

	while (infile.ReadNextLine(buffer) > 0)
	{
		start = 0;
		end = get_char_end( buffer, start, ':');
		strncpy( tag, buffer+start, end-start );
		tag[end-start] = 0;
		unsigned int item_idx = atoi( tag+5 );

		start = end + 1;
		end = get_end_string( buffer, start);
		strncpy( tag, buffer+start, end-start );
		tag[end-start] = 0;
		unsigned int _num_values = atoi( tag );
		m_item_desc[item_idx].num_values = _num_values;
		if ( _num_values < 256 )
			m_item_desc[item_idx].num_bytes = 1;
		else if ( _num_values < 65536 )
			m_item_desc[item_idx].num_bytes = 2;
		else
			m_item_desc[item_idx].num_bytes = 4;

		m_item_desc[item_idx].value_names.resize(_num_values);
		m_item_desc[item_idx].value_counts.resize(_num_values);

		for ( unsigned int i = 0; i < _num_values; i++ )
		{
			infile.ReadNextLine(buffer);
			start = 0;
			end = get_char_end_last( buffer, start, ':');
			strncpy( tag, buffer+start, end-start );
			tag[end-start] = 0;
			m_item_desc[item_idx].value_names[i] = tag;

			start = end + 1;
			end = get_end_string( buffer, start);
			strncpy( tag, buffer+start, end-start );
			tag[end-start] = 0;
			m_item_desc[item_idx].value_counts[i] = atoi( tag );
		}
	}

	for ( t_Item_Desc::iterator it = m_item_desc.begin()+1; it != m_item_desc.end(); it++ )
	{
		if ( (*it).num_values == 1 )
		{
			it = m_item_desc.erase( it );
			it--;
		}
	}

	unsigned int _offset = sizeof(time_t);
	for ( t_Item_Desc::iterator it = m_item_desc.begin()+1; it != m_item_desc.end(); it++ )
	{
		(*it).offset = _offset;
		_offset += (*it).num_bytes;
	}

	infile.close();
}

void CDataManager::LoadData( char *filename )
{
	char item_desc_filename[256], tmp[256];
	int len = strlen(filename);
	strncpy( tmp, filename, len-5 );
	tmp[len-5] = 0;
	sprintf( item_desc_filename, "%s.stats", tmp );

	Load_Item_Desc_File( item_desc_filename );

	FILE *infile;
	unsigned int fsize = 0;
	infile = fopen( filename, "rb" );
	fseek( infile, 0, SEEK_END );
	fsize = ftell(infile);
	fseek( infile, 0, SEEK_SET );

	m_TotalBytes = sizeof(time_t);
	for ( unsigned int i = 1; i < Get_Num_Items(); i++ )
		m_TotalBytes += m_item_desc[i].num_bytes;

	if ( m_NumberOfRecords != fsize / m_TotalBytes )
		 wxLogError( "Data file and description file are not matched.\n");

	m_DataBuffer = (unsigned char*) malloc( fsize );
	fread( m_DataBuffer,1,fsize, infile );

	m_MinTime = *((time_t *) m_DataBuffer);
	m_MaxTime = *((time_t *) (m_DataBuffer+(m_NumberOfRecords-1)*m_TotalBytes));

	m_CurTime = m_MinTime;
	m_TimeRange = 1;

	m_startidx = 0; 
	m_endidx = m_NumberOfRecords - 1;

//	cudaMalloc( (void **)&m_d_DataBuffer, fsize );
//	cudaMemcpy( m_d_DataBuffer, m_DataBuffer, fsize, cudaMemcpyHostToDevice );

	fclose(infile);

	m_ifDataReady = true;
	m_ifGraphReady = false;
}

unsigned int CDataManager::binarytimesearch( unsigned int _first, unsigned int _last, time_t _key )
{
//	_key -= m_MinTime;
	unsigned int first = _first;
	int last = _last;
	while (first <= last)
	{
	   unsigned int mid = (first + last) / 2;  // compute mid point.
	   time_t middlevalue = *((time_t *) (m_DataBuffer+mid*m_TotalBytes));
	   if (_key > middlevalue) 
		   first = mid + 1;  // repeat search in top half.
	   else if (_key < middlevalue) 
		   last = mid - 1; // repeat search in bottom half.
	   else
		   return mid;     // found it. return position /////
	}
	return (first + 1);    // failed to find key
}

void CDataManager::SelectTimeSpanIndex()
{
	m_startidx = binarytimesearch( 0, m_NumberOfRecords - 1, m_CurTime );
	m_endidx = binarytimesearch( m_startidx, m_NumberOfRecords - 1, m_CurTime + m_TimeRange );
	if ( m_endidx >= m_NumberOfRecords )
		m_endidx = m_NumberOfRecords;
}


void CDataManager::PrepareHistogramData( unsigned int _selection )
{
	unsigned int _sel = _selection;
	if ( _sel < 1 )
		return;

	clock_t start, finish;
	start = clock();
	unsigned int _numvalues = m_item_desc[_sel].num_values;
	unsigned int byteoffset = m_item_desc[_sel].offset;

	unsigned int numbytes = m_item_desc[_sel].num_bytes;

	if ( m_HistogramData != NULL )
		free( m_HistogramData );
	m_HistogramData = (unsigned int *)malloc(_numvalues*sizeof(unsigned int));

	for ( unsigned int i = 0; i <  _numvalues; i++ )
		m_HistogramData[i] = 0;
	for ( unsigned int i = m_startidx; i <= m_endidx; i++ )
	{
		if ( numbytes == 1 )
		{
			unsigned char item = *((unsigned char *) (m_DataBuffer+i*m_TotalBytes+byteoffset));
			m_HistogramData[item]++;
		}
		else if ( numbytes == 2 )
		{
			unsigned short item = *((unsigned short *) (m_DataBuffer+i*m_TotalBytes+byteoffset));
			m_HistogramData[item]++;
		}
		else
		{
			unsigned int item = *((unsigned int *) (m_DataBuffer+i*m_TotalBytes+byteoffset));
			m_HistogramData[item]++;
		}
	}
	finish = clock();
	m_histoTime = (double)(finish - start);
}

void CDataManager::PrepareParaCoordData()
{
	t_uintvector &si = m_ParaCoordData.selectedItems;
	if ( si.size() < 2 )
		return;

	t_recordset::iterator it;
	pair<t_recordset::iterator,bool> ret;

	clock_t start, finish;
	start = clock();

	m_ParaCoordData.records.clear();

	for ( unsigned int i = m_startidx; i <= m_endidx; i++ )
	{
		t_uintvector vec;
		for ( unsigned int j = 0; j < si.size(); j++ )
		{
			unsigned int idx = si[j];
			unsigned int numbytes = m_item_desc[idx].num_bytes;
			if ( numbytes == 1 )
			{
				unsigned char item = *((unsigned char *) (m_DataBuffer+i*m_TotalBytes+m_item_desc[idx].offset));
				vec.push_back( (unsigned int) item );
			}
			else if ( numbytes == 2 )
			{
				unsigned short item = *((unsigned short *) (m_DataBuffer+i*m_TotalBytes+m_item_desc[idx].offset));
				vec.push_back( (unsigned int) item );
			}
			else
			{
				unsigned int item = *((unsigned int *) (m_DataBuffer+i*m_TotalBytes+m_item_desc[idx].offset));
				vec.push_back( item );
			}
		}
		ret = m_ParaCoordData.records.insert( pair<t_uintvector, unsigned int>(vec, 1));
		if ( ret.second == false )
			m_ParaCoordData.records[vec] = m_ParaCoordData.records[vec] + 1;
	}

	ParaCoord_PrepareRenderData();
	finish = clock();
	m_paraCoordTime = (double)(finish - start);
}

void CDataManager::ParaCoord_PrepareRenderColorData()
{
	if ( m_ParaCoordData.colordata == NULL )
		return;

	t_uintvector &si = m_ParaCoordData.selectedItems;
	unsigned int color_rs = si.size()*3;

	memset( m_ParaCoordData.colordata, (unsigned char)(LIGHT_GRAY[0]*255), m_ParaCoordData.records.size()*color_rs*sizeof(char) );

	unsigned char _red[3];
	for ( int i = 0; i < 3; i++ )
		_red[i] = (unsigned char)(RED[i]*255);

	unsigned int ridx = 0;
	for ( t_recordset::iterator it = m_ParaCoordData.records.begin(); it != m_ParaCoordData.records.end(); it++, ridx++ )
	{
		unsigned int sinum = si.size();
		for ( unsigned int i = 0; i < sinum; i++ )
		{
			for ( t_pickset::iterator psit = m_pickset.begin(); psit != m_pickset.end(); psit++ )
			{
				if ( (*psit).first == si[i] && (*psit).second == ((*it).first)[i] )
				{
					for ( unsigned int j = 0; j < sinum; j++ )
						memcpy( m_ParaCoordData.colordata+ridx*color_rs+j*3, _red, 3*sizeof( char ) );
					break;
				}
			}
		}
	}
}

void CDataManager::ParaCoord_PrepareRenderData()
{
	t_uintvector &si = m_ParaCoordData.selectedItems;
	float step = 0.8f / (si.size() - 1 );
	float start = 0.1f;

	if ( m_ParaCoordData.vertexdata != NULL )
		free( m_ParaCoordData.vertexdata );
	if ( m_ParaCoordData.indexdata != NULL )
		free( m_ParaCoordData.indexdata );
	if ( m_ParaCoordData.colordata != NULL )
		free( m_ParaCoordData.colordata );

	unsigned int vertex_rs = si.size()*3;
	unsigned int index_rs = (si.size()-1)*2;
	unsigned int color_rs = si.size()*3;
	m_ParaCoordData.vertexdata = (float *)malloc( m_ParaCoordData.records.size()*vertex_rs*sizeof(float));
	m_ParaCoordData.indexdata = (unsigned int *)malloc( m_ParaCoordData.records.size()*index_rs*sizeof(int));
	m_ParaCoordData.colordata = (unsigned char *) malloc( m_ParaCoordData.records.size()*color_rs*sizeof(char));

	unsigned int *pid = m_ParaCoordData.indexdata;

	vector<float> steps;
	steps.resize( si.size() );
	for ( int i = 0; i < si.size(); i++ )
		steps[i] = 0.9f / (m_item_desc[si[i]].num_values - 1);

	unsigned int ridx = 0;
	for ( t_recordset::iterator it = m_ParaCoordData.records.begin(); it != m_ParaCoordData.records.end(); it++, ridx++ )
	{
		float pos2[3] = {0.0f, 0.0f, 0.0f};
		
		unsigned int sinum = si.size();
		for ( unsigned int i = 0; i < sinum; i++ )
		{
			pos2[2] = 0.0f;
			for ( t_pickset::iterator psit = m_pickset.begin(); psit != m_pickset.end(); psit++ )
			{
				if ( (*psit).first == si[i] && (*psit).second == ((*it).first)[i] )
				{
					pos2[2] = 0.000001f;
					break;
				}
			}
			pos2[0] = start + i*step;
			pos2[1] = ((*it).first)[i] * steps[i] + 0.05f;
			memcpy( m_ParaCoordData.vertexdata+ridx*vertex_rs+i*3, pos2, 3*sizeof( float ) );
			if ( i > 0 )
			{
				*pid = ridx*si.size()+i-1;
				*(pid+1) = *pid+1;
				pid += 2;
			}	
		}
	}
	ParaCoord_PrepareRenderColorData();
}


void CDataManager::PrepareGraphData()
{
	if(!m_ifGraphReady)
		return;

	//clear the numEdges for the previous node information
	t_edge::iterator it; 
	/*
	unsigned int offset  = m_item_desc[m_Graph.selected_item[0]].num_values;
	for (it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		int id1 = (*it).first.first;
		int id2 = (*it).first.second;

		m_Graph.graph_node[id1].weight =1.0f;
		m_Graph.graph_node[id1].numEdge =0;
		m_Graph.graph_node[id2+offset].weight =1.0f;
		m_Graph.graph_node[id2+offset].numEdge =0;
	}
	*/


	pair<t_edge::iterator,bool> ret;
	int idx1 =m_Graph.selected_item[0];
	int idx2 =m_Graph.selected_item[1];

	m_edge_prepare.clear();
	
	clock_t start, finish;
	start = clock();
	for ( unsigned int i = m_startidx; i <= m_endidx; i++ )
	{
		t_intpair intpair;
		switch(m_item_desc[idx1].num_bytes)
		{
		case 1:
			intpair.first = *(unsigned char*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx1].offset);
			break;
		case 2:
			intpair.first = *(unsigned short*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx1].offset);
			break;
		case 4:
			intpair.first = *(unsigned int*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx1].offset);
			break;
		}

		switch(m_item_desc[idx2].num_bytes)
		{
		case 1:
			intpair.second = *(unsigned char*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx2].offset);
			break;
		case 2:
			intpair.second = *(unsigned short*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx2].offset);
			break;
		case 4:
			intpair.second = *(unsigned int*)(m_DataBuffer+i*m_TotalBytes+m_item_desc[idx2].offset);
			break;
		}

		ret = m_edge_prepare.insert(pair<t_intpair,unsigned int>(intpair,1));
		if(ret.second==false)
			m_edge_prepare[intpair] = m_edge_prepare[intpair]+1;
	}
	finish = clock();
	m_graphTime = (double)(finish - start);


	PrepareEdgeLayoutEdgeNode();

}

void CDataManager::PrepareEdgeLayoutEdgeNode()
{
	m_Graph.graph_edge_num = m_edge_prepare.size();
	if(m_Graph.graph_edge!=NULL)
	{
		cudaFree(m_Graph.graph_edge);
		free(m_Graph.graph_edge);
		m_Graph.graph_edge = NULL;
	}
	m_Graph.graph_edge = new t_GraphEdge[m_Graph.graph_edge_num*2];

	t_edge::iterator it;
	//unsigned int offset  = m_item_desc[m_Graph.selected_item[0]].num_values;

	if(m_Graph.graph_node != NULL)
	{
		cudaFree(m_Graph.graph_node);
		free(m_Graph.graph_node);
		m_Graph.graph_node = NULL;
	}


	t_ref id_x;
	t_ref id_y;
	pair<t_ref::iterator,bool> ret;
	
	for(it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		ret = id_x.insert(pair<unsigned int,unsigned int>((*it).first.first,1));
		if(ret.second == false)
			id_x[(*it).first.first] = id_x[(*it).first.first] + 1;

		ret = id_y.insert(pair<unsigned int,unsigned int>((*it).first.second,1));
		if(ret.second == false)
			id_y[(*it).first.second] = id_y[(*it).first.second] + 1;
		//DBOUT(" "<<(*it).first.first<<" "<<(*it).first.second<<"\n");
	}

	m_Graph.graph_node_num = id_x.size() +  id_y.size();

	srand ( time(NULL) );

 	m_Graph.graph_node = new t_GraphNode[m_Graph.graph_node_num];
	m_LayoutParameter.range = (float(m_Graph.graph_node_num));


	for(int i=0; i< m_Graph.graph_node_num; i++)
	{
		m_Graph.graph_node[i].weight =1.0f;
		m_Graph.graph_node[i].edge_ref =0;
		m_Graph.graph_node[i].pos[0] = (((float)rand())/RAND_MAX-0.5)*2.0;//*m_LayoutParameter.range;
		m_Graph.graph_node[i].pos[1] = (((float)rand())/RAND_MAX-0.5)*2.0;//*m_LayoutParameter.range;
		m_Graph.graph_node[i].stable = false;
		m_Graph.graph_node[i].begin =0;
		m_Graph.graph_node[i].numEdge =0;
	}


	map_x.clear();
	map_y.clear();
 

	t_ref::iterator node_it;
	unsigned int node_index = 0;
	for(node_it = id_x.begin(); node_it != id_x.end(); node_it++)
	{
		
		map_x.insert(pair<unsigned int, unsigned int>((*node_it).first,node_index));

		m_Graph.graph_node[node_index].kind = m_Graph.selected_item[0];
		m_Graph.graph_node[node_index].index = (*node_it).first;

		m_Graph.graph_node[node_index].numEdge = (*node_it).second;
		m_Graph.graph_node[node_index].weight = (*node_it).second;
		node_index++;

	}

	for(node_it = id_y.begin(); node_it != id_y.end(); node_it++)
	{
		
		map_y.insert(pair<unsigned int, unsigned int>((*node_it).first,node_index));

		m_Graph.graph_node[node_index].kind = m_Graph.selected_item[1];
		m_Graph.graph_node[node_index].index = (*node_it).first;
	
		m_Graph.graph_node[node_index].numEdge = (*node_it).second;
		m_Graph.graph_node[node_index].weight = (*node_it).second;
		node_index++;
	}

	for(unsigned int i=1;i<m_Graph.graph_node_num;i++)
		m_Graph.graph_node[i].begin = m_Graph.graph_node[i-1].begin + m_Graph.graph_node[i-1].numEdge; 




	unsigned int index =0;  
	/*
	for (it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		int id1 = (*it).first.first;
		int id2 = (*it).first.second;

		m_Graph.graph_node[id1].weight +=(*it).second;
		m_Graph.graph_node[id1].numEdge++;
		m_Graph.graph_node[id2+offset].weight +=(*it).second;
		m_Graph.graph_node[id2+offset].numEdge++;
	}

	 
	index=0;*/

	t_edge edge_tmp;
	t_intpair intpair;
	for (it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		m_Graph.graph_edge[index].node = map_y[(*it).first.second];
		m_Graph.graph_edge[index].weight =(*it).second;
		index++;

		intpair.first = (*it).first.second;
		intpair.second = (*it).first.first;
		edge_tmp.insert(pair<t_intpair,unsigned int>(intpair,(*it).second));
	}

	for (it = edge_tmp.begin();it!=edge_tmp.end();it++)
	{
		m_Graph.graph_edge[index].node = map_x[(*it).first.second];
		m_Graph.graph_edge[index].weight =(*it).second;
		index++;
	}

	/*
	for(int i=0;i<m_Graph.graph_node_num;i++)
	{
		DBOUT("ID:"<<i<<",trueId:"<<m_Graph.graph_node[i].index<<",NumEdge:"<<m_Graph.graph_node[i].numEdge<<",kind:"<<m_Graph.graph_node[i].kind<<",Begin:"<<m_Graph.graph_node[i].begin<<"\n");
	}

	for(int i=0;i<m_Graph.graph_edge_num*2;i++)
	{
		DBOUT("id:"<<i<<",id"<<m_Graph.graph_edge[i].node<<"\n");
	}
	DBOUT("END");
	*/

/*
	for (it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		m_Graph.graph_edge[index].node = (*it).first.second+offset;
		m_Graph.graph_edge[index].weight =(*it).second;
		m_Graph.graph_edge[index+m_Graph.graph_edge_num].node = (*it).first.first;
		m_Graph.graph_edge[index+m_Graph.graph_edge_num].weight =(*it).second;
		index++;
	}
	*/

}


void CDataManager::CreateGraph()
{
	if(m_Graph.graph_node!=NULL)
	{
		cudaFree(m_Graph.graph_node);
		m_Graph.graph_node = NULL;
		m_edge_prepare.clear();
	}
	if(m_Graph.graph_edge!=NULL)
	{
		cudaFree(m_Graph.graph_edge);
	}
/*
	pair<t_edge::iterator,bool> ret;
	int idx1 =m_Graph.selected_item[0];
	int idx2 =m_Graph.selected_item[1];

	//if(idx1==5&&idx2==6)
	//	m_Graph.kind = 1;
	//else
	m_Graph.kind = 0;

	m_Graph.graph_node_num  = m_item_desc[idx1].num_values + m_item_desc[idx2].num_values;
	
	srand ( time(NULL) );

 	m_Graph.graph_node = new t_GraphNode[m_Graph.graph_node_num];
	m_LayoutParameter.range = (float)sqrt(float(m_Graph.graph_node_num));
	
	for (int i=0;i<m_item_desc[idx1].num_values;i++)
	{
		m_Graph.graph_node[i].kind = idx1;
		m_Graph.graph_node[i].index = i;
		//	m_Graph.graph_node[i].name ="";
		m_Graph.graph_node[i].weight =1.0f;
		m_Graph.graph_node[i].edge_ref =0;
		m_Graph.graph_node[i].pos[0] = (((float)rand())/RAND_MAX-0.5)*2.0*m_LayoutParameter.range;
		m_Graph.graph_node[i].pos[1] = (((float)rand())/RAND_MAX-0.5)*2.0*m_LayoutParameter.range;
		m_Graph.graph_node[i].stable = false;
		m_Graph.graph_node[i].begin =0;
		m_Graph.graph_node[i].numEdge =0;

	}

	int offset = m_item_desc[idx1].num_values;
	for (int i=0;i<m_item_desc[idx2].num_values;i++)
	{
		m_Graph.graph_node[i+offset].kind = idx2;
		m_Graph.graph_node[i+offset].index = i;
		//	m_Graph.graph_node[i+offset].name ="";
		m_Graph.graph_node[i+offset].weight =1.0f;
		m_Graph.graph_node[i+offset].edge_ref =0;
		m_Graph.graph_node[i+offset].pos[0] = (((float)rand())/RAND_MAX-0.5)*2.0*m_LayoutParameter.range;
		m_Graph.graph_node[i+offset].pos[1] = (((float)rand())/RAND_MAX-0.5)*2.0*m_LayoutParameter.range;
		m_Graph.graph_node[i+offset].stable = false;
		m_Graph.graph_node[i+offset].begin =0;
		m_Graph.graph_node[i+offset].numEdge =0;
	}
	*/

	m_ifGraphReady = true;

}

void CDataManager::GraphLayout()
{
	set<int> id_x;
	set<int> id_y;

	t_edge::iterator it;



	/*
	for (it = m_edge_prepare.begin();it!=m_edge_prepare.end();it++)
	{
		int id1 = (*it).first.first;
		int id2 = (*it).first.second;

		m_Graph.graph_node[id1].weight +=(*it).second;
		m_Graph.graph_node[id1].numEdge++;
		m_Graph.graph_node[id2+offset].weight +=(*it).second;
		m_Graph.graph_node[id2+offset].numEdge++;
	}
	*/




	m_LayoutParameter.numEdge = m_Graph.graph_edge_num;
	m_LayoutParameter.numNode= m_Graph.graph_node_num;

	float repSum  = 2.0*m_Graph.graph_edge_num;
	float density =1.0/repSum;

	m_LayoutParameter.repFactor = density*pow((float)repSum,(float)(0.5)*(m_LayoutParameter.attExponent-m_LayoutParameter.repExponent));
	m_LayoutParameter.graFactor = density*repSum*pow(m_LayoutParameter.graFactor_previous,(m_LayoutParameter.attExponent-m_LayoutParameter.repExponent));
	m_LayoutParameter.baryCenter[0] = m_LayoutParameter.baryCenter[1] =0.0f; 	

 	kernel_function(&m_LayoutParameter,m_Graph.graph_node,m_Graph.graph_edge);

	for (int i=0;i<m_Graph.graph_node_num;i++)
	{
		if (m_Graph.graph_node[i].pos[0]>m_Graph.boundary[1])
			m_Graph.boundary[1]=m_Graph.graph_node[i].pos[0];
		else if(m_Graph.graph_node[i].pos[0]<m_Graph.boundary[0])
			m_Graph.boundary[0]=m_Graph.graph_node[i].pos[0];

		if (m_Graph.graph_node[i].pos[1]>m_Graph.boundary[1])
			m_Graph.boundary[1]=m_Graph.graph_node[i].pos[1];
		else if(m_Graph.graph_node[i].pos[1]<m_Graph.boundary[0])
			m_Graph.boundary[0]=m_Graph.graph_node[i].pos[1];
	}

	m_Graph.boundary[2] = m_Graph.boundary[0]+m_Graph.boundary[1]; 
	if(m_Graph.boundary[2]>0)
		m_Graph.boundary[2] = m_Graph.boundary[1];
	else
		m_Graph.boundary[2] = 0- m_Graph.boundary[0];
}

bool CDataManager::LoadGraph(char *filename)
{
//	wxLogMessage("%s",filename);
	
	FILE * loadfile;
	loadfile = fopen(filename,"rb");
	unsigned char tmp_char;
	unsigned int  tmp_int;
	float		  tmp_float;

	fread(&tmp_char,1,1,loadfile);
	m_Graph.selected_item[0] = tmp_char;
	fread(&tmp_char,1,1,loadfile);
	m_Graph.selected_item[1] = tmp_char;
	fread(&tmp_float,1,sizeof(float),loadfile);
	m_LayoutParameter.range = tmp_float;

	//if(m_Graph.graph_node!=NULL)
	{
		cudaFree(m_Graph.graph_node);
		m_Graph.graph_node = NULL;
		m_edge_prepare.clear();
	}
	//if(m_Graph.graph_edge!=NULL)
	{
		cudaFree(m_Graph.graph_edge);
	}

	pair<t_edge::iterator,bool> ret;
	int idx1 =m_Graph.selected_item[0];
	int idx2 =m_Graph.selected_item[1];

	m_Graph.kind = 0;

	m_Graph.graph_node_num  = m_item_desc[idx1].num_values + m_item_desc[idx2].num_values;

	m_Graph.graph_node = new t_GraphNode[m_Graph.graph_node_num];

	for (int i=0;i<m_item_desc[idx1].num_values;i++)
	{
		m_Graph.graph_node[i].kind = idx1;
		m_Graph.graph_node[i].index = i;
		m_Graph.graph_node[i].weight =1.0f;
		m_Graph.graph_node[i].edge_ref =0;
		fread(&(m_Graph.graph_node[i].pos[0]),1,sizeof(float),loadfile);
		fread(&(m_Graph.graph_node[i].pos[1]),1,sizeof(float),loadfile);
	 	m_Graph.graph_node[i].stable = false;
	}

	int offset = m_item_desc[idx1].num_values;
	for (int i=0;i<m_item_desc[idx2].num_values;i++)
	{
		m_Graph.graph_node[i+offset].kind = idx2;
		m_Graph.graph_node[i+offset].index = i;
		m_Graph.graph_node[i+offset].weight =1.0f;
		m_Graph.graph_node[i+offset].edge_ref =0;
		fread(&(m_Graph.graph_node[i+offset].pos[0]),1,sizeof(float),loadfile);
		fread(&(m_Graph.graph_node[i+offset].pos[1]),1,sizeof(float),loadfile);
		m_Graph.graph_node[i+offset].stable = false;
	}
	m_ifGraphReady = true;
	return true;
}
bool CDataManager::SaveGraph(char *filename)
{
//	wxLogMessage("%s",filename);
	FILE* savefile;
	savefile = fopen(filename,"wb");
	unsigned char tmp_char;
	unsigned int  tmp_int;
	float		  tmp_float;

	tmp_char =(unsigned char)m_Graph.selected_item[0];
	fwrite(&tmp_char,1,1,savefile);
	tmp_char =(unsigned char)m_Graph.selected_item[1];
	fwrite(&tmp_char,1,1,savefile);
	tmp_float = m_LayoutParameter.range;
	fwrite(&tmp_float,sizeof(float),1,savefile);

	for (int i=0;i<m_Graph.graph_node_num;i++)
	{
		tmp_float = m_Graph.graph_node[i].pos[0];
		fwrite(&tmp_float,sizeof(float),1,savefile);
		tmp_float = m_Graph.graph_node[i].pos[1];
		fwrite(&tmp_float,sizeof(float),1,savefile);
	}

	fflush(savefile);
	fclose(savefile);

	return true;
}

}//end of namespace