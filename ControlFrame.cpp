#include "ControlFrame.h"
#include "HistogramFrame.h"
#include "ParaCoordFrame.h"
#include "logframe.h"
#include "datamanager.h"
#include "GraphFrame.h"

#include <wx/notebook.h>
#include <wx/listbox.h>
#include <wx/datectrl.h>
#include <wx/combo.h>

namespace AMT
{

enum
{
    ID_QUIT  = wxID_EXIT,
    ID_LOADDATA = 101,
	ID_SYNCFRAMES,
	ID_NEWCONTROLFRAME,
	ID_Control_Listbox,
	ID_Control_TimeSlider,
	ID_Control_RangeSlider,
	ID_Control_DateControl,
	ID_Control_HourControl,
	ID_Control_MinuteControl,
	ID_Control_SecondControl,
	ID_Control_Animate,
	ID_Control_ParaListBox,
	ID_Range_Ratio,
	ID_Speed_Combo,
	ID_Scale_CheckBox,
	ID_VertialScale_Slider,
	ID_ValueSelect_Listbox,
	ID_Clear_Selection,

	ID_Control_GraphListBox,
	ID_Control_GraphForceButton,
	ID_Control_CreateGraphButton,
	ID_Control_Layout_LoadGraph,
	ID_Control_Layout_SaveGraph,
	ID_Control_Layout_att,
	ID_Control_Layout_rep,
	ID_Control_Layout_gra,
	ID_Control_Layout_range,
	ID_Control_Layout_pointSize,
	ID_Control_Layout_Edge,
	ID_Control_Layout_Pin,
};

BEGIN_EVENT_TABLE(CControlFrame,wxFrame)
  EVT_IDLE    (CControlFrame::OnIdle)
  EVT_MENU    (ID_QUIT,  CControlFrame::OnQuit)
  EVT_MENU    (ID_LOADDATA,  CControlFrame::OnLoadData)
  EVT_MENU    (ID_SYNCFRAMES,  CControlFrame::OnSyncAllFrames)
  EVT_MENU    (ID_NEWCONTROLFRAME,  CControlFrame::OnNewControlFrame)
  EVT_CLOSE	  (CControlFrame::OnClose)
  EVT_LISTBOX (ID_Control_Listbox, CControlFrame::OnListboxSelect)
  EVT_LISTBOX (ID_ValueSelect_Listbox, CControlFrame::OnValueSelect)
  EVT_COMMAND_SCROLL  (ID_Control_TimeSlider, CControlFrame::OnTimeSliderScroll)
  EVT_COMMAND_SCROLL  (ID_Control_RangeSlider, CControlFrame::OnRangeSliderScroll)
  EVT_COMMAND_SCROLL_BOTTOM(ID_Control_TimeSlider, CControlFrame::OnTimeSliderScrollBottom) 
  EVT_BUTTON ( ID_Control_Animate, CControlFrame::OnAnimate )
  EVT_BUTTON ( ID_Clear_Selection, CControlFrame::OnClearSelection )
  EVT_LISTBOX (ID_Control_ParaListBox, CControlFrame::OnParaListboxSelect)
  EVT_COMBOBOX( ID_Range_Ratio, CControlFrame::OnRangeRatioSelect)
  EVT_COMBOBOX( ID_Speed_Combo, CControlFrame::OnAnimationSpeedSelect)
  EVT_CHECKBOX( ID_Scale_CheckBox, CControlFrame::OnLogScaleCheck)
  EVT_COMMAND_SCROLL  (ID_VertialScale_Slider, CControlFrame::OnVerticalScaleScroll)

  EVT_COMMAND_SCROLL  (ID_Control_Layout_att, CControlFrame::OnLayoutAttScroll)
  EVT_COMMAND_SCROLL  (ID_Control_Layout_rep, CControlFrame::OnLayoutRepScroll)
  EVT_COMMAND_SCROLL  (ID_Control_Layout_gra, CControlFrame::OnLayoutGraScroll)
  EVT_COMMAND_SCROLL  (ID_Control_Layout_range, CControlFrame::OnLayoutRangeScroll)
  EVT_COMMAND_SCROLL  (ID_Control_Layout_pointSize, CControlFrame::OnLayoutSizeScroll)
  EVT_BUTTON ( ID_Control_GraphForceButton, CControlFrame::OnGraphForce )
  EVT_BUTTON ( ID_Control_CreateGraphButton, CControlFrame::OnGraphCreate )
  EVT_BUTTON ( ID_Control_Layout_LoadGraph, CControlFrame::OnLayoutLoadGraph )
  EVT_BUTTON ( ID_Control_Layout_SaveGraph, CControlFrame::OnLayoutSaveGraph )
  EVT_BUTTON ( ID_Control_Layout_Edge, CControlFrame::OnLayoutEdgeShow)
  EVT_BUTTON ( ID_Control_Layout_Pin, CControlFrame::OnLayoutPin)


  EVT_LISTBOX (ID_Control_GraphListBox, CControlFrame::OnGraphListboxSelect)

END_EVENT_TABLE()


vector<CControlFrame*> CControlFrame::s_ControlFrameList;

CControlFrame* CControlFrame::Create( CLOGFrame *lf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style )
{
	CControlFrame* controlframe = new CControlFrame(lf, title, pos, size, style);
	controlframe->m_hf->m_cf = controlframe;
	controlframe->m_pcf->m_cf = controlframe;
	controlframe->m_gf->m_cf = controlframe;
	controlframe->Show(true);

	s_ControlFrameList.push_back( controlframe );
	return controlframe;
}

CControlFrame::CControlFrame( CLOGFrame *lf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style )
		: wxFrame( NULL, wxID_ANY, title, wxPoint(pos.x+s_ControlFrameList.size()*50, pos.y+s_ControlFrameList.size()*50), size, style ), \
		m_lf(lf), m_hf(NULL),m_pcf(NULL),  m_if_animate(false), m_invfps(33.3334f), m_accum_millisec(0),m_gf(NULL)
{

	m_hf = CHistogramFrame::Create("Histogram Window", wxPoint(s_ControlFrameList.size()*50, s_ControlFrameList.size()*50), wxSize(640, 660));
	m_pcf = CParaCoordFrame::Create("Parallel Coordinate Window", wxPoint(640+s_ControlFrameList.size()*50, s_ControlFrameList.size()*50), wxSize(640, 660));
	m_gf = CGraphFrame::Create("Graph Window", wxPoint(1280+s_ControlFrameList.size()*50, s_ControlFrameList.size()*50), wxSize(640, 660));

	m_datamanager = new CDataManager();
	
	wxMenuBar *menu_bar = new wxMenuBar();

	wxMenu *menuFile = new wxMenu;
	menuFile->Append( ID_LOADDATA, _T("&Load Data file...\tCtrl-L"));
	menuFile->AppendSeparator();
	menuFile->Append( ID_NEWCONTROLFRAME, _T("&New Control Window\tCtrl-N"));
//	m_syncmenu = new wxMenuItem( menuFile, ID_SYNCFRAMES, _T("&Time Sync Control"), _T("Time Sync Contro"));
	menuFile->AppendCheckItem(  ID_SYNCFRAMES, _T("&Time Sync Control"), _T("Time Sync Contro"));
	m_syncmenu = menuFile->FindItem( ID_SYNCFRAMES, &menuFile );
	menuFile->AppendSeparator();
	menuFile->Append( ID_QUIT, _T("E&xit\tCtrl-Q"));
	menu_bar->Append(menuFile, _T("&File"));

	SetMenuBar( menu_bar );

    // make a panel with some controls
    m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_sizer = new wxBoxSizer(wxVERTICAL);

	m_pNotebook = new wxNotebook( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

	wxPanel *panel = new wxPanel(m_pNotebook,wxID_ANY);

	wxTextCtrl *control = new wxTextCtrl(panel, wxID_ANY,  _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE );
	control = new wxTextCtrl(panel, wxID_ANY,  _T("Value Names:"), wxPoint(160,3), wxDefaultSize, wxBORDER_NONE );

	m_pListBox = new wxListBox( panel, ID_Control_Listbox, wxPoint(0, 25), wxSize(150, size.GetHeight()-120), 0, NULL, wxLB_EXTENDED, wxDefaultValidator );
	m_valueSelectBox = new wxListBox( panel, ID_ValueSelect_Listbox, wxPoint(160, 25), wxSize(150, size.GetHeight()-120), 0, NULL, wxLB_EXTENDED | wxLB_HSCROLL, wxDefaultValidator );
	m_scalecheckbox = new wxCheckBox(panel, ID_Scale_CheckBox, _T("Log Scale"), wxPoint(320, 4), wxSize(100, 25));
	m_scalecheckbox->SetValue( true );

	control = new wxTextCtrl(panel, wxID_ANY,  _T("Vertical Scale:"), wxPoint(420,7), wxSize( 80, 25), wxBORDER_NONE );
	m_verticalscale_slider = new wxSlider( panel, ID_VertialScale_Slider, 1, 1, 10, wxPoint( 495, 7),  wxSize(size.GetWidth()-550, 25) );
	m_verticalscale_slider->SetRange( 1, 5000 );
	m_verticalscale_slider->SetValue( 1 );

	m_clearSelectionButton = new wxButton( panel, ID_Clear_Selection, _T("Clear Selection"), wxPoint( 320, 35), wxSize( 150, 25) ); 

	control = new wxTextCtrl(panel, wxID_ANY,  _T("Selected Value:"), wxPoint(320,size.GetHeight()-150), wxSize( 120, 25), wxBORDER_NONE );
	m_selectedValue = new wxTextCtrl(panel, wxID_ANY,  _T(""), wxPoint(320,size.GetHeight()-125), wxSize( size.GetWidth() - 550, 25));//, wxBORDER_NONE );

	m_pNotebook->AddPage( panel, "Histogram", false, -1 );

	panel = new wxPanel(m_pNotebook,wxID_ANY);
	m_pNotebook->AddPage( panel, "Parallel Coordinate", false, -1 );
	control = new wxTextCtrl(panel, wxID_ANY,  _T("Item Names:"), wxPoint(0,3), wxDefaultSize, wxBORDER_NONE );
	m_pParaSel = new wxListBox(panel, ID_Control_ParaListBox, wxPoint(0, 25), wxSize(150, size.GetHeight()-120), 0, NULL, wxLB_MULTIPLE );

	panel = new wxPanel(m_pNotebook,wxID_ANY);
	m_pNotebook->AddPage( panel, "Graph", false, -1 );

	m_pGraphSel = new wxListBox(panel, ID_Control_GraphListBox, wxPoint(0, 22), wxSize(150, size.GetHeight()-120), 0, NULL, wxLB_MULTIPLE );
	m_pGraphCreate = new wxButton( panel, ID_Control_CreateGraphButton, _T("Create Graph"), wxPoint( 160, 25), wxSize( 125, 25) ); 
	m_pGraphForceApply = new wxButton( panel, ID_Control_GraphForceButton, _T("Layout: OFF"), wxPoint( 160, 55), wxSize( 125, 25) ); 
	(void)new wxStaticBox( panel, wxID_ANY, _T("&Layout parameters setting"),
		wxPoint(300, 5), wxSize(size.GetWidth()-350, 180));
	m_LayoutLoadGraph = new wxButton( panel, ID_Control_Layout_LoadGraph, _T("Load Graph"), wxPoint( 160, 85), wxSize( 125, 25) ); 
	m_LayoutSaveGraph = new wxButton( panel, ID_Control_Layout_SaveGraph, _T("Save Graph"), wxPoint( 160, 115), wxSize( 125, 25) );

	m_pEdge = new wxButton( panel, ID_Control_Layout_Edge, _T("EdgeShow: On"), wxPoint( 160, 145), wxSize( 125, 25) ); 
	m_pPin =  new wxButton( panel, ID_Control_Layout_Pin, _T("Pin: Off"), wxPoint( 160, 175), wxSize( 125, 25) ); 
	

	m_Layout_att = new wxSlider( panel, ID_Control_Layout_att, 1, 1, 10, wxPoint( 460,30), wxSize( size.GetWidth()-600, 25) );
	m_Layout_att_text = new wxTextCtrl (panel, wxID_ANY, wxT("Attractive Force"),wxPoint(310, 30),wxSize(120,25), wxBORDER_NONE);
	m_Layout_rep = new wxSlider( panel, ID_Control_Layout_rep, 1, 1, 10, wxPoint( 460,60), wxSize( size.GetWidth()-600, 25) );
	m_Layout_rep_text = new wxTextCtrl (panel, wxID_ANY, wxT("Repulsive  Force"),wxPoint(310, 60),wxSize(120,25), wxBORDER_NONE);
	m_Layout_gra = new wxSlider( panel, ID_Control_Layout_gra, 1, 1, 10, wxPoint( 460,90), wxSize( size.GetWidth()-600, 25) );
	m_Layout_gra_text = new wxTextCtrl (panel, wxID_ANY, wxT("Gravity    Force"),wxPoint(310, 90),wxSize(120,25), wxBORDER_NONE);
	m_Layout_pointSize = new wxSlider( panel, ID_Control_Layout_pointSize, 1, 1, 10, wxPoint( 460,120), wxSize( size.GetWidth()-600, 25) );
	m_Layout_pointSize_text = new wxTextCtrl (panel, wxID_ANY, wxT("Point          Size"),wxPoint(310, 120),wxSize(120,25), wxBORDER_NONE);
	m_Layout_range = new wxSlider( panel, ID_Control_Layout_range, 1, 1, 10, wxPoint( 460,150), wxSize( size.GetWidth()-600, 25) );
	m_Layout_range_text = new wxTextCtrl (panel, wxID_ANY, wxT("Range       Size"),wxPoint(310, 150),wxSize(120,25), wxBORDER_NONE);

	






	panel = new wxPanel(m_pNotebook,wxID_ANY);


	m_pNotebook->AddPage( panel, "Animation", false, -1 );
	control = new wxTextCtrl(panel, wxID_ANY,  _T("Current Time:"), wxPoint(0,0), wxSize(80,25), wxBORDER_NONE );
	m_timeStartSlider = new wxSlider( panel, ID_Control_TimeSlider, 1, 1, 10, wxPoint( 0,25 ), wxSize( size.GetWidth()-40, 25) );

	m_DataPicker = new wxDatePickerCtrl( panel, ID_Control_TimeSlider, wxDefaultDateTime, wxPoint( 80,0 ) );
	m_hourCombo = new wxComboBox( panel, ID_Control_HourControl, "", wxPoint( 180,0 ), wxSize(35,25));
	for ( int i = 0; i < 24; i++ )
	{
		char tmp[8];
		sprintf( tmp, "%d", i );
		m_hourCombo->Append( _T(tmp));
	}
	m_minuteCombo = new wxComboBox( panel, ID_Control_MinuteControl, "", wxPoint( 215,0 ), wxSize(35,25) );
	for ( int i = 0; i < 60; i++ )
	{
		char tmp[8];
		sprintf( tmp, "%d", i );
		m_minuteCombo->Append( _T(tmp));
	}
	m_secondCombo = new wxComboBox( panel, ID_Control_SecondControl, "", wxPoint( 250,0 ), wxSize(35,25) );
	for ( int i = 0; i < 60; i++ )
	{
		char tmp[8];
		sprintf( tmp, "%d", i );
		m_secondCombo->Append( _T(tmp));
	}

	unsigned int _x = 80, _y = 50;
	control = new wxTextCtrl(panel, wxID_ANY,  _T("Time Range:"), wxPoint(0,_y), wxSize(80,25), wxBORDER_NONE );
	m_rangeRatioCombo = new wxComboBox( panel, ID_Range_Ratio, "", wxPoint( _x+260,_y ), wxSize(60,25) );
	for ( float ratio = 0.05f; ratio <1.01f; ratio+=0.05f )
	{
		char str[8];
		sprintf( str, "%1.2f", ratio );
		m_rangeRatioCombo->Append(_T(str));
	}
	m_rangeRatioCombo->Select(0);
	m_timeRangeSlider = new wxSlider( panel, ID_Control_RangeSlider, 1, 1, 10, wxPoint( 0,_y+25), wxSize( size.GetWidth()-40, 25) );

	m_timeRangeText = new wxTextCtrl( panel, wxID_ANY, "", wxPoint(_x,_y), wxSize(120,25), wxBORDER_NONE );
	m_recordNumberText = new wxTextCtrl( panel, wxID_ANY, "", wxPoint(_x+120,_y), wxSize(120,25), wxBORDER_NONE );

	m_animateButton = new wxButton( panel, ID_Control_Animate, _T("Animation: OFF"), wxPoint( 0, _y+50), wxSize( 150, 25) ); 
	m_SpeedCombo = new wxComboBox( panel, ID_Speed_Combo, "", wxPoint( 160,_y+50), wxSize(100,25) );
	m_SpeedCombo->Append(_T("Faster"));
	for ( int fps = 5; fps <= 30; fps+=5 )
	{
		char str[8];
		sprintf( str, "%d FPS", fps );
		m_SpeedCombo->Append(_T(str));
	}
	m_SpeedCombo->Select(0);

	m_sizer->Insert(0, m_pNotebook, wxSizerFlags(5).Expand().Border());
	m_sizer->Show( m_pNotebook );
	m_sizer->Layout();

	m_panel->SetSizer(m_sizer);
//	m_sizer->Fit(this);

	SetExtraStyle(wxWS_EX_PROCESS_IDLE );
}

void CControlFrame::OnClearSelection( wxCommandEvent& event )
{
	CDataManager *dm = m_datamanager;
	if ( dm != NULL )
	{
		t_pickset::iterator it;
		for ( it = dm->m_pickset.begin(); it != dm->m_pickset.end();  )
		{
			if ( (*it).first == dm->GetCurrentItem() )
			{
				if(dm->m_ifGraphReady)
				{
					int offset =0;
					if((*it).first==dm->m_Graph.selected_item[0])
						offset = 0;
					else
						offset = dm->m_item_desc[dm->m_Graph.selected_item[0]].num_values;
					dm->m_Graph.graph_node[(*it).second+offset].edge_ref = 0;
					dm->m_Graph.graph_node[(*it).second+offset].stable = false;
				} 
				dm->m_pickset.erase( it );
				it = dm->m_pickset.begin();
			}
			else
				it++;
		}
	}

	dm->ParaCoord_PrepareRenderColorData();

	m_pcf->GetCanvas()->BindColorBuffer();
	RefreshAll();
}

void CControlFrame::OnValueSelect( wxCommandEvent& event )
{
	int _sel = event.GetSelection();
	wxString str = event.GetString();

	CDataManager *dm = m_datamanager;

	pair<t_pickset::iterator, bool> ret;
	ret = dm->m_pickset.insert( pair<unsigned int, unsigned int> (dm->GetCurrentItem(), _sel ) );

	if ( ret.second == false )
		dm->m_pickset.erase( ret.first );

	m_selectedValue->SetValue( str );

	RefreshAll();
}

void CControlFrame::OnVerticalScaleScroll( wxScrollEvent &event )
{
	int _scale = m_verticalscale_slider->GetValue();
	m_hf->GetCanvas()->SetVerticalScale( (float) _scale );
	m_hf->GetCanvas()->Refresh( false );
}

void CControlFrame::OnLogScaleCheck( wxCommandEvent& event )
{
	m_hf->GetCanvas()->SetLogScale(m_scalecheckbox->IsChecked());
	m_hf->GetCanvas()->Refresh( false );
}

void CControlFrame::OnAnimationSpeedSelect(wxCommandEvent& event )
{
	if ( m_if_animate )
		m_animateButton->SetLabel( _T("Animation: OFF"));

	int _sel = m_SpeedCombo->GetSelection();
	if ( _sel == 0 )
		m_invfps = 0.0001;
	else
		m_invfps = 1000.0f / (_sel * 5.0f);
	m_accum_millisec = 0;
}

void CControlFrame::OnRangeRatioSelect(wxCommandEvent& event)
{
	int _sel = m_rangeRatioCombo->GetSelection()+1;
	CDataManager *dm = m_datamanager;

	if ( dm != NULL )
	{
		dm->m_RangeRatio = _sel * 0.05;
		if ( dm->m_RangeRatio > 1.0 )
			dm->m_RangeRatio = 1.0;

		m_timeRangeSlider->SetRange(1, (unsigned int)((dm->m_MaxTime - dm->m_MinTime)*dm->m_RangeRatio));
		m_timeRangeSlider->SetValue(1);
		dm->m_TimeRange = m_timeRangeSlider->GetValue() * dm->m_RangeStep;
		char tmp[32];
		sprintf( tmp, "%d Seconds", dm->m_TimeRange);
		m_timeRangeText->ChangeValue( _T(tmp) );

		UpdateData();
	}
}

void CControlFrame::RefreshAll()
{
	if ( m_hf != NULL )
		m_hf->GetCanvas()->Refresh(false);
	if ( m_pcf != NULL )
		m_pcf->GetCanvas()->Refresh(false);
	if ( m_gf != NULL )
		m_gf->GetCanvas()->Refresh(false);
}

void CControlFrame::OnAnimate(wxCommandEvent& event)
{
	if ( m_if_animate )
	{
		m_if_animate = false;
		m_animateButton->SetLabel( _T("Animation: OFF"));
		m_accum_millisec = 0;
	}
	else
	{
		m_if_animate = true;
		m_animateButton->SetLabel( _T("Animation: ON"));
		m_pre_animatetime = clock();
	}

	if ( m_syncmenu->IsChecked() )
	{
		for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
		{
			if ( this != (*it) )
			{
				if ( GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
				{
					(*it)->m_if_animate = m_if_animate;
					if ( m_if_animate )
					{
						(*it)->m_animateButton->SetLabel( _T("Animation: ON"));
					}
					else
					{
						(*it)->m_animateButton->SetLabel( _T("Animation: OFF"));
					}
				}
			}
		}
	}
}

void CControlFrame::OnTimeSliderScrollBottom( wxScrollEvent &event )
{
	m_if_animate = false;
	m_animateButton->SetLabel( _T("Animation: OFF"));
}

void CControlFrame::OnIdle( wxIdleEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( m_if_animate )
	{
		bool if_asap = false;
		if ( m_SpeedCombo->GetSelection() == 0 ) // as fast as possible
			if_asap = true;
		
		clock_t _now = clock();
		m_accum_millisec += _now - m_pre_animatetime;
		if ( if_asap || m_accum_millisec > m_invfps )
		{
			if ( m_timeStartSlider->GetValue() < m_timeStartSlider->GetMax() )
			{
				m_timeStartSlider->SetValue( m_timeStartSlider->GetValue() + 30000 );
				wxScrollEvent _event;
				OnTimeSliderScroll( _event);
				if ( m_syncmenu->IsChecked() )
				{
					for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
					{
						if ( this != (*it) && GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
						{
							if ( (*it)->m_timeStartSlider->GetValue() <(*it)->m_timeStartSlider->GetMax() && (*it)->m_timeStartSlider->GetValue() > (*it)->m_timeStartSlider->GetMin() )
							{
								(*it)->m_timeStartSlider->SetValue( (*it)->m_timeStartSlider->GetValue() + 30000 );
								wxScrollEvent _event;
								(*it)->OnTimeSliderScroll( _event);
							}
							else
							{
								(*it)->m_if_animate = false;
								(*it)->m_animateButton->SetLabel( _T("Animation: OFF"));
							}
						}
					}
				}
			}
			else
			{
				m_if_animate = false;
				m_animateButton->SetLabel( _T("Animation: OFF"));
			}
			m_accum_millisec = 0;
		}
		else
		{
			m_pre_animatetime = _now;
		}
	}
	else if(dm->m_Graph.if_layout)
	{
		m_gf->GetCanvas()->Refresh(false);
	}
	else
		event.Skip();
	event.RequestMore();
}

void CControlFrame::OnClose( wxCloseEvent& event )
{
	if ( m_hf != NULL )
		m_hf->Close(true);
	if ( m_lf != NULL )
		m_lf->Close(true);
	//if (m_gf->IsShown())
	//	m_gf->Close(true);
	if(m_pcf != NULL)
		m_pcf->Close();
	if(m_gf != NULL)
		m_gf->Close();
	Destroy();
}

void CControlFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
	CDataManager *dm = m_datamanager;
	if ( dm != NULL )
		delete dm;
	Close(true);
}

void CControlFrame::OnTimeSliderScroll( wxScrollEvent &event )
{
	unsigned int _value = m_timeStartSlider->GetValue();
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady )
		return;

	dm->m_CurTime = ((time_t) _value) * dm->m_TimeStep +  dm->m_MinTime;
	UpdateDateTime();
	UpdateData();

	if ( m_syncmenu->IsChecked() )
	{
		for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
		{
			if ( this != (*it) )
			{
				if ( GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
				{
					(*it)->SetCurrentTime( dm->m_CurTime );
				}
			}
		}
	}
}

void CControlFrame::OnRangeSliderScroll( wxScrollEvent &event )
{
	unsigned int _value = m_timeRangeSlider->GetValue();
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady )
		return;

	char tmp[32];
	sprintf( tmp, "%d Seconds", _value * dm->m_RangeStep);
	m_timeRangeText->ChangeValue( _T(tmp) );

	dm->m_TimeRange = ((time_t)m_timeRangeSlider->GetValue()) * dm->m_RangeStep;

	UpdateData();
}

void CControlFrame::SetCurrentTime( time_t _curtime )
{
	if ( _curtime >= m_datamanager->m_MinTime && _curtime <= m_datamanager->m_MaxTime )
	{
		m_datamanager->m_CurTime = _curtime;
		m_timeStartSlider->SetValue((m_datamanager->m_CurTime - m_datamanager->m_MinTime)/m_datamanager->m_TimeStep);
		UpdateDateTime();

		UpdateData();
	}
}

void CControlFrame::UpdateDateTime()
{
	CDataManager *dm = m_datamanager;
	wxDateTime _datatime( dm->m_CurTime );

	struct tm *timeinfo = gmtime( (const time_t *)(&dm->m_CurTime) );

	m_DataPicker->SetValue( _datatime );
	m_hourCombo->Select( timeinfo->tm_hour );
	m_minuteCombo->Select( timeinfo->tm_min );
	m_secondCombo->Select( timeinfo->tm_sec );
}

void CControlFrame::OnNewControlFrame(wxCommandEvent &WXUNUSED(event))
{
	CControlFrame *currentframe = CControlFrame::Create(NULL, "AnimeVis Control Panel", wxPoint(0, 670), wxSize(1900, 350) );
	s_ControlFrameList.push_back( currentframe );
}

void CControlFrame::OnSyncAllFrames( wxCommandEvent &event)
{
	bool _ret = event.IsChecked();
	for ( vector<CControlFrame*>::iterator it = s_ControlFrameList.begin(); it != s_ControlFrameList.end(); it++ )
	{
		if ( this != (*it) )
		{
			(*it)->m_syncmenu->Check( _ret );
			if ( _ret && GetDataManager()->m_ifDataReady && (*it)->GetDataManager()->m_ifDataReady )
			{
				(*it)->SetCurrentTime( GetDataManager()->m_CurTime );
			}
		}
	}
}

void CControlFrame::OnLoadData(wxCommandEvent &WXUNUSED(event))
{
    wxString filename = wxFileSelector(_T("Select Data file"));
    if ( !filename )
        return;

	CControlFrame *currentframe = this;

	CDataManager *dm = currentframe->GetDataManager();

	if ( dm->m_ifDataReady )
		return;

	dm->LoadData( (char *) filename.c_str() );

	for ( unsigned int i = 1; i < dm->m_item_desc.size(); i++ )
	{
		currentframe->m_pListBox->Append( wxString::FromUTF8(dm->m_item_desc[i].name.c_str()));
		currentframe->m_pParaSel->Append( wxString::FromUTF8(dm->m_item_desc[i].name.c_str()));
		currentframe->m_pGraphSel->Append( wxString::FromUTF8(dm->m_item_desc[i].name.c_str()));
	}
	currentframe->m_pListBox->SetSelection(0);
	currentframe->m_timeStartSlider->SetRange(0,(dm->m_MaxTime - dm->m_MinTime)/dm->m_TimeStep);
	currentframe->m_timeStartSlider->SetValue(0);
	currentframe->m_timeRangeSlider->SetRange(1, (unsigned int)((dm->m_MaxTime - dm->m_MinTime)*dm->m_RangeRatio));
	currentframe->m_timeRangeSlider->SetValue(1);
	char tmp[32];
	sprintf( tmp, "%d Seconds", currentframe->m_timeRangeSlider->GetValue() * dm->m_RangeStep);
	currentframe->m_timeRangeText->ChangeValue( _T(tmp) );
	currentframe->UpdateDateTime();

	
	currentframe->m_Layout_gra->SetRange(0,100);//actual value range is [0-1], ratio is 100
	currentframe->m_Layout_gra->SetValue(25);
	currentframe->m_Layout_pointSize->SetRange(0,1000);//actual value  range is [0-10], ratio is 100
	currentframe->m_Layout_pointSize->SetValue(110);
	currentframe->m_Layout_att->SetRange(0,2000);//actual value range is [0-20], ratio is 100
	currentframe->m_Layout_att->SetValue(517);
	currentframe->m_Layout_rep->SetRange(0,1000);//actual value range is [0-10], ratio is 100
	currentframe->m_Layout_rep->SetValue(0);
	currentframe->m_Layout_range->SetRange(1,(dm->m_Graph).graph_node_num);
	currentframe->m_Layout_range->SetValue((int)sqrt((float)(dm->m_Graph).graph_node_num));

	sprintf(tmp,"Attractive Force %.2f",currentframe->m_Layout_att->GetValue()/100.0);
	currentframe->m_Layout_att_text->ChangeValue(_T(tmp));

	sprintf(tmp,"Repulsive Force %.2f",currentframe->m_Layout_rep->GetValue()/100.0);
	currentframe->m_Layout_rep_text->ChangeValue(_T(tmp));

	sprintf(tmp,"Gravity     Force %.2f",currentframe->m_Layout_gra->GetValue()/100.0);
	currentframe->m_Layout_gra_text->ChangeValue(_T(tmp));

	sprintf(tmp,"Point         Size %.2f",currentframe->m_Layout_pointSize->GetValue()/1000.0);
	currentframe->m_Layout_pointSize_text->ChangeValue(_T(tmp));

	sprintf(tmp,"Range       Size %.2f",currentframe->m_Layout_range->GetValue());
	currentframe->m_Layout_range_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.range = currentframe->m_Layout_range->GetValue();
	dm->m_LayoutParameter.attExponent = currentframe->m_Layout_att->GetValue()/100.0;
	dm->m_LayoutParameter.repExponent = currentframe->m_Layout_rep->GetValue()/100.0;
	dm->m_LayoutParameter.graFactor_previous =  currentframe->m_Layout_gra->GetValue()/100.0;
	dm->m_LayoutParameter.nodeSize =  currentframe->m_Layout_pointSize->GetValue()/1000.0;
	sprintf( tmp, "%d Seconds", currentframe->m_timeRangeSlider->GetValue() * dm->m_RangeStep);
	currentframe->m_timeRangeText->ChangeValue( _T(tmp) );

	currentframe->UpdateData();
}

void CControlFrame::UpdateData()
{
	CDataManager *dm = m_datamanager;

	dm->SelectTimeSpanIndex();

	char str[32];
	sprintf( str, "%d records", dm->GetNumberofQueriedRecords() );
	m_recordNumberText->ChangeValue( _T(str) );

	dm->PrepareHistogramData( dm->GetCurrentItem() );
	if ( dm->m_ParaCoordData.selectedItems.size() > 1 )
	{
		dm->PrepareParaCoordData();
		m_pcf->GetCanvas()->BindVertexAndIndexBuffer();
		m_pcf->GetCanvas()->BindColorBuffer();
	}
	dm->PrepareGraphData();
	m_hf->GetCanvas()->Refresh(false);
	m_pcf->GetCanvas()->Refresh(false);
	m_gf->GetCanvas()->Refresh(false);
}

void CControlFrame::OnParaListboxSelect(wxCommandEvent &event)
{
	int nSel = event.GetSelection();
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady )
		return;
	//wxArrayInt _selection;
	//m_pParaSel->GetSelections( _selection );

	bool found = false;
	for ( t_uintvector::iterator it = dm->m_ParaCoordData.selectedItems.begin(); it != dm->m_ParaCoordData.selectedItems.end(); it++)
	{
		if ( (*it) == nSel+1 )
		{
			dm->m_ParaCoordData.selectedItems.erase( it );
			found = true;
			break;
		}
	}
	if ( !found )
		dm->m_ParaCoordData.selectedItems.push_back( nSel+1 );
		
//	wxLogMessage( "%d Items are selected", event.GetSelection()) ;

	if ( dm->m_ParaCoordData.selectedItems.size() > 1 )
	{
		dm->PrepareParaCoordData();
		m_pcf->GetCanvas()->BindVertexAndIndexBuffer();
		m_pcf->GetCanvas()->BindColorBuffer();
	}
	m_pcf->GetCanvas()->Refresh(false);
}

void CControlFrame::OnListboxSelect(wxCommandEvent& event)
{
    int nSel = event.GetSelection()+1;
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady )
		return;

	m_valueSelectBox->Clear();
	for ( unsigned int i = 0; i < dm->m_item_desc[nSel].num_values; i++ )
	{
		m_valueSelectBox->Append( _T(dm->m_item_desc[nSel].value_names[i].c_str()) );
	}

	dm->SetCurrentItem( nSel );
	dm->PrepareHistogramData( nSel );
	wxLogMessage( "Item %d selected", nSel );
	m_hf->GetCanvas()->Refresh(false);
}



void CControlFrame::OnGraphListboxSelect(wxCommandEvent &event)
{
	CDataManager *dm = GetDataManager();
	if ( !dm->m_ifDataReady )
		return;
	wxArrayInt _selection;
	m_pGraphSel->GetSelections( _selection );

	wxLogMessage( "%d Items are selected", _selection.Count() );
}


void CControlFrame::OnLayoutAttScroll( wxScrollEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady || !dm->m_ifGraphReady )
		return;

	char tmp[32];
	sprintf(tmp,"Attractive Force %.2f",m_Layout_att->GetValue()/100.0);
	m_Layout_att_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.attExponent = m_Layout_att->GetValue()/100.0;
	UpdateData();
}
void CControlFrame::OnLayoutRepScroll( wxScrollEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady || !dm->m_ifGraphReady )
		return;

	char tmp[32];
	sprintf(tmp,"Repulsive Force %.2f",m_Layout_rep->GetValue()/100.0);
	m_Layout_rep_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.repExponent = m_Layout_rep->GetValue()/100.0;
	UpdateData();

}
void CControlFrame::OnLayoutGraScroll( wxScrollEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady || !dm->m_ifGraphReady )
		return;

	char tmp[32];
	sprintf(tmp,"Gravity     Force %.2f",m_Layout_gra->GetValue()/100.0);
	m_Layout_gra_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.graFactor_previous = m_Layout_gra->GetValue()/100.0;
	UpdateData();

}
void CControlFrame::OnLayoutRangeScroll( wxScrollEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady || !dm->m_ifGraphReady )
		return;

	char tmp[32];
	sprintf(tmp,"Range       Size %.2f",(float)m_Layout_range->GetValue());
	m_Layout_range_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.range = m_Layout_range->GetValue();
	m_gf->GetCanvas()->m_Ortho[0] = 0-m_Layout_range->GetValue();
	m_gf->GetCanvas()->m_Ortho[1] = m_Layout_range->GetValue();
	m_gf->GetCanvas()->m_Ortho[2] = 0-m_Layout_range->GetValue();
	m_gf->GetCanvas()->m_Ortho[3] = m_Layout_range->GetValue();
	
	UpdateData();

}
void CControlFrame::OnLayoutSizeScroll( wxScrollEvent &event )
{
	CDataManager *dm = m_datamanager;
	if ( !dm->m_ifDataReady || !dm->m_ifGraphReady )
		return;

	char tmp[32];
	sprintf(tmp,"Point         Size %.2f",m_Layout_pointSize->GetValue()/1000.0);
	m_Layout_pointSize_text->ChangeValue(_T(tmp));

	dm->m_LayoutParameter.nodeSize = m_Layout_pointSize->GetValue()/1000.0;
	UpdateData();

}

void CControlFrame::OnGraphCreate(wxCommandEvent& event)
{
	wxArrayInt _selection;
	m_pGraphSel->GetSelections( _selection );
	if((int)(_selection.Count())!=2)
	{
		wxMessageBox( _("You need choose two items to create graph!"),
			_("Message"),wxOK | wxICON_INFORMATION, this );
	}
	else
	{
		// wxLogMessage( "Now Layout the graph items:%s and items:%s");
		CDataManager *dm = m_datamanager;
		dm->m_Graph.selected_item[0] = _selection[0]+1;
		dm->m_Graph.selected_item[1] = _selection[1]+1;
		dm->m_ifGraphReady = false;
		dm->CreateGraph();
		dm->PrepareGraphData();

		m_Layout_range->SetRange(1,(dm->m_Graph).graph_node_num);
		m_Layout_range->SetValue((int)sqrt((float)(dm->m_Graph).graph_node_num));
		dm->m_LayoutParameter.range = m_Layout_range->GetValue();
		char tmp[32];
		sprintf(tmp,"Range       Size %.2f",(float)m_Layout_range->GetValue());
		m_Layout_range_text->ChangeValue(_T(tmp));

		m_gf->GetCanvas()->m_Ortho_backup[0]=-1.0;
		m_gf->GetCanvas()->m_Ortho_backup[1]=1.0;
		m_gf->GetCanvas()->m_Ortho_backup[2]=-1.0;
		m_gf->GetCanvas()->m_Ortho_backup[3]=1.0;

		m_gf->GetCanvas()->m_Ortho[0] =m_gf->GetCanvas()->m_Ortho[2] =0- dm->m_LayoutParameter.range;
		m_gf->GetCanvas()->m_Ortho[1] =m_gf->GetCanvas()->m_Ortho[3] =dm->m_LayoutParameter.range;

		m_gf->GetCanvas()->Refresh(false);
	}

}

void CControlFrame::OnGraphForce(wxCommandEvent& event)
{	
	CDataManager *dm = m_datamanager;
	if(dm->m_Graph.if_layout ==false)
	{
		dm->m_Graph.if_layout = true;
		m_pGraphForceApply->SetLabel(_T("Layout: ON"));
	}
	else{
		dm->m_Graph.if_layout = false;
		m_pGraphForceApply->SetLabel(_T("Layout: OFF"));
		m_gf->GetCanvas()->m_Ortho_backup[0] = m_gf->GetCanvas()->m_Ortho[0];
		m_gf->GetCanvas()->m_Ortho_backup[1] = m_gf->GetCanvas()->m_Ortho[1];
		m_gf->GetCanvas()->m_Ortho_backup[2] = m_gf->GetCanvas()->m_Ortho[2];
		m_gf->GetCanvas()->m_Ortho_backup[3] = m_gf->GetCanvas()->m_Ortho[3];
	}

	m_gf->GetCanvas()->Refresh(false);

}

void CControlFrame::OnLayoutLoadGraph(wxCommandEvent& event)
{
	CDataManager *dm = m_datamanager;
	if(!dm->m_ifDataReady)
	{
		wxMessageBox( _("You need to load data first!"),
			_("Message"),wxOK | wxICON_INFORMATION, this );
		return;
	}


	wxFileDialog 
		openFileDialog(this, _("Open file"), "", "",
		"All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	if(dm->LoadGraph((char*)openFileDialog.GetPath().c_str()))
	{

		dm->PrepareGraphData();
	

		wxArrayInt _selection;
		m_pGraphSel->GetSelections( _selection );
		for ( int i = 0; i < _selection.Count(); i++ )
			m_pGraphSel->Deselect(i);
		m_pGraphSel->SetSelection(dm->m_Graph.selected_item[0]-1);
		m_pGraphSel->SetSelection(dm->m_Graph.selected_item[1]-1);

		m_Layout_range->SetRange(1,(dm->m_Graph).graph_node_num);
		m_Layout_range->SetValue(dm->m_LayoutParameter.range);
		dm->m_LayoutParameter.range = m_Layout_range->GetValue();
		char tmp[32];
		sprintf(tmp,"Range       Size %.2f",(float)m_Layout_range->GetValue());
		m_Layout_range_text->ChangeValue(_T(tmp));

		m_gf->GetCanvas()->m_Ortho_backup[0]=-1.0;
		m_gf->GetCanvas()->m_Ortho_backup[1]=1.0;
		m_gf->GetCanvas()->m_Ortho_backup[2]=-1.0;
		m_gf->GetCanvas()->m_Ortho_backup[3]=1.0;

		m_gf->GetCanvas()->m_Ortho[0] =m_gf->GetCanvas()->m_Ortho[2] =0- dm->m_LayoutParameter.range;
		m_gf->GetCanvas()->m_Ortho[1] =m_gf->GetCanvas()->m_Ortho[3] =dm->m_LayoutParameter.range;

		m_gf->GetCanvas()->Refresh(false);

// 		dm->PrepareEdgeLayoutEdgeNode();
// 		m_gf->GetCanvas()->Refresh();
		wxLogMessage(openFileDialog.GetPath());
	}
	else
		wxLogMessage("error: Loading the graph data failed %s",openFileDialog.GetPath().c_str());
}

void CControlFrame::OnLayoutEdgeShow(wxCommandEvent& event)
{
	CDataManager *dm = m_datamanager;
	
	if(dm->m_Graph.edge_show == true)
	{
		dm->m_Graph.edge_show = false;
		m_pEdge->SetLabel(_T("EdgeShow: Off"));
	}
	else
	{
		dm->m_Graph.edge_show = true;
		m_pEdge->SetLabel(_T("EdgeShow: On"));
	}
	m_gf->GetCanvas()->Refresh(false);

}
void CControlFrame::OnLayoutPin(wxCommandEvent& event)
{
	CDataManager *dm = m_datamanager;
	if(dm->m_Graph.node_pin == true)
	{
		dm->m_Graph.node_pin = false;
		m_pPin->SetLabel(_T("Pin: Off"));
	}
	else
	{
		dm->m_Graph.node_pin = true;
		m_pPin->SetLabel(_T("Pin: On"));
	}
	m_gf->GetCanvas()->Refresh(false);
}

void CControlFrame::OnLayoutSaveGraph(wxCommandEvent& event)
{
	CDataManager *dm = m_datamanager;
	if(!dm->m_ifDataReady)
	{
		wxMessageBox( _("You need to load data first!"),
			_("Message"),wxOK | wxICON_INFORMATION, this );
		return;
	}

	wxFileDialog 
		saveFileDialog(this, _("Save file"), "", ".graph",
		"files (*.graph*)|*.graph*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;

	if(dm->SaveGraph((char*)saveFileDialog.GetPath().c_str()))
	{
		m_gf->GetCanvas()->Refresh(false);
		wxLogMessage(saveFileDialog.GetPath());
	}
	else
		wxLogMessage("error: Saving the graph data failed %s",saveFileDialog.GetPath().c_str());


	wxLogMessage(saveFileDialog.GetPath());

}

}// end of name space