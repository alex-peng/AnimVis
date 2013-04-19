#ifndef _HISTOGRAMFRAME_H_
#define _HISTOGRAMFRAME_H_

#include <vector>
using namespace std;
#include <wx/glcanvas.h>
#include "text.h"

namespace AMT
{

class CHistogramCanvas;
class CRenderObject;
class CHistogramFrame;
class CControlFrame;
class CDataManager;

class CHistogramFrame: public wxFrame
{
	friend class CHistogramCanvas;
	friend class CControlFrame;
public:
	static CHistogramFrame *Create( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE );

    CHistogramFrame( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~CHistogramFrame();

	CControlFrame *m_cf;

    void OnExit(wxCommandEvent& event);
//	void OnFocusGot(wxFocusEvent& event);

    void SetCanvas( CHistogramCanvas *canvas ) { m_canvas = canvas; }
    CHistogramCanvas *GetCanvas() { return m_canvas; }

private:
    CHistogramCanvas *m_canvas;

    DECLARE_EVENT_TABLE()
};

class CHistogramCanvas: public wxGLCanvas
{
    friend class CHistogramFrame;
public:
    CHistogramCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("CHistogramCanvas") );

 	~CHistogramCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
	void OnKeyDown( wxKeyEvent& event );
	void OnMouse( wxMouseEvent& event );
	void OnIdle( wxIdleEvent& event );

    void Render();
    void InitGL();

	void SetLogScale ( bool _value ) { m_if_logscale = _value; }
	void SetVerticalScale( float _value ) { m_vertical_scale = _value; }

	CDataManager *GetDataManager();
private:
	HDC m_hDC;
	GLuint	m_base;

	CHistogramFrame *rf;
	bool m_init;
	wxSize m_size;

	GLuint m_selectBuf[32];
	GLint m_hits;
	GLint m_viewport[4];
	float m_Ortho[6];
	float m_newOrtho[4];
	bool m_if_dragging;
	int m_x,m_y;
	int m_mode;
	bool m_if_logscale;

    int framesPerSecond;        // This will store our fps
    long lastTime;           // This will hold the time from the last frame
    char strFrameRate[50];         // We will store the string here for the window title

	unsigned int m_maxvalue;

	float m_vertical_scale;

//	text_t glview_text;

	void CalculateFrameRate();

	void InitOrtho()
	{
		m_Ortho[0] = -0.1f;
		m_Ortho[1] = 1.06f;
		m_Ortho[2] = -0.1f;
		m_Ortho[3] = 1.06f;
		m_Ortho[4] = -1.0f;
		m_Ortho[5] = 1.0f;
	}

	void DrawRenderingStatus();
	void DrawAxis();
	void DrawHistogram();
	void ProcessHits(GLint hits, GLuint buffer[], int x, int y);
	void StartPicking();
	void StopPicking();

	GLvoid BuildFont(GLvoid);
	GLvoid KillFont(GLvoid);									// Delete The Font List
	GLvoid glPrint(const char *fmt, ...);	


	DECLARE_EVENT_TABLE()
};

}
#endif