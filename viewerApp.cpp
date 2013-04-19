#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/app.h"
#include "HistogramFrame.h"
#include "viewerApp.h"
#include "logframe.h"
#include "ControlFrame.h"
#include "ParaCoordFrame.h"
#include "datamanager.h"
#include "GraphFrame.h"


IMPLEMENT_APP(CViewApp)

bool CViewApp::OnInit()
{
    // Create the main frame window
	
	AMT::CLOGFrame *logframe = new AMT::CLOGFrame( "LOG window", wxPoint(1920, 600), wxSize(600, 200) );
	logframe->Show(true);
	
	AMT::CControlFrame *controlFrame = AMT::CControlFrame::Create(logframe, "AnimeVis Control Panel", wxPoint(0, 670), wxSize(1900, 350) );
	 
	wxIdleEvent::SetMode( wxIDLE_PROCESS_SPECIFIED );

    return true;
}
