#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <GL/glew.h>
#include "HistogramFrame.h"
#include "ParaCoordFrame.h"
#include "ControlFrame.h"
#include "datamanager.h"
#include "glColor.h"
#include "glhelper.h"
//#include "object.h"
//#include "GL/glu.h"
#include "GL/freeglut.h"

#define ONE_SECOND 1


namespace AMT
{

enum
{
    ID_QUIT  = wxID_EXIT,
};

BEGIN_EVENT_TABLE(CHistogramFrame, wxFrame)
    EVT_MENU(ID_QUIT, CHistogramFrame::OnExit)
END_EVENT_TABLE()


CHistogramFrame *CHistogramFrame::Create( const wxString& title, const wxPoint& pos,
										 const wxSize& size, long style )
{
	CHistogramFrame *rf = new CHistogramFrame(title, pos, size, style);
    rf->Show(true);
    return rf;
}

CHistogramFrame::~CHistogramFrame()
{
	if ( m_canvas != NULL )
		delete m_canvas;

	//for ( int i = 0; i < m_renderObjectList.size(); i++ )
	//	m_renderObjectList[i]->destroyMyself();
}

CHistogramFrame::CHistogramFrame(const wxString& title, const wxPoint& pos,
						   const wxSize& size, long style)
						   : wxFrame( NULL, wxID_ANY, title, pos, size, style ), m_cf(NULL)
{
    wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(ID_QUIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

	m_canvas = new CHistogramCanvas( this, wxID_ANY,wxDefaultPosition, wxSize(size.GetWidth(),size.GetWidth() ));
}

void CHistogramFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    Close(true);
}

BEGIN_EVENT_TABLE(CHistogramCanvas, wxGLCanvas)
    EVT_SIZE(CHistogramCanvas::OnSize)
    EVT_PAINT(CHistogramCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(CHistogramCanvas::OnEraseBackground)
	EVT_KEY_DOWN(CHistogramCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(CHistogramCanvas::OnMouse)
	EVT_IDLE(CHistogramCanvas::OnIdle) 
END_EVENT_TABLE()

CHistogramCanvas::CHistogramCanvas( wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name ), m_if_logscale(true), m_vertical_scale(1.0f)
{
	rf = (CHistogramFrame*) parent;
    m_init = false;
	m_hDC = NULL;
	m_mode = GL_RENDER;
	InitOrtho();
	m_newOrtho[0] = -1.0f;
	m_if_dragging = false;
}

CHistogramCanvas::~CHistogramCanvas()
{
	KillFont();
}

GLvoid CHistogramCanvas::BuildFont(GLvoid)								// Build Our Bitmap Font
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

GLvoid CHistogramCanvas::KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(m_base, 96);							// Delete All 96 Characters
}

GLvoid CHistogramCanvas::glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
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


void CHistogramCanvas::InitGL()
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

//	glClearColor( 0.15f, 0.2f, 0.3f, 1.0f );
	glClearColor( WHITE[0], WHITE[1], WHITE[2], WHITE[3] );
	glClearDepth(1.0);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
 	glEnable(GL_NORMALIZE);

//	glview_text.Load("font.tga"); 

	HWND hWnd = (HWND) GetHandle();
	m_hDC = GetDC(hWnd);

	BuildFont();
}

void CHistogramCanvas::DrawRenderingStatus()
{
	CDataManager *dm = GetDataManager();

	if ( dm->m_ifDataReady )
	{
		// draw frame rate on screen
		glColor4fv( LIGHT_GRAY );
		glRasterPos2f( -0.1, -0.1 );
		glPrint( "%s", strFrameRate );


		char str[64];
		sprintf( str, "Mining Time: %d ms", dm->m_histoTime );
		glRasterPos2f( 0.1, -0.1 );
		glPrint( "%s", str );
	}
}

void CHistogramCanvas::DrawAxis()
{
	glColor4fv( BLACK );
	glBegin( GL_LINES );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 0.0, 1.0, 0.0 );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 1.0, 0.0, 0.0 );
	glEnd();
	float step = 1.0f / 6.0f;

	for ( int i = 1; i < 7; i++ )
	{
		glBegin( GL_LINES );
			glVertex3f( 0.0, i * step, 0.0 );
			glVertex3f( -0.01, i * step, 0.0 );
		glEnd();
	}

	CDataManager *dm = GetDataManager();

	unsigned int _sel = dm->GetCurrentItem();
	unsigned int nValues = dm->m_item_desc[_sel].num_values;

	m_maxvalue = dm->m_item_desc[_sel].value_counts[0];
	for ( int i = 1; i < nValues; i++ )
		if ( m_maxvalue < dm->m_item_desc[_sel].value_counts[i] )
			m_maxvalue = dm->m_item_desc[_sel].value_counts[i];

	char str[256];

	strcpy( str, dm->m_item_desc[dm->GetCurrentItem()].name.c_str() );
	int len = strlen( str );
	glRasterPos2f( 0.4, -0.08);
	glPrint( "%s", str );

	sprintf( str, "%d", (int)(m_maxvalue / m_vertical_scale));
	glRasterPos2f( -0.1, 0.956 );
	glPrint( "%s", str );
	sprintf( str, "0");
	glRasterPos2f( -0.1, 0.0 );
	glPrint( "%s", str );
	for ( int i = 1; i < 6; i++ )
	{
		float temp, _value;
		if ( m_if_logscale )
		{
			temp = log10((float)(m_maxvalue+1)) * i / 6.0f;
			_value = pow( 10.0f, temp);
		}
		else
			_value = (float)m_maxvalue * i / 6.0f;

		_value /= m_vertical_scale;
		sprintf( str, "%d", (int)_value );
		glRasterPos2f( -0.1, i*step);
		glPrint( "%s", str );
	}

	if ( m_if_dragging )
	{
		glColor4fv( BLACK );
		glBegin( GL_LINE_LOOP );
		glVertex3f( m_newOrtho[0], m_newOrtho[1], 0.9 );
		glVertex3f( m_newOrtho[0], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[3], 0.9 );
		glVertex3f( m_newOrtho[2], m_newOrtho[1], 0.9 );
		glEnd();
	}
}

void CHistogramCanvas::StartPicking()
{
	GLint viewport[4];
	float ratio;

	glSelectBuffer(32,m_selectBuf);

	glGetIntegerv(GL_VIEWPORT,m_viewport);

	glRenderMode(GL_SELECT);

	glInitNames();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix(m_x,m_viewport[3]-m_y,1,1,m_viewport);
	glOrtho( m_Ortho[0], m_Ortho[1], m_Ortho[2], m_Ortho[3], m_Ortho[4], m_Ortho[5] );
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

}

void CHistogramCanvas::StopPicking()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glFlush();
	m_hits = glRenderMode(GL_RENDER);
	if (m_hits > 0){
		ProcessHits(m_hits,m_selectBuf,0,0);
	}
	else{
		DrawAxis();
		DrawHistogram();
	}
	m_mode = GL_RENDER;
}

void CHistogramCanvas::ProcessHits(GLint hits, GLuint buffer[], int x, int y)
{
	GLint i, j, numberOfNames=0;
	GLuint names,*ptr,minZ,*ptrNames;

	ptr = (GLuint *) buffer;
	minZ = 0xffffffff;
	for (i = 0; i < hits; i++) {	
		names = *ptr;
		ptr++;
		if (*ptr < minZ) {
			numberOfNames = names;
			minZ = *ptr;
			ptrNames = ptr+2;
		}  
		ptr += names+2;
	}

	if (numberOfNames > 0) 
	{
		ptr = ptrNames;
		CDataManager *dm = GetDataManager();
		unsigned int _sel = dm->GetCurrentItem();

		rf->m_cf->m_selectedValue->SetValue( dm->m_item_desc[_sel].value_names[*ptr] );

		pair<t_pickset::iterator, bool> ret;
		ret = dm->m_pickset.insert( pair<unsigned int, unsigned int> (_sel, (*ptr) ) );
		
		if(dm->m_ifGraphReady)
		{
			int offset =0;
			if(_sel==dm->m_Graph.selected_item[0])
				offset = 0;
			else
				offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
			dm->m_Graph.graph_node[(*ptr)+offset].edge_ref = 3;
			dm->m_Graph.graph_node[(*ptr)+offset].stable = true;
		}

		if ( ret.second == false )
		{
			dm->m_pickset.erase( ret.first );
			if(dm->m_ifGraphReady)
			{
				int offset =0;
				if(_sel==dm->m_Graph.selected_item[0])
					offset = 0;
				else
					offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
				dm->m_Graph.graph_node[(*ptr)+offset].edge_ref = 0;
				dm->m_Graph.graph_node[(*ptr)+offset].stable = false;
			}
		}
		GetDataManager()->ParaCoord_PrepareRenderColorData();
		rf->m_cf->m_pcf->GetCanvas()->BindColorBuffer();
		rf->m_cf->RefreshAll();
	}

	DrawAxis();
	DrawHistogram();
}

void CHistogramCanvas::DrawHistogram()
{
	CDataManager *dm = GetDataManager();
	unsigned int *_data = dm->GetHistogramData();

	unsigned int _sel = dm->GetCurrentItem(); 

	unsigned int nValues = dm->m_item_desc[_sel].num_values;

	float step = 1.0f / nValues;
	float delta = 0.1f * step;
	for ( int i = 0; i < nValues; i++ )
	{
		bool iffound = false;
		for ( t_pickset::iterator it = dm->m_pickset.begin(); it != dm->m_pickset.end(); it++ )
		{
			if ( (*it).first == _sel && (*it).second == i )
			{
				float pos[3];
				pos[0] = step*(i+1) - 0.5*step;
				pos[1] = 0.01;
				pos[2] = 0.5;

				glColor4fv(BLACK);
				char str[256];
				glRasterPos3fv(pos);
				sprintf( str, "%d", _data[i] );
				glPrint("%s",str);
				pos[1] = -0.03;
				glRasterPos3fv(pos);
				sprintf( str, "%s", (char*)(dm->m_item_desc[_sel].value_names[i]).c_str() );
				glPrint("%s",str);

				iffound = true;
				break;
			}
		}

		if ( iffound )
			glColor4fv( RED );
		else
			glColor4fv( LIGHT_GRAY );

		float _height;
		if ( m_if_logscale )
			_height = log10( (float)(_data[i] + 1)) / log10((float)m_maxvalue + 1);
		else
			_height = (float)_data[i] / m_maxvalue;

		_height *= m_vertical_scale;
		 
		glPushMatrix();
		glPushName(i);
		
		glBegin( GL_QUADS );
		glVertex3f( i*step+delta, 0.0, 0.0 );
		glVertex3f( i*step+delta, _height, 0.0 );
		glVertex3f( (i+1)*step-delta, _height, 0.0 );
		glVertex3f( (i+1)*step-delta, 0.0, 0.0 );
		glEnd();

		glPopName();
		glPopMatrix();


	}

}

void CHistogramCanvas::Render()
{
	int w = m_size.x, h = m_size.y;

	glViewport (0, 20, w, h );
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho( m_Ortho[0], m_Ortho[1], m_Ortho[2], m_Ortho[3], m_Ortho[4], m_Ortho[5] );
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_mode == GL_SELECT )
		StartPicking();

	CDataManager *dm = GetDataManager();
	if ( dm->m_ifDataReady )
	{
		DrawAxis();
		DrawHistogram();

		if( m_mode == GL_SELECT )
			StopPicking();
	}

	CalculateFrameRate();

	DrawRenderingStatus();
	 
	glFlush();
	SwapBuffers();
}


void CHistogramCanvas::OnIdle( wxIdleEvent& event )
{
	Render();
	 
	Refresh(false);
}

void CHistogramCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
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

    Render();
}

void CHistogramCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
//    wxGLCanvas::OnSize(event);
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);

	glViewport (0, 20, w, h ); 
    m_size.SetHeight( h );
	m_size.SetWidth( w );
}

void CHistogramCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void CHistogramCanvas::OnKeyDown( wxKeyEvent& event )
{
	SetCurrent();
	char ch = event.GetKeyCode();
	if ( ch == ' ' )
	{
		InitOrtho();
		Refresh(false);
	}
	//if (  ch == 'I' || ch == 'i' )
	//{
	//	if(m_showInfo==false)
	//		m_showInfo=true;
	//	else
	//		m_showInfo = false;
	//}

}

void CHistogramCanvas::OnMouse( wxMouseEvent& event )
{
//	CRenderObject *ro = rf->selectedObject;
//	if ( ro == NULL )
//		return;
	int mouse_x = event.GetX();
	int mouse_y = event.GetY();
	m_x = mouse_x;
	m_y = mouse_y;

	//if(m_showInfo)
	//{
	//	m_mode = GL_SELECT;
	//	GetParent()->Refresh();
	//}
	if (event.LeftDown())
	{
		 m_mode = GL_SELECT;
		 GetParent()->Refresh();// call back idle function
	} 
	else if ( event.MiddleDown())
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
			m_Ortho[0] = m_newOrtho[0];
			m_Ortho[1] = m_newOrtho[2];
			Refresh(false);
		}
	}

}

void CHistogramCanvas::CalculateFrameRate()
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

CDataManager *CHistogramCanvas::GetDataManager() 
{ 
	return rf->m_cf->GetDataManager(); 
}

} // end of namespace
