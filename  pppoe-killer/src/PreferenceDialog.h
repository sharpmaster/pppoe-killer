#ifndef _PREFERENCEDIALOG_H
#define _PREFERENCEDIALOG_H

#include <wx/notebook.h>

class PacketPanel;

class PreferenceDialog : public wxDialog
{
public:
	PreferenceDialog(wxWindow* parent);
	~PreferenceDialog();
private:
	wxNotebook *m_notebook;
	PacketPanel *m_packet_panel;
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
