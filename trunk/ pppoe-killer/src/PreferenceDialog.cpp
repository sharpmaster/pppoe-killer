#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <boost/lexical_cast.hpp>
#include "PreferenceDialog.h"
#include "MainFunction.h"
#include "Resource.h"

BEGIN_EVENT_TABLE(PreferenceDialog, wxDialog)
	EVT_BUTTON(wxID_OK, PreferenceDialog::OnExit)
	EVT_BUTTON(wxID_CANCEL, PreferenceDialog::OnExit)
END_EVENT_TABLE()

PreferenceDialog::PreferenceDialog(wxWindow* parent, MainFunction *func)
	: wxDialog(parent, PKID_PREFDIALOG, _T("設定選項"))
{
	m_notebook = new wxNotebook(this, wxID_ANY);
	m_packet_panel = new PacketPanel(m_notebook);

	m_notebook->AddPage(m_packet_panel, _T("封包設定"));

	wxBoxSizer *root_sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *button_sizer = CreateButtonSizer(wxOK | wxCANCEL);
	root_sizer->Add(m_notebook, 1, wxEXPAND | wxALL, 10);
	root_sizer->Add(button_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT |wxBOTTOM, 10);
	
	SetSizer(root_sizer);
	root_sizer->Fit(this);
	root_sizer->SetSizeHints(this);

	m_mainfunc = func;

	m_packet_panel->setInterval(m_mainfunc->getPacketInterval());
}

PreferenceDialog::~PreferenceDialog()
{
}

void PreferenceDialog::OnExit(wxCommandEvent& event)
{
	int id = event.GetId();

	if(id == wxID_OK)
	{
		m_mainfunc->setPacketInterval(m_packet_panel->getInterval());
		m_mainfunc->saveConfig();
	}

	EndModal(id);
}

PacketPanel::PacketPanel(wxWindow* parent)
	: wxPanel(parent)
{
	wxBoxSizer *root_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *interval_label = new wxStaticText(this, wxID_ANY, _T("封包發送間隔時間(毫秒)"));

	m_interval = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_RIGHT);
	root_sizer->Add(interval_label, 0, wxALL, 5);
	root_sizer->Add(m_interval, 0, wxALL, 5);
	SetSizer(root_sizer);
	root_sizer->Fit(this);
	root_sizer->SetSizeHints(this);
}

unsigned int PacketPanel::getInterval()
{
	long v;
	wxString s = m_interval->GetValue();

	if(s.ToLong(&v) == false)
		v = 0;

	if(v < 0)
		v = 0;
	
	return (unsigned int)v;
}

void PacketPanel::setInterval(unsigned int value)
{
	m_interval->SetValue(wxString::Format("%d", value));
}
