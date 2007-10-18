#ifndef _MAINFRAME_H
#define _MAINFRAME_H

#include <wx/taskbar.h>

class MainApp : public wxApp
{
public:
	MainApp() {}
	bool OnInit();
};

DECLARE_APP(MainApp)

class MainPanel;
class MainTrayIcon;

class MainFrame : public wxFrame 
{
private:
	MainPanel *m_panel;
	MainTrayIcon *m_tray;
	
	void restore();

public:
	MainFrame();
	~MainFrame();
	
	void forward(wxCommandEvent& event);
	void menu_restore(wxCommandEvent & event);
	void tray_restore(wxTaskBarIconEvent & event);
	void exit(wxCommandEvent& event);
	void about(wxCommandEvent& event);
	void close(wxCloseEvent& event);
	void preference(wxCommandEvent& event);

	wxMenu* GetTrayMenu();

	DECLARE_EVENT_TABLE()
};

class wxToggleButton;
class wxGrid;
class MainFunction;

class MainPanel : public wxPanel
{
private:
	wxComboBox *m_cards;
	wxToggleButton *m_detect;
	wxListBox *m_maclist;
	wxButton *m_kill;
	wxToggleButton *m_autokill;
	wxButton *m_mark;
	wxStaticText *m_dstmac;

	MainFunction *m_mainfunc;
public:
	MainPanel(wxFrame *frame);
	MainFunction* GetMainFunction();
};

class MainTrayIcon : public wxTaskBarIcon
{
private:
	MainFrame *m_mainframe;
public:
	MainTrayIcon(MainFrame* mainframe) {m_mainframe = mainframe;}
	wxMenu* CreatePopupMenu() {return m_mainframe->GetTrayMenu();}
};

#endif

