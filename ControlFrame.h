#pragma once

#include "wx/wx.h"
#include <vector>
using namespace std;

class wxNotebook;
class wxListBox;
class wxPanel;
class wxBoxSizer;
class wxComboBox;
class wxDatePickerCtrl;

namespace AMT
{
class CLOGFrame;
class CHistogramFrame;
class CGraphFrame;
class CParaCoordFrame;
class ControlFrame;
class CDataManager;

class CControlFrame : public wxFrame
{
public:
	static CControlFrame* Create( CLOGFrame *lf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	static vector<CControlFrame*> s_ControlFrameList;

	CControlFrame( CLOGFrame *lf, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

	void OnQuit(wxCommandEvent &WXUNUSED(event));
	void OnLoadData(wxCommandEvent &WXUNUSED(event));
	void OnClose( wxCloseEvent &event );
	void OnListboxSelect(wxCommandEvent &event);
	void OnParaListboxSelect(wxCommandEvent &event);
	void OnTimeSliderScroll( wxScrollEvent &event );
	void OnRangeSliderScroll( wxScrollEvent &event );
	void OnIdle( wxIdleEvent &event );
	void OnTimeSliderScrollBottom( wxScrollEvent &event );
	void OnAnimate(wxCommandEvent& event);
	void OnRangeRatioSelect(wxCommandEvent& event );
	void OnAnimationSpeedSelect(wxCommandEvent& event );
	void OnLogScaleCheck( wxCommandEvent& event );
	void OnVerticalScaleScroll( wxScrollEvent &event );
	void OnValueSelect( wxCommandEvent& event );
	void OnClearSelection( wxCommandEvent& event );

	void AttachHistogramFrame( CHistogramFrame *hf ) { m_hf = hf; }
//	void AttachGraphFrame( CGraphFrame *gf ) { m_gf = gf; }	
	void AttachParaCoordFrame( CParaCoordFrame *pcf ) { m_pcf = pcf; }
	void SetCurrentTime( time_t _curtime );

	void OnGraphListboxSelect(wxCommandEvent &event);
	void OnLayoutAttScroll( wxScrollEvent &event );
	void OnLayoutRepScroll( wxScrollEvent &event );
	void OnLayoutGraScroll( wxScrollEvent &event );
	void OnLayoutRangeScroll( wxScrollEvent &event );
	void OnLayoutSizeScroll( wxScrollEvent &event );
	void OnGraphForce(wxCommandEvent& event);
	void OnGraphCreate(wxCommandEvent& event);
	void OnLayoutLoadGraph(wxCommandEvent& event);
	void OnLayoutSaveGraph(wxCommandEvent& event);
	void OnLayoutEdgeShow(wxCommandEvent& event);
	void OnLayoutPin(wxCommandEvent& event);

	void RefreshAll();

	CDataManager* GetDataManager() { return m_datamanager; }

	CLOGFrame *m_lf;
	CHistogramFrame *m_hf;
	CGraphFrame* m_gf;	
	CParaCoordFrame *m_pcf;

	wxBoxSizer	*m_sizer;
    wxPanel		*m_panel;
	wxNotebook	*m_pNotebook;
    wxListBox	*m_pListBox;
	wxListBox	*m_valueSelectBox;
	wxListBox	*m_pParaSel;
	wxSlider	*m_timeStartSlider, *m_timeRangeSlider;
	wxComboBox	*m_rangeRatioCombo;
	wxButton	*m_animateButton;
	wxComboBox	*m_SpeedCombo;
	wxCheckBox	*m_scalecheckbox;
	wxSlider	*m_verticalscale_slider;
	wxMenuItem	*m_syncmenu;

	wxButton	*m_clearSelectionButton;
	
	wxComboBox *m_hourCombo, *m_minuteCombo, *m_secondCombo;

	wxTextCtrl *m_timeRangeText, *m_recordNumberText, *m_selectedValue;

	wxDatePickerCtrl *m_DataPicker;

	wxListBox *m_pGraphSel;
	wxButton  *m_pGraphForceApply;
	wxButton  *m_pGraphCreate;
	wxButton  *m_LayoutLoadGraph, *m_LayoutSaveGraph;
	wxButton  *m_pEdge;
	wxButton  *m_pPin;
	wxSlider *m_Layout_att, *m_Layout_rep,*m_Layout_gra,*m_Layout_pointSize,*m_Layout_range;
	wxTextCtrl *m_Layout_att_text, *m_Layout_rep_text,*m_Layout_gra_text,*m_Layout_pointSize_text,*m_Layout_range_text;


	bool m_if_animate;

	void UpdateDateTime();
	void UpdateData();

private:


// 	CHistogramFrame *m_hf;
// 	CGraphFrame* m_gf;	
// 	CParaCoordFrame *m_pcf;
	CDataManager *m_datamanager;


	float m_invfps;

	unsigned int m_accum_millisec;
	clock_t m_pre_animatetime;

	void OnNewControlFrame(wxCommandEvent &WXUNUSED(event));
	void OnSyncAllFrames( wxCommandEvent &event);


	DECLARE_EVENT_TABLE()
};

}