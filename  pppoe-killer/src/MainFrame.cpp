#define _WINSOCKAPI_
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <wx/aboutdlg.h>
#include "pppoe.xpm"
#include "Resource.h"
#include <glib/GProperties.h>
#include <glib/GThread.h>
#include <glib/net/GNetTool.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "MainFrame.h"
#include "MainFunction.h"
#include "PreferenceDialog.h"

using namespace glib;
using namespace glib::net;
using namespace std;
using namespace log4cxx;

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() {
#ifdef WIN32
	HANDLE hMutexOneInstance = CreateMutex( NULL, FALSE, _T("PKILLER_GUID"));
	bool AlreadyRunning = ( GetLastError() == ERROR_ALREADY_EXISTS || 
                           GetLastError() == ERROR_ACCESS_DENIED);
	if(AlreadyRunning == true)
	{
		::wxMessageBox(wxT("PPPoE Killer正在執行中"), wxT("Error"), wxOK | wxICON_ERROR);
		return false;
	}
#endif

	try
	{
		PropertyConfigurator::configure(File("log4cxx.conf"));
	}
	catch(...)
	{
		// do something
	}
	
	wxIcon icon(pppoe_xpm);
	// Create the main frame window
	MainFrame *frame = new MainFrame;
	frame->SetIcon(icon);
	frame->SetSize(450, 300);
	frame->Centre();
	frame->Show(true);
	SetTopWindow(frame);
	
	return true;
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(PKID_ENTERMAC, MainFrame::forward)
	EVT_MENU(PKID_ENTERISPMAC, MainFrame::forward)
	EVT_MENU(PKID_SAVE, MainFrame::forward)
	EVT_MENU(PKID_LOAD, MainFrame::forward)
	EVT_MENU(PKID_EXIT, MainFrame::exit)
	EVT_MENU(PKID_RESTORE, MainFrame::menu_restore)
	EVT_MENU(PKID_SETTING, MainFrame::preference)
	EVT_MENU(wxID_ABOUT, MainFrame::about)
	EVT_TASKBAR_LEFT_DCLICK(MainFrame::tray_restore)
	EVT_CLOSE(MainFrame::close)
END_EVENT_TABLE()

MainFrame::MainFrame()
       : wxFrame(NULL, PKID_MAINFRAME, _T("PPPoE Killer v1.1"),
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
	m_panel = new MainPanel(this);
	m_tray = new MainTrayIcon(this);
	wxMenu *func_menu = new wxMenu;
	wxMenu *entermac_menu = new wxMenu;
	wxMenu *help_menu = new wxMenu;
	wxMenuBar *menu_bar = new wxMenuBar;

	//m_tray->SetEvtHandlerEnabled(false);
	m_tray->SetNextHandler(this);

	entermac_menu->Append(PKID_ENTERMAC, _T("目標電腦(&T)"));
	entermac_menu->Append(PKID_ENTERISPMAC, _T("ISP(&I)"));

	func_menu->AppendSubMenu(entermac_menu, _T("輸入MAC(&M)"));
	func_menu->Append(PKID_LOAD, _T("讀取(&L)"));
	func_menu->Append(PKID_SAVE, _T("儲存(&S)"));
	func_menu->Append(PKID_SETTING, _T("設定(&P)"));
	func_menu->AppendSeparator();
	func_menu->Append(PKID_EXIT, _T("結束(&X)"));
	help_menu->Append(wxID_ABOUT, _T("關於(&A)"));
	
	menu_bar->Append(func_menu, _T("選項(&F)"));
	menu_bar->Append(help_menu, _T("說明(&H)"));
	
	SetMenuBar(menu_bar);

	boost::thread *helper = new boost::thread(boost::bind(&MainFrame::helperthread, this));
}

MainFrame::~MainFrame()
{
	delete m_tray;
}

void MainFrame::restore()
{
	m_tray->RemoveIcon();
	Show(true);
}

void MainFrame::helperthread()
{
	while(true)
	{
		m_panel->GetMainFunction()->refreshButton();
		GThread::sleep(500);
	}
}

void MainFrame::menu_restore(wxCommandEvent & event)
{
	restore();
}

void MainFrame::tray_restore(wxTaskBarIconEvent & event)
{
	restore();
}

wxMenu* MainFrame::GetTrayMenu()
{
	wxMenu *traymenu = new wxMenu;
	traymenu->Append(PKID_RESTORE, _T("開啟PPPoE Killer(&O)"));
	traymenu->Append(PKID_EXIT, _T("結束(&X)"));
	traymenu->SetNextHandler(this);
	
	return traymenu;

}

void MainFrame::forward(wxCommandEvent & event)
{
	m_panel->GetMainFunction()->ProcessEvent(event);
}

void MainFrame::about(wxCommandEvent & event)
{
	wxAboutDialogInfo info;

	info.SetName(wxT("PPPoE Killer"));
	info.SetVersion(wxT("v1.1"));
	info.SetWebSite(wxT("http://www.google.com/group/pppoe-killer"));
	info.SetCopyright(wxT("(C) 2007 ptthippo@ptt.cc <ptthippo@gmail.com>"));

	wxAboutBox(info);
}

void MainFrame::exit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::preference(wxCommandEvent & event)
{
	PreferenceDialog dialog(this, m_panel->GetMainFunction());
	dialog.SetSize(400, 300);
	dialog.ShowModal();
}

void MainFrame::close(wxCloseEvent& event)
{
	if(event.CanVeto())
	{
		m_tray->SetIcon(this->GetIcon(), "PPPoE Killer v1.1");
		this->Show(false);
	}
	else
	{
		this->Destroy();
	}
}

MainPanel::MainPanel(wxFrame *frame)
	: wxPanel(frame, wxID_ANY)
{
	vector<GNetAdapter> adapters;

	adapters = GNetTool::getLocalAdapters();
	m_cards = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	for(vector<GNetAdapter>::size_type i = 0; i < adapters.size(); i++)
	{
		string *clientData = new string;
		*clientData = adapters[i].getName();
#ifdef WIN32
		m_cards->Append(adapters[i].getDescription(), (void*)clientData);
#else
		m_cards->Append(adapters[i].getName(), (void*)clientData);
#endif
	}

	if(adapters.size() > 0)
		m_cards->SetSelection(0);

	m_detect = new wxToggleButton(this, PKID_DETECT, "偵測");
	wxBoxSizer *cards_sizer = new wxBoxSizer(wxHORIZONTAL);
	cards_sizer->Add(m_cards, 1, wxEXPAND | wxALL, 10);
	cards_sizer->Add(m_detect, 0, wxEXPAND | wxALL, 10);

	m_maclist = new wxListBox(this, PKID_MACLIST);

	m_kill = new wxButton(this, PKID_KILL, "殺掉");
	m_autokill = new wxToggleButton(this, PKID_AUTOKILL, "自動殺掉");
	m_mark = new wxButton(this, PKID_MARK, "標記");
	wxBoxSizer *toolbar_sizer = new wxBoxSizer(wxVERTICAL);
	toolbar_sizer->Add(m_kill, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);
	toolbar_sizer->AddSpacer(5);
	toolbar_sizer->Add(m_autokill, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
	toolbar_sizer->AddSpacer(5);
	toolbar_sizer->Add(m_mark, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
	wxBoxSizer *list_sizer = new wxBoxSizer(wxHORIZONTAL);
	list_sizer->Add(m_maclist, 1, wxEXPAND | wxALL, 10);
	list_sizer->Add(toolbar_sizer, 0, wxEXPAND | wxALL);

	wxStaticText *tlabel = new wxStaticText(this, wxID_ANY, "ISP MAC - ");
	m_dstmac = new wxStaticText(this, wxID_ANY, "");
	wxBoxSizer *lbl_sizer = new wxBoxSizer(wxHORIZONTAL);
	lbl_sizer->Add(tlabel, 0, wxEXPAND | wxALL);
	lbl_sizer->Add(m_dstmac, 0, wxEXPAND | wxALL);

	wxBoxSizer *root_sizer = new wxBoxSizer(wxVERTICAL);
	root_sizer->Add(cards_sizer, 0, wxEXPAND | wxALL);
	root_sizer->Add(list_sizer, 1, wxEXPAND | wxALL);
	root_sizer->Add(lbl_sizer, 0, wxEXPAND | wxALL, 10);

	SetSizer(root_sizer);

	m_mainfunc = new MainFunction(m_cards, m_maclist, m_dstmac, m_autokill, m_kill);
	m_detect->PushEventHandler(m_mainfunc);
	m_kill->PushEventHandler(m_mainfunc);
	m_autokill->PushEventHandler(m_mainfunc);
	m_mark->PushEventHandler(m_mainfunc);
	m_maclist->PushEventHandler(m_mainfunc);

	m_mainfunc->loadConfig();
}

MainFunction* MainPanel::GetMainFunction()
{
	return m_mainfunc;
}

