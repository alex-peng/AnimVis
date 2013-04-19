#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <GL/glew.h>
#include "ParaCoordFrame.h"
#include "datamanager.h"
#include "ControlFrame.h"
#include "glColor.h"
#include "glhelper.h"
#include "shaderhelper.h"
#include "GL/freeglut.h"

#define ONE_SECOND 1

namespace AMT
{

enum
{
    ID_QUIT  = wxID_EXIT,
};

BEGIN_EVENT_TABLE(CParaCoordFrame, wxFrame)
    EVT_MENU(ID_QUIT, CParaCoordFrame::OnExit)
END_EVENT_TABLE()


CParaCoordFrame *CParaCoordFrame::Create( const wxString& title, const wxPoint& pos,
										 const wxSize& size, long style )
{
	CParaCoordFrame *rf = new CParaCoordFrame(title, pos, size, style);
	
    rf->Show(true);

    return rf;
}

CParaCoordFrame::~CParaCoordFrame()
{
	if ( m_canvas != NULL )
		delete m_canvas;
}

CParaCoordFrame::CParaCoordFrame(const wxString& title, const wxPoint& pos,
						   const wxSize& size, long style)
						   : wxFrame( NULL, wxID_ANY, title, pos, size, style ), m_cf(NULL)
{
    wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(ID_QUIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

	m_canvas = new CParaCoordCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth() ));
}

void CParaCoordFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    Close(true);
}

BEGIN_EVENT_TABLE(CParaCoordCanvas, wxGLCanvas)
    EVT_SIZE(CParaCoordCanvas::OnSize)
    EVT_PAINT(CParaCoordCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(CParaCoordCanvas::OnEraseBackground)
	EVT_KEY_DOWN(CParaCoordCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(CParaCoordCanvas::OnMouse)
	EVT_IDLE(CParaCoordCanvas::OnIdle) 
END_EVENT_TABLE()

CParaCoordCanvas::CParaCoordCanvas( wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name ), m_VertexBufferID(0), m_IndexBufferID(0), m_ColorBufferID(0)
{
	rf = (CParaCoordFrame*) parent;
    m_init = false;
	m_hDC = NULL;
	m_if_dragging = false;
	m_if_selecting = false;
	InitOrtho();
}

CParaCoordCanvas::~CParaCoordCanvas()
{
	KillFont();
	if ( m_VertexBufferID != -1 )
		glDeleteBuffers(1, &m_VertexBufferID );
	if ( m_IndexBufferID != -1 )
		glDeleteBuffers(1, &m_IndexBufferID );
	if ( m_ColorBufferID != -1 )
		glDeleteBuffers(1, &m_ColorBufferID );
}

GLvoid CParaCoordCanvas::BuildFont(GLvoid)								// Build Our Bitmap Font
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

GLvoid CParaCoordCanvas::KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(m_base, 96);							// Delete All 96 Characters
}

GLvoid CParaCoordCanvas::glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
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

void CParaCoordCanvas::SetShader()
{
	char *vs = NULL, *fs = NULL, *gs = NULL;

	//First, create our shaders 
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

	//Read in the programs
	vs = textFileRead("GLSL/curveline.vert");
	fs = textFileRead("GLSL/curveline.frag");
	gs = textFileRead("GLSL/curveline.geom");

	//Setup a few constant pointers for below
	const char * ff = fs;
	const char * vv = vs;
	const char * gg = gs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);
	glShaderSource(g, 1, &gg, NULL);

	free(vs);free(fs);
	free(gs);

	glCompileShader(v);
	glCompileShader(f);
	glCompileShader(g);

	p = glCreateProgram();

	glAttachShader(p,f);
	glAttachShader(p,v);
	glAttachShader(p,g);

	glProgramParameteriEXT(p,GL_GEOMETRY_INPUT_TYPE_EXT,GL_LINES);
	glProgramParameteriEXT(p,GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_LINE_STRIP);

	int temp;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
	glProgramParameteriEXT(p,GL_GEOMETRY_VERTICES_OUT_EXT,temp);

	glLinkProgram(p);

	char *infoLog;
	infoLog = printShaderInfoLog(v);
	if ( infoLog != NULL )
	{
		wxLogMessage("Vertex Shader InfoLog: %s\n",infoLog);
		delete(infoLog);
	}
	infoLog = printShaderInfoLog(f);
	if ( infoLog != NULL )
	{
		wxLogMessage("Fragment Shader InfoLog: %s\n",infoLog);
		delete(infoLog);
	}
	infoLog = printShaderInfoLog(g);
	if ( infoLog != NULL )
	{
		wxLogMessage("printShaderInfoLog: %s\n",infoLog);
		delete(infoLog);
	}
	infoLog = printProgramInfoLog(p);
	if ( infoLog != NULL )
	{
		wxLogMessage("Program InfoLog: %s\n",infoLog);
		delete(infoLog);
	}
}

void CParaCoordCanvas::InitGL()
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

	glClearColor( WHITE[0],WHITE[1],WHITE[2],WHITE[3] );
	glClearDepth(1.0);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
 	glEnable(GL_NORMALIZE);

	SetShader();

	HWND hWnd = (HWND) GetHandle();
	m_hDC = GetDC(hWnd);

	BuildFont();
}

void CParaCoordCanvas::DrawRenderingStatus()
{
	CDataManager *dm = GetDataManager();

	if ( dm->m_ifDataReady )
	{
		glColor4fv( LIGHT_GRAY );
		glRasterPos2f( 0.0, 0.0 );
		glPrint( "%s", strFrameRate );
		
		char str[64];
		sprintf( str, "Mining Time: %d ms", dm->m_paraCoordTime );
		glRasterPos2f( 0.2, 0.0 );
		glPrint( "%s", str );
	}
}

void CParaCoordCanvas::BindVertexAndIndexBuffer()
{
	SetCurrent();
	if ( m_VertexBufferID != 0 )
	{
		glDeleteBuffers(1, &m_VertexBufferID );
		m_VertexBufferID = 0;
	}
	if ( m_IndexBufferID != 0 )
	{
		glDeleteBuffers(1, &m_IndexBufferID );
		m_IndexBufferID = 0;
	}

	glGenBuffers(1, &m_VertexBufferID);
	glGenBuffers(1, &m_IndexBufferID);

	CDataManager *dm = GetDataManager();
	unsigned int numVerts = dm->m_ParaCoordData.records.size() * dm->m_ParaCoordData.selectedItems.size();
	unsigned int numIndices = dm->m_ParaCoordData.records.size() * ( dm->m_ParaCoordData.selectedItems.size() - 1 ) * 2;

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, numVerts*3*sizeof(float),  dm->m_ParaCoordData.vertexdata, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_IndexBufferID);
	glBufferData(GL_ARRAY_BUFFER, numIndices*sizeof(unsigned int), dm->m_ParaCoordData.indexdata, GL_STATIC_DRAW);
}

void CParaCoordCanvas::BindColorBuffer()
{
	SetCurrent();
	if ( m_ColorBufferID != -1 )
	{
		glDeleteBuffers(1, &m_ColorBufferID );
		m_ColorBufferID = 0;
	}
	glGenBuffers(1, &m_ColorBufferID);

	CDataManager *dm = GetDataManager();
	unsigned int numColors = dm->m_ParaCoordData.records.size() * dm->m_ParaCoordData.selectedItems.size();

	glBindBuffer(GL_ARRAY_BUFFER, m_ColorBufferID);
	glBufferData(GL_ARRAY_BUFFER, numColors*3*sizeof(unsigned char), dm->m_ParaCoordData.colordata, GL_DYNAMIC_COPY);
}

void CParaCoordCanvas::DrawAxis()
{
	CDataManager *dm = GetDataManager();

	t_uintvector &si = dm->m_ParaCoordData.selectedItems;

	float step = 0.0f, start;
	if ( si.size() == 1 )
	{
		start = 0.5;
	}
	else
	{
		step = 0.8f / (si.size() - 1 );
		start = 0.1f;
	}
	for ( int i = 0; i < si.size(); i++ )
	{
		glColor4fv( GRAY );
		glBegin( GL_LINES );
			glVertex3f( start+(i*step), 0.05, 0.0 );
			glVertex3f( start+(i*step), 0.95, 0.0 );
		glEnd();
		glColor4fv( BLACK );
		glRasterPos2f( start-0.05+(i*step), 0.03);

		glPrint( dm->m_item_desc[si[i]].name.c_str() );
	}

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

void CParaCoordCanvas::DrawParallelCoordinate()
{
	t_recordset::iterator it;
	CDataManager *dm = GetDataManager();

	t_uintvector &si = dm->m_ParaCoordData.selectedItems;

	if ( si.size() < 2 )
		return;

	float step = 0.8f / (si.size() - 1 );
	float start = 0.1f;

	vector<float> steps;
	steps.resize( si.size() );
	for ( int i = 0; i < si.size(); i++ )
		steps[i] = 0.9f / (dm->m_item_desc[si[i]].num_values - 1);

	glUseProgram(p);

	glColor4fv( LIGHT_GRAY );
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
	glVertexPointer(3,GL_FLOAT,0,0);

	glBindBuffer(GL_ARRAY_BUFFER, m_ColorBufferID);
	glColorPointer( 3, GL_UNSIGNED_BYTE, 0, 0 );

//	glVertexPointer( 3, GL_FLOAT, 0, dm->m_ParaCoordData.vertexdata );
//	glColorPointer( 3, GL_UNSIGNED_BYTE, 0, dm->m_ParaCoordData.colordata );

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
	glDrawElements(GL_LINES, (si.size() - 1 )*dm->m_ParaCoordData.records.size()*2, GL_UNSIGNED_INT, NULL);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glUseProgram(NULL);

	float pos[3] = {0.0f, 0.0f, 0.6f};
	for ( t_pickset::iterator psit = dm->m_pickset.begin(); psit != dm->m_pickset.end(); psit++ )
	{
		for ( unsigned int i = 0; i < si.size(); i++ )
		{
			if ( (*psit).first == si[i] )
			{
				pos[0] = start + i*step;
				pos[1] = (*psit).second * steps[i] + 0.05f;
				glColor4fv( RED );
				glPointSize( 5 );
				glBegin(GL_POINTS);
				glVertex3fv( pos );
				glEnd();
				glColor4fv( BLACK );
				pos[0]+= 0.01f * (m_Ortho[1]-m_Ortho[0]);
				glRasterPos3fv(pos);
				char str[256];
				sprintf( str, "%s", (char*)(dm->m_item_desc[(*psit).first].value_names[(*psit).second]).c_str() );
				glPrint("%s",str);
			}
		}
	}
}

void CParaCoordCanvas::SelectData()
{
	CDataManager *dm = GetDataManager();

	unsigned int itemidx = 0;
	int valueidx = 0;

	t_uintvector &si = dm->m_ParaCoordData.selectedItems;

	if ( si.size() < 2 )
		return;

	float step = 0.8f / (si.size() - 1 );
	float start = 0.1f;

	int x1,x2;
	if ( m_newOrtho[0] - start < 0.0f )
		x1 = (m_newOrtho[0] - start - step) / step;
	else
		x1 = (m_newOrtho[0] - start) / step;
	if ( m_newOrtho[2] - start < 0.0f )
		x2 = (m_newOrtho[2] - start - step) / step;
	else
		x2 = (m_newOrtho[2] - start) / step;
	if ( x2 != x1+1 )
	{
		Refresh(false);
		return;
	}

	itemidx = si[x2];

	step = 0.9f / (dm->m_item_desc[itemidx].num_values - 1);
	start = 0.05;

	int y1 = (m_newOrtho[1] - start) / step;
	int y2 = (m_newOrtho[3] - start) / step;

	set<int> recordset;
	for ( t_recordset::iterator it = dm->m_ParaCoordData.records.begin(); it != dm->m_ParaCoordData.records.end(); it++ )
	{
		valueidx = (*it).first[x2];
		recordset.insert( (int)valueidx );
	}

	for ( set<int>::iterator it = recordset.begin(); it != recordset.end(); it++ )
	{
		valueidx = *it;
		if ( (y1 < valueidx && valueidx <= y2) || ((m_newOrtho[1] - start < 0.0f) && y2 == 0) )
		{
			rf->m_cf->m_selectedValue->SetValue( dm->m_item_desc[itemidx].value_names[valueidx] );

			pair<t_pickset::iterator, bool> ret;
			ret = dm->m_pickset.insert( pair<unsigned int, unsigned int> (itemidx, (unsigned int)valueidx ) );
			if(dm->m_ifGraphReady)
			{
				int offset =0;
				if(itemidx==dm->m_Graph.selected_item[0])
					offset = 0;
				else
					offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
					dm->m_Graph.graph_node[valueidx+offset].edge_ref = 3;
					dm->m_Graph.graph_node[valueidx+offset].stable = true;
			}

			if ( ret.second == false )
			{
				dm->m_pickset.erase( ret.first );
				if(dm->m_ifGraphReady)
				{
					int offset =0;
					if(itemidx==dm->m_Graph.selected_item[0])
						offset = 0;
					else
						offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
					dm->m_Graph.graph_node[valueidx+offset].edge_ref = 0;
					dm->m_Graph.graph_node[valueidx+offset].stable = false;
				}
			}
		}
	}
 
	GetDataManager()->ParaCoord_PrepareRenderColorData();
	BindColorBuffer();
	rf->m_cf->RefreshAll();
}

void CParaCoordCanvas::Render()
{
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

	int w = m_size.x, h = m_size.y;

	glViewport (0, 20, w, h );
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho( m_Ortho[0], m_Ortho[1], m_Ortho[2], m_Ortho[3], m_Ortho[4], m_Ortho[5] );
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CDataManager *dm = GetDataManager();
	if ( dm->m_ifDataReady )
	{
		DrawAxis();
		DrawParallelCoordinate();
	}

	CalculateFrameRate();

	DrawRenderingStatus();

    glFlush();
    SwapBuffers();
}


void CParaCoordCanvas::OnIdle( wxIdleEvent& event )
{
	Refresh(false);
}

void CParaCoordCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
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
//	wxLogMessage("CGLCanvas::OnPaint has been called.");
    Render();
}

void CParaCoordCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
//    wxGLCanvas::OnSize(event);
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);

	glViewport (0, 0, w, h ); 
    m_size.SetHeight( h );
	m_size.SetWidth( w );
}

void CParaCoordCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void CParaCoordCanvas::OnKeyDown( wxKeyEvent& event )
{
	SetCurrent();
	char ch = event.GetKeyCode();
//	if (  ch == 'f' || ch == 'F' )
//		rePositionCamera();
	if ( ch == ' ' )
	{
		InitOrtho();
		Refresh(false);
	}
}

void CParaCoordCanvas::OnMouse( wxMouseEvent& event )
{
	int mouse_x = event.GetX();
	int mouse_y = event.GetY();

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
		}
	}
}

void CParaCoordCanvas::CalculateFrameRate()
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

CDataManager *CParaCoordCanvas::GetDataManager()
{ 
	return rf->m_cf->GetDataManager(); 
}


} // end of namespace
