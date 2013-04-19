#ifndef _LOGFRAME_H_
#define _LOGFRAME_H_

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/bookctrl.h"

namespace AMT
{

class CLogPanel: public wxPanel
{
public:
    CLogPanel(wxFrame *frame, const wxPoint& pos, const wxSize& size );
    void OnSize( wxSizeEvent& event );
    wxTextCtrl    *m_logtext;
//    wxTextCtrl    *m_alltext;
//    wxTextCtrl    *m_errortext;
    wxBookCtrl    *m_book;

private:
    wxLog *m_logTargetOld_log;
//    wxLog *m_logTargetOld_all;
//    wxLog *m_logTargetOld_error;

    DECLARE_EVENT_TABLE()
};


class CLOGFrame: public wxFrame
{
public:
    CLOGFrame( const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
private:
    CLogPanel *m_logpanel;

    DECLARE_EVENT_TABLE()
};

}

#endif