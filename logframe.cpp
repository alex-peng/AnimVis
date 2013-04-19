#include "logframe.h"

namespace AMT
{

BEGIN_EVENT_TABLE(CLogPanel, wxPanel)
EVT_SIZE      (CLogPanel::OnSize)
END_EVENT_TABLE()

const int  ID_BOOK              = 1000;

void CLogPanel::OnSize( wxSizeEvent& event )
{
    int x = 0;
    int y = 0;
    GetClientSize( &x, &y );

	int Border = 0;
    if (m_book) m_book->SetSize( Border, Border, x-2*Border, y-2*Border );
    if (m_logtext) m_logtext->SetSize( Border, Border, x-2*Border, y-2*Border );
//    if (m_errortext) m_errortext->SetSize( Border, Border, x-2*Border, y-2*Border );
//    if (m_alltext) m_alltext->SetSize( Border, Border, x-2*Border, y-2*Border );
}

CLogPanel::CLogPanel(wxFrame *frame, const wxPoint& pos, const wxSize& size )
	: wxPanel( frame, wxID_ANY, pos, size )
{
    m_logtext = NULL;
//    m_errortext = NULL;
//    m_alltext = NULL;
    m_book = NULL;

    m_book = new wxBookCtrl(this, ID_BOOK);

	wxPanel *panel = new wxPanel(m_book);
	m_logtext = new wxTextCtrl(panel, wxID_ANY, _T(""),
                            pos, size, wxTE_MULTILINE | wxTE_READONLY );
    m_logtext->SetBackgroundColour(wxT("wheat"));
    m_logTargetOld_log = wxLog::SetActiveTarget(new wxLogTextCtrl(m_logtext));
    m_book->AddPage(panel, _T("Log Messages") );
/*
	panel = new wxPanel(m_book);
	m_errortext = new wxTextCtrl(panel, wxID_ANY, _T(""),
                            pos, size, wxTE_MULTILINE | wxTE_READONLY);
    m_errortext->SetBackgroundColour(wxT("wheat"));
    m_logTargetOld_error = wxLog::SetActiveTarget(new wxLogTextCtrl(m_errortext));
    m_book->AddPage(panel, _T("Error Messages") );

	panel = new wxPanel(m_book);
	m_alltext = new wxTextCtrl(panel, wxID_ANY, _T(""),
                            pos, size, wxTE_MULTILINE | wxTE_READONLY);
    m_alltext->SetBackgroundColour(wxT("wheat"));
    m_logTargetOld_all = wxLog::SetActiveTarget(new wxLogTextCtrl(m_alltext));
    m_book->AddPage(panel, _T("Other Messages") );
*/

}

BEGIN_EVENT_TABLE(CLOGFrame, wxFrame)
END_EVENT_TABLE()

CLOGFrame::CLOGFrame( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style)
		: wxFrame( NULL, wxID_ANY, title, pos, size, style )
{
	wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(wxID_EXIT, _T("Exit"), _T("Quit Log window"));

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(fileMenu, _T("&File"));

    m_logpanel = new CLogPanel( this, wxDefaultPosition, size );
}

}