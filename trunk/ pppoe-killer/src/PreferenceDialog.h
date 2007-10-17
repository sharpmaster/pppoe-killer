#ifndef _PREFERENCEDIALOG_H
#define _PREFERENCEDIALOG_H

#include <wx/notebook.h>

class PacketPanel;
class MainFunction;

class PreferenceDialog : public wxDialog
{
public:
	PreferenceDialog(wxWindow* parent, MainFunction *func);
	~PreferenceDialog();
private:
	void OnExit(wxCommandEvent& event);
	
	wxNotebook *m_notebook;
	PacketPanel *m_packet_panel;

	MainFunction *m_mainfunc;

	DECLARE_EVENT_TABLE()
};

class PacketPanel : public wxPanel
{
public:
	PacketPanel(wxWindow* parent);
	~PacketPanel() {}

	void setInterval(unsigned int value);
	unsigned int getInterval();
private:
	wxTextCtrl *m_interval;
};

#endif
