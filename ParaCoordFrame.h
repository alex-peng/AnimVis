#ifndef _PARACOORDFRAME_H_
#define _PARACOORDFRAME_H_

//#include <vector>
//using namespace std;
#include <wx/glcanvas.h>

namespace AMT
{

class CParaCoordCanvas;
class CParaCoordFrame;
class CControlFrame;
class CDataManager;

class CParaCoordFrame: public wxFrame
{
	friend class CParaCoordCanvas;
	friend class CControlFrame;
public:
	static CParaCoordFrame *Create( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE );

    CParaCoordFrame( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~CParaCoordFrame();

	CControlFrame *m_cf;

    void OnExit(wxCommandEvent& event);
//	void OnFocusGot(wxFocusEvent& event);

    void SetCanvas( CParaCoordCanvas *canvas ) { m_canvas = canvas; }
    CParaCoordCanvas *GetCanvas() { return m_canvas; }

private:
    CParaCoordCanvas *m_canvas;

    DECLARE_EVENT_TABLE()
};

class CParaCoordCanvas: public wxGLCanvas
{
    friend class CParaCoordFrame;
public:
    CParaCoordCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("CParaCoordCanvas") );

 	~CParaCoordCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
	void OnKeyDown( wxKeyEvent& event );
	void OnMouse( wxMouseEvent& event );
	void OnIdle( wxIdleEvent& event );

    void Render();
    void InitGL();

	void BindVertexAndIndexBuffer();
	void BindColorBuffer();

	CDataManager *GetDataManager();

private:
	HDC m_hDC;
	GLuint	m_base;

	CParaCoordFrame *rf;
	bool m_init;
	wxSize m_size;
	float m_Ortho[6];
	float m_newOrtho[4];
	bool m_if_dragging;
	bool m_if_selecting;

	GLuint v,f,p,g;	//for shaders

    int framesPerSecond;        // This will store our fps
    long lastTime;           // This will hold the time from the last frame
    char strFrameRate[50];         // We will store the string here for the window title

	GLuint m_VertexBufferID, m_IndexBufferID, m_ColorBufferID;

	void CalculateFrameRate();

	void DrawRenderingStatus();
	void DrawAxis();
	void DrawParallelCoordinate();

	void SelectData();
	void SetShader();

	void InitOrtho()
	{
		m_Ortho[0] = 0.0f;
		m_Ortho[1] = 1.0f;
		m_Ortho[2] = 0.0f;
		m_Ortho[3] = 1.0f;
		m_Ortho[4] = -1.0f;
		m_Ortho[5] = 1.0f;
	}


	GLvoid BuildFont(GLvoid);
	GLvoid KillFont(GLvoid);									// Delete The Font List
	GLvoid glPrint(const char *fmt, ...);	


	DECLARE_EVENT_TABLE()
};

}
#endif