#pragma once

#include <vector>
using namespace std;
#include <wx/glcanvas.h>
#include "text.h"
namespace AMT
{

class CGraphCanvas;
class CRenderObject;
class CGraphFrame;
class CControlFrame;
class CDataManager;

class CGraphFrame: public wxFrame
{
	friend class CGraphCanvas;
	friend class CControlFrame;
public:
	static CGraphFrame *Create( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE );

    CGraphFrame( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~CGraphFrame();

	CControlFrame *m_cf;
	
    void OnExit(wxCommandEvent& event);
//	void OnFocusGot(wxFocusEvent& event);

    void SetCanvas( CGraphCanvas *canvas ) { m_canvas = canvas; }
    CGraphCanvas *GetCanvas() { return m_canvas; }

	void AddRenderObject( CRenderObject *ro );

private:
    CGraphCanvas *m_canvas;
	vector<CRenderObject*> m_renderObjectList;
	CRenderObject *selectedObject;

    DECLARE_EVENT_TABLE()
};

class CGraphCanvas: public wxGLCanvas
{
    friend class CGraphFrame;
public:
    CGraphCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("CGLCanvas") );

 	~CGraphCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
	void OnKeyDown( wxKeyEvent& event );
	void OnMouse( wxMouseEvent& event );
	void OnIdle( wxIdleEvent& event );

    void Render();
    void InitGL();
	
	CDataManager *GetDataManager();

	float m_Ortho[6];
	float m_Ortho_backup[4];

private:
	HDC m_hDC;
	GLuint	m_base;

	CGraphFrame *rf;
	bool m_init;
	wxSize m_size;
	bool m_showEdge;

    int framesPerSecond;        // This will store our fps
    long lastTime;           // This will hold the time from the last frame
    char strFrameRate[50];         // We will store the string here for the window title

	unsigned int m_maxvalue;

	unsigned int numTriangleCount;
	
	
	
	 
	 
	float node_size;

 
	int  m_node;
	

	//
	
	float m_newOrtho[4];
	float m_newOrtho_pre[4];
	bool m_if_dragging;
	bool m_if_selecting;
	bool m_if_choseDrag;


	void CalculateFrameRate();

	void DrawExtraInfo();
	void DrawRenderingStatus();
	void DrawGraph();
	void DrawGeneralGraph();
	void GetColor(string str);
	void SelectData();


	void InitOrtho()
	{
		m_Ortho[0] = m_Ortho_backup[0];
		m_Ortho[1] = m_Ortho_backup[1];
		m_Ortho[2] = m_Ortho_backup[2];
		m_Ortho[3] = m_Ortho_backup[3];
		m_Ortho[4] = -1.0f;
		m_Ortho[5] = 1.0f;
	}


	GLvoid BuildFont(GLvoid);
	GLvoid KillFont(GLvoid);									// Delete The Font List
	GLvoid glPrint(const char *fmt, ...);	

	DECLARE_EVENT_TABLE()
};

}
//#endif