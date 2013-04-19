#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <GL/glew.h>
#include "GraphFrame.h"
#include "datamanager.h"
#include "ControlFrame.h"
#include "object.h"
//#include "GL/glu.h"
#include "GL/freeglut.h"
#include "glColor.h"
#include "glhelper.h"

#define ONE_SECOND 1



namespace AMT
{

 
enum
{
    ID_QUIT  = wxID_EXIT,
};

BEGIN_EVENT_TABLE(CGraphFrame, wxFrame)
    EVT_MENU(ID_QUIT, CGraphFrame::OnExit)
END_EVENT_TABLE()


CGraphFrame *CGraphFrame::Create( const wxString& title, const wxPoint& pos,
										 const wxSize& size, long style )
{
	CGraphFrame *rf = new CGraphFrame(title, pos, size, style);
	
    rf->Show(true);

    return rf;
}

CGraphFrame::~CGraphFrame()
{
	if ( m_canvas != NULL )
		delete m_canvas;

	for ( int i = 0; i < m_renderObjectList.size(); i++ )
		m_renderObjectList[i]->destroyMyself();
}

void CGraphFrame::AddRenderObject( CRenderObject *ro )
{
	m_renderObjectList.push_back( ro );
}

CGraphFrame::CGraphFrame(const wxString& title, const wxPoint& pos,
						   const wxSize& size, long style)
						   : wxFrame( NULL, wxID_ANY, title, pos, size, style ), m_cf(NULL)
{
    wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(ID_QUIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

	m_canvas = new CGraphCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth() ));

	m_renderObjectList.resize(0);
}

void CGraphFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    Close(true);
}

BEGIN_EVENT_TABLE(CGraphCanvas, wxGLCanvas)
    EVT_SIZE(CGraphCanvas::OnSize)
    EVT_PAINT(CGraphCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(CGraphCanvas::OnEraseBackground)
	EVT_KEY_DOWN(CGraphCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(CGraphCanvas::OnMouse)
	EVT_IDLE(CGraphCanvas::OnIdle) 
END_EVENT_TABLE()

CGraphCanvas::CGraphCanvas( wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
	rf = (CGraphFrame*) parent;
    m_init = false;
	m_hDC = NULL;
 
	node_size = 0.0015;
	m_node = -1;

	m_if_dragging = false;
	m_if_selecting = false;
	m_if_choseDrag = false;

	m_Ortho_backup[0]=-1;
	m_Ortho_backup[1]=1;
	m_Ortho_backup[2]=-1;
	m_Ortho_backup[3]=1;

	m_showEdge = true;
	InitOrtho();

}

CGraphCanvas::~CGraphCanvas()
{
	KillFont();
}

GLvoid CGraphCanvas::BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	m_base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	-12,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Arial");					// Font Name

	oldfont = (HFONT)SelectObject(m_hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(m_hDC, 32, 96, m_base);				// Builds 96 Characters Starting At Character 32
	SelectObject(m_hDC, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
}

GLvoid CGraphCanvas::KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(m_base, 96);							// Delete All 96 Characters
}

GLvoid CGraphCanvas::glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(m_base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

 


void CGraphCanvas::InitGL()
{
	SetCurrent();
	GLenum err = glewInit();
	if (GLEW_OK != err)
		wxLogError( "%s\n", glewGetErrorString(err) );

	if ( GLEW_ARB_vertex_buffer_object )
		wxLogMessage( "VBO extersion is supported!\n");
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
		wxLogMessage("Ready for GLSL - vertex, fragment, and geometry units.\n");
	else {
		wxLogError("Not totally ready :( \n");
	}

	glClearColor( WHITE[0], WHITE[1], WHITE[2], WHITE[3] );
	glClearDepth(1.0);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
 	glEnable(GL_NORMALIZE);


	HWND hWnd = (HWND) GetHandle();
	m_hDC = GetDC(hWnd);

	BuildFont();
}

void CGraphCanvas::GetColor(string str)
{
	if(str.compare("10.32.0.1")==0)
		glColor3f(1.0,0.0,0.0);
	else if(str.compare("172.23.0.1")==0)
		glColor3f(0.0,1.0,0.0);
	else if(str.compare("10.32.0.100")==0)
		glColor3f(0.0,0.0,1.0);
	else if(str.compare("172.25.0.1")==0)
		glColor3f(0.5,0.5,0.0);
	else if (str.compare("10.99.99.2")==0)
		glColor3f(0.5,0.0,0.5);
	else if(str.compare("172.23.0.10")==0)
		glColor3f(0.0,0.5,0.5);
	else if(str.compare("172.23.0.2")==0)
		glColor3f(0.5,0.25,0.25);
	else if(str.compare("10.32.1.100")==0)
		glColor3f(0.25,0.5,0.25);


	else if(str.compare("198.41.0.4")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("128.9.0.107")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("129.33.4.12")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("128.8.10.90")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("192.203.230.10")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("192.5.5.241")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("192.112.36.4")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("128.63.2.53")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("192.36.148.17")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("192.58.128.30")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("193.0.14.129")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("198.32.64.12")==0)
		glColor3f(0.75,0.75,0.25);
	else if(str.compare("202.12.27.33")==0)
		glColor3f(0.75,0.75,0.25);

	else if(str.compare("10.32.2.100")==0)
		glColor3f(0.75,0.25,0.25);
	else if(str.compare("10.32.2.101")==0)
		glColor3f(0.75,0.25,0.25);

	else 
	{
		string temp;
		size_t found;
		found=str.find_last_of(".");
		temp = str.substr(0,found);
		str  = str.substr(found+1);
		if(temp.compare("10.32.0")==0)
		{
			int ip_last  = atoi(str.c_str());
			if(ip_last>=201&&ip_last<=210)
				glColor3f(0.25,0.5,0.25);
			else
				glColor3f(0.3,0.3,0.3);
		}
		else if(temp.compare("10.32.1")==0)
		{
			int ip_last  = atoi(str.c_str());
			if(ip_last>=201&&ip_last<=206)
				glColor3f(0.25,0.5,0.25);
			else
				glColor3f(0.3,0.3,0.3);
		}

		else if(temp.compare("10.32.5")==0)
		{
			int ip_last  = atoi(str.c_str());
			if(ip_last>=1&&ip_last<=254)
				glColor3f(0.25,0.5,0.25);
			else
				glColor3f(0.3,0.3,0.3);

		}

		else if(temp.compare("172.23.214")==0)
			glColor3f(0.25,0.25,0.75);
		else if(temp.compare("172.23.229")==0)
			glColor3f(0.25,0.25,0.75);
		else
		{
			string temp_str;
			size_t found;
			found=temp.find_last_of(".");
			temp_str = temp.substr(0,found);

			if(temp_str.compare("172.23")==0)
				glColor3f(0.75,0.25,0.75);
			else
				glColor3f(0.3,0.3,0.3);

		}
	}

}


void CGraphCanvas::DrawGeneralGraph()
{
	float temp_float[2];
	CDataManager *dm = GetDataManager();
	//glEnable(GL_COLOR);
	node_size = dm->m_LayoutParameter.nodeSize;
	//node_size =m_Ortho[1]*0.005;
	float temp_node_size  = node_size;
	//temp_node_size =0.002;
	float ratio = (m_Ortho[1]-m_Ortho[0])/(m_Ortho[3]-m_Ortho[2]);
	ratio =1.0/ratio;

	//recalculate the node position
	if(dm->m_Graph.if_layout)
	{
		dm->GraphLayout();
	}


	glPointSize( dm->m_LayoutParameter.nodeSize*30 );
	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBegin( GL_POINTS );
	for(int i=0;i<(dm->m_Graph).graph_node_num;i++)
	{
		temp_float[0] = (dm->m_Graph).graph_node[i].pos[0];
		temp_float[1] = (dm->m_Graph).graph_node[i].pos[1];

 		if((dm->m_Graph).graph_node[i].kind == dm->m_Graph.selected_item[0])
 			glColor3fv(RED);
 		else if((dm->m_Graph).graph_node[i].kind == dm->m_Graph.selected_item[1])
			glColor3fv(BLUE);
		glVertex3f(temp_float[0], temp_float[1], 0);
	}		 
	glEnd();

	if(dm->m_Graph.node_pin == true)
	{
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{
			 
			unsigned int offset;
			if((*psit).first==dm->m_Graph.selected_item[0])
			{
				offset = dm->map_x[(*psit).second];
				 
			}
			else if((*psit).first==dm->m_Graph.selected_item[1])
			{
				offset = dm->map_y[(*psit).second];
				 
			}

			dm->m_Graph.graph_node[offset].stable = true;
		}
	}
	else if(dm->m_Graph.node_pin == false)
	{
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{
			 
			unsigned int offset;
			if((*psit).first==dm->m_Graph.selected_item[0])
			{
				offset = dm->map_x[(*psit).second];
				 
			}
			else if((*psit).first==dm->m_Graph.selected_item[1])
			{
				offset = dm->map_y[(*psit).second];
				 
			}

			dm->m_Graph.graph_node[offset].stable = false;
		}
	}
	

	if(dm->m_Graph.edge_show){
 		if(dm->m_edge_prepare.size()>0)
		{
			glLineWidth(0.5);
			t_edge::iterator it;
			const float *color_node1 = GRAY;
			const float *color_node2 = GRAY;
			bool node1_find= false;
			bool node2_find = false;
			for (it=dm->m_edge_prepare.begin() ;it!=dm->m_edge_prepare.end();it++)
			{

				int id1 = dm->map_x[(*it).first.first];
				int id2 = dm->map_y[(*it).first.second];

				unsigned int offset  = 0;//dm->m_item_desc[(dm->m_Graph).selected_item[0]].num_values;
				glBegin(GL_LINES);
 				if((dm->m_Graph).graph_node[id1].edge_ref ==3)
 					glColor4fv(RED_TRANSPART);
 				else
					glColor4fv(GRAY_TRANSPART);
				glVertex3f((dm->m_Graph).graph_node[id1].pos[0], (dm->m_Graph).graph_node[id1].pos[1], 0);
				if((dm->m_Graph).graph_node[id2+offset].edge_ref ==3)
					glColor4fv(RED_TRANSPART);
				else
					glColor4fv(GRAY_TRANSPART);
				glVertex3f((dm->m_Graph).graph_node[id2+offset].pos[0], (dm->m_Graph).graph_node[id2+offset].pos[1], 0);
				glEnd();
				//(dm->m_Graph).graph_node[id1].edge_ref = (dm->m_Graph).graph_node[id2+offset].edge_ref =0;

			}
		}
	}
	if(dm->m_Graph.edge_show != true)
	{
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{
			float pos[2];
			float pos2[2];
			int offset;
			int kind, index;

			if((*psit).first==dm->m_Graph.selected_item[0])
			{
				offset = dm->map_x[(*psit).second];
				kind = 0;
			}
			else if((*psit).first==dm->m_Graph.selected_item[1])
			{
				offset = dm->map_y[(*psit).second];
				kind = 1;
			}

			pos[0] = dm->m_Graph.graph_node[offset].pos[0];
			pos[1] = dm->m_Graph.graph_node[offset].pos[1];
			
			for(int i=0; i < dm->m_Graph.graph_node[offset].numEdge; i++)
			{
				glBegin(GL_LINES);
				glColor4fv(RED_TRANSPART);
				glVertex3f(pos[0],pos[1],0);
				glColor4fv(GRAY_TRANSPART);

				int edgeIndex = dm->m_Graph.graph_edge[i+dm->m_Graph.graph_node[offset].begin].node;
				pos2[0] = dm->m_Graph.graph_node[edgeIndex].pos[0];
				pos2[1] = dm->m_Graph.graph_node[edgeIndex].pos[1];
				glVertex3f(pos2[0],pos2[1],0);
				glEnd();

				glPushMatrix();
				glTranslatef(pos2[0],pos2[1],0);
				char str[64];
				
				 
				index = dm->m_Graph.graph_node[edgeIndex].index;
				 
				sprintf( str, "%s",dm->m_item_desc[dm->m_Graph.selected_item[1-kind]].value_names[index].c_str());
				glRasterPos2f( 0, 0.0 );
				glPrint( "%s", str );
				glPopMatrix();
			}
			
			
		}

	}

	for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
	{
		glPushMatrix();
		float pos[2];
		int offset;
		int kind;
		if((*psit).first==dm->m_Graph.selected_item[0])
		{
			offset = dm->map_x[(*psit).second];
			kind = dm->m_Graph.selected_item[0];
		}
		else
		{
			offset = dm->map_y[(*psit).second];
			kind = dm->m_Graph.selected_item[1];
		}

		pos[0] = dm->m_Graph.graph_node[offset].pos[0];
		pos[1] = dm->m_Graph.graph_node[offset].pos[1];
		unsigned int index = dm->m_Graph.graph_node[offset].index;
		dm->m_Graph.graph_node[offset].edge_ref =3;
		glColor3fv(GREEN);
		glTranslatef(pos[0],pos[1],0);
		char str[64];
		sprintf( str, "%s",dm->m_item_desc[kind].value_names[index].c_str());
		glRasterPos2f( 0, 0.0 );
		glPrint( "%s", str );
		glPopMatrix();
	}


	char str[255];
	sprintf( str, "Red Node:%s,Num:%d",dm->m_item_desc[dm->m_Graph.selected_item[0]].name.c_str(),dm->m_Graph.graph_node_num);
	glPushMatrix();
	glTranslatef(m_Ortho[0]*0.95,m_Ortho[1]*0.95,0);
	glColor4fv( RED );
	glRasterPos2f( 0, 0 );
	glPrint( "%s", str );
	glPopMatrix();


	sprintf( str, "Blue Node:%s,Num:%d",dm->m_item_desc[dm->m_Graph.selected_item[1]].name.c_str(),dm->m_Graph.graph_edge_num);
	glPushMatrix();
	glTranslatef(0.0,m_Ortho[1]*0.95,0);
	glColor4fv( BLUE );
	glRasterPos2f( 0, 0 );
	glPrint( "%s", str );
	glPopMatrix();


}

 
void CGraphCanvas::DrawExtraInfo()
{
	if ( m_if_dragging || m_if_selecting)
	{
		if ( m_if_dragging )
			glColor4fv( BLACK );
		else
			glColor4fv( RED );

		glBegin( GL_LINE_LOOP );
		glVertex3f( m_newOrtho[0], m_newOrtho[1], 0.9 );
		glVertex3f( m_newOrtho[0], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[1], 0.9 );
		glEnd();
	}
}

void CGraphCanvas::DrawRenderingStatus()
{
	CDataManager *dm =  GetDataManager();
	glColor4fv(BLACK);

	if ( dm->m_ifDataReady )
	{
		glPushMatrix();
		glTranslatef(m_Ortho[0]*0.95,m_Ortho[0]*0.95,0);
		glColor4fv( LIGHT_GRAY );
		glRasterPos2f( 0, 0 );
		glPrint( "%s", strFrameRate );
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,m_Ortho[0]*0.95,0);
		char str[64];
		sprintf( str, "Mining Time: %d ms", dm->m_graphTime );
		glRasterPos2f( 0, 0 );
		glPrint( "%s", str );
		glPopMatrix();
	}

}

void CGraphCanvas::Render()
{

	int w = m_size.x, h = m_size.y;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);

	CDataManager *dm =  GetDataManager();
	
	//m_Ortho[0] =m_Ortho[2] =0- dm->m_LayoutParameter.range;
	//m_Ortho[1] =m_Ortho[3] =dm->m_LayoutParameter.range;
 
	glOrtho( m_Ortho[0], m_Ortho[1], m_Ortho[2], m_Ortho[3], m_Ortho[4], m_Ortho[5] );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	if(dm->m_ifGraphReady)
	{
		DrawExtraInfo();
			
		DrawGeneralGraph();

		CalculateFrameRate();
		DrawRenderingStatus();

		glFlush();
		SwapBuffers();
	}
	 
}


void CGraphCanvas::OnIdle( wxIdleEvent& event )
{ 
	Render();
	Refresh(false);
}

void CGraphCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
//	wxLogMessage("CGLCanvas::OnPaint has been called.");

	wxPaintDC dc(this);

	if ( !GetContext() )
		return;
	SetCurrent();
	// Init OpenGL once, but after SetCurrent
	if (!m_init)
	{
		InitGL();
		m_init = true;
	}

    Render();
}



void CGraphCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
//    wxGLCanvas::OnSize(event);
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
    m_size.SetHeight( h );
	m_size.SetWidth( w );

	glViewport(0, 0, w, h);
}

void CGraphCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void CGraphCanvas::OnKeyDown( wxKeyEvent& event )
{
	SetCurrent();
	char ch = event.GetKeyCode();
	CDataManager *dm = GetDataManager();
	if(ch =='p'||ch =='P')
	{
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{
			float pos[2];
			int offset;
			if((*psit).first==dm->m_Graph.selected_item[0])
				offset =0;
			else
				offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
			dm->m_Graph.graph_node[offset+(*psit).second].stable = true;
		}

	}
	if ( ch == ' ' )
	{
		//InitOrtho();
		
		m_Ortho[0] =m_Ortho[2] =0- dm->m_LayoutParameter.range;
		m_Ortho[1] =m_Ortho[3] =dm->m_LayoutParameter.range;
		//dm->m_pickset.clear();
		Refresh(false);
	}
	if(ch =='e'||ch =='E')
	{
		if(m_showEdge == false)
			m_showEdge = true;
		else
			m_showEdge = false;
	}
	if ( ch =='d'||ch =='D')
	{
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{
			float pos[2];
			int offset;
			if((*psit).first==dm->m_Graph.selected_item[0])
				offset =0;
			else
				offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
			dm->m_Graph.graph_node[offset+(*psit).second].edge_ref =0;
			dm->m_Graph.graph_node[offset+(*psit).second].stable = false;
		}

		dm->m_pickset.clear();
		GetDataManager()->ParaCoord_PrepareRenderColorData();
		rf->m_cf->RefreshAll();
	}
	if (ch =='c'||ch =='C')
	{
		m_Ortho[0]=-3;
		m_Ortho[1]=3;
		m_Ortho[2]=-3;
		m_Ortho[3]=3;
		m_Ortho[4]=m_Ortho[4]*3;
		m_Ortho[5]=m_Ortho[5]*3;
	}


	Refresh(false);
}

void CGraphCanvas::OnMouse( wxMouseEvent& event )
{

	int mouse_x = event.GetX();
	int mouse_y = event.GetY();


	if(event.LeftDClick())
	{
		//if ( !m_if_dragging )
		{
			SetCurrent();
			GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
			Refresh(false);
			m_if_choseDrag = true;
		}
	}
	else if(event.RightUp())
	{
 		m_if_choseDrag = false;
// 		m_node = -1;
//		GetWorldCoordinate( m_newOrtho_pre[2], m_newOrtho_pre[3], mouse_x, mouse_y );
	}
	else if(event.RightDown())
	{
	//	GetWorldCoordinate( m_newOrtho_pre[2], m_newOrtho_pre[3], mouse_x, mouse_y );
	}
 	
	else if(event.Dragging()&&event.RightIsDown())
	{
// 		if(m_if_choseDrag==false)
// 		{
// 			GetWorldCoordinate( m_newOrtho_pre[2], m_newOrtho_pre[3], mouse_x, mouse_y );
// 			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
// 			m_if_choseDrag = true;
// 			Refresh(false);
// 			return;
// 		}

		GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
		CDataManager *dm = GetDataManager();
		for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
		{

			//float pos[2];
			int offset;
			if((*psit).first==dm->m_Graph.selected_item[0])
			{
				offset = dm->map_x[(*psit).second];
				//kind = 0;
			}
			else if((*psit).first==dm->m_Graph.selected_item[1])
			{
				offset = dm->map_y[(*psit).second];
				//kind = 1;
			}

			//pos[0] = dm->m_Graph.graph_node[offset].pos[0];
			//pos[1] = dm->m_Graph.graph_node[offset].pos[1];

			
			
			dm->m_Graph.graph_node[offset].pos[0] +=(m_newOrtho[2]-m_newOrtho_pre[2]);
		    dm->m_Graph.graph_node[offset].pos[1] +=(m_newOrtho[3]-m_newOrtho_pre[3]);
		}
		GetWorldCoordinate( m_newOrtho_pre[2], m_newOrtho_pre[3], mouse_x, mouse_y );
		Refresh();

// 		if(m_node>=0)
// 		{
// 			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
// 			CDataManager *dm =  GetDataManager();
// 			dm->m_Graph.graph_node[m_node].pos[0] = m_newOrtho[2];
// 			dm->m_Graph.graph_node[m_node].pos[1] = m_newOrtho[3];
// 			
// 			m_if_choseDrag = false;
// 		}
	}

	if ( event.MiddleDown())
	{
		if ( !m_if_dragging )
		{
			SetCurrent();
			GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
			m_newOrtho[2] = m_newOrtho[0]; m_newOrtho[3] = m_newOrtho[1];
		}
	}
	else if ( event.Dragging() && event.MiddleIsDown() )
	{
		m_if_dragging = true;
		GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
		Refresh(false);
	}
	else if ( event.MiddleUp() )
	{
		if ( m_if_dragging )
		{
			m_if_dragging = false;
			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
			if ( m_newOrtho[0] > m_newOrtho[2] )
			{
				m_Ortho[0] = m_newOrtho[2];
				m_Ortho[1] = m_newOrtho[0];
			}
			else
			{
				m_Ortho[0] = m_newOrtho[0];
				m_Ortho[1] = m_newOrtho[2];
			}
			if ( m_newOrtho[1] > m_newOrtho[3] )
			{
				m_Ortho[2] = m_newOrtho[3];
				m_Ortho[3] = m_newOrtho[1];
			}
			else
			{
				m_Ortho[2] = m_newOrtho[1];
				m_Ortho[3] = m_newOrtho[3];
			}
			Refresh(false);
		}
	}

	if ( event.LeftDown())
	{
		if ( !m_if_selecting )
		{
			SetCurrent();
			GetWorldCoordinate( m_newOrtho[0], m_newOrtho[1], mouse_x, mouse_y );
			m_newOrtho[2] = m_newOrtho[0]; m_newOrtho[3] = m_newOrtho[1];
		}
	}
	else if ( event.Dragging() && event.LeftIsDown() )
	{
		m_if_selecting = true;
		GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
		Refresh(false);
	}
	else if ( event.LeftUp() )
	{
		if ( m_if_selecting )
		{
			m_if_selecting = false;
			GetWorldCoordinate( m_newOrtho[2], m_newOrtho[3], mouse_x, mouse_y );
			if ( m_newOrtho[0] > m_newOrtho[2] )
			{
				float tmp = m_newOrtho[0];
				m_newOrtho[0] = m_newOrtho[2];
				m_newOrtho[2] = tmp;
			}

			if ( m_newOrtho[1] > m_newOrtho[3] )
			{
				float tmp = m_newOrtho[3];
				m_newOrtho[3] = m_newOrtho[1];
				m_newOrtho[1] = tmp;
			}
				SelectData();

 				m_newOrtho_pre[2] = m_newOrtho[2] = (m_newOrtho[0] + m_newOrtho[2])*0.5f;
 				m_newOrtho_pre[3] = m_newOrtho[3] = (m_newOrtho[1] + m_newOrtho[3])*0.5f;
				
		}
	//	GetWorldCoordinate( m_newOrtho_pre[2], m_newOrtho_pre[3], mouse_x, mouse_y );
	//	Refresh(false);
		rf->m_cf->RefreshAll();
	}


}


void CGraphCanvas::SelectData()
{
	CDataManager *dm = GetDataManager();

	for (int i=0;i<dm->m_Graph.graph_node_num;i++)
	{
		float value_x  = dm->m_Graph.graph_node[i].pos[0];
		float value_y  = dm->m_Graph.graph_node[i].pos[1];
		if( (value_x>m_newOrtho[0]&&value_x<m_newOrtho[2])&&(value_y>m_newOrtho[1]&&value_y<m_newOrtho[3]))
		{
			pair<t_pickset::iterator, bool> ret;
			ret = dm->m_pickset.insert( pair<unsigned int, unsigned int> (dm->m_Graph.graph_node[i].kind, dm->m_Graph.graph_node[i].index ) );
		//	dm->m_Graph.graph_node[i].stable = true;
			dm->m_Graph.graph_node[i].edge_ref = 3;//indicate this node is in the selected node;

			
			if ( ret.second == false )
			{
				dm->m_pickset.erase( ret.first );
				dm->m_Graph.graph_node[i].stable = false;
				dm->m_Graph.graph_node[i].edge_ref = 0;
			}
			//break ;
		}
	}
// 	printf("%d, %d",range[0],range[1]);
	GetDataManager()->ParaCoord_PrepareRenderColorData();
	rf->m_cf->RefreshAll();
}

void CGraphCanvas::CalculateFrameRate()
{
	SetCurrent();
    long cur_t = (long)((long)(clock()) / 1000);

    // Increase the frame counter
    ++framesPerSecond;

    if( cur_t - lastTime >= ONE_SECOND )
    {
        lastTime = cur_t;

        // Copy the frames per second into a string to display in the window
        sprintf(strFrameRate, "Frames Rate: %d", framesPerSecond);

        // Reset the frames per second
        framesPerSecond = 0;
    }
}

CDataManager *CGraphCanvas::GetDataManager() 
{ 
	return rf->m_cf->GetDataManager(); 
}


} // end of namespace
