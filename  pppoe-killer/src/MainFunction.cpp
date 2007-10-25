#include <glib/net/GNetTool.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <fstream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/find.hpp>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <glib/GProperties.h>
#include <glib/GLogger.h>
#include <glib/GBase64.h>
#include <glib/GTime.h>
#include "Resource.h"
#include "AutoKiller.h"
#include "ManualKiller.h"
#include "MainFunction.h"
#include "GPacketDetector.h"
#include "VictimEntry.h"
#include "PADTGenerator.h"


using namespace std;
using namespace glib;
using namespace glib::net;
using namespace log4cxx;
namespace ba = boost::algorithm;

MainFunction::MainFunction(wxComboBox* cards, wxListBox *list, wxStaticText *ispmac,
						   wxToggleButton *autokill_btn, wxButton *kill_btn)
{
	m_cards = cards;
	m_maclist = list;
	m_ispmac = ispmac;
	m_autokill_btn = autokill_btn;
	m_kill_btn = kill_btn;

	m_padi_dtr = NULL;
	m_pado_dtr = NULL;

	fill(m_dstmac.begin(), m_dstmac.end(), (char)0);
	m_packet_interval = 0;

	int index = m_cards->GetSelection();
	if(index == wxNOT_FOUND)
		m_func_ifname = "";
	else
		m_func_ifname = *((string*)(m_cards->GetClientData(index)));
	

	m_logger = Logger::getLogger("main");
}

MainFunction::~MainFunction()
{
	if(m_padi_dtr != NULL)
	{
		if(m_padi_dtr->isAlive() == true)
			m_padi_dtr->waitStop();
		delete m_padi_dtr;
		m_padi_dtr = NULL;
	}

	if(m_pado_dtr != NULL)
	{
		if(m_pado_dtr->isAlive() == true)
			m_pado_dtr->waitStop();
		delete m_pado_dtr;
		m_pado_dtr = NULL;
	}

	if(m_arpreq_dtr != NULL)
	{
		if(m_arpreq_dtr->isAlive() == true)
			m_arpreq_dtr->waitStop();
		delete m_arpreq_dtr;
		m_arpreq_dtr = NULL;
	}

	clear_data();
}

void MainFunction::refreshButton()
{
	pc_list_selected();
}

string MainFunction::getMACString(const boost::array<unsigned char, 6> & mac)
{
	stringstream ss;
	string s = GNetTool::getMACString(mac);

	ss << s.substr(0, 2) << ":" << s.substr(2, 2) << ":" <<
		 s.substr(4, 2) << ":" << s.substr(6, 2) << ":" <<
		 s.substr(8, 2) << ":" << s.substr(10, 2);

	return ss.str();
}

boost::array<unsigned char, 6> MainFunction::parseMAC(const std::string & macstr)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(":");
	tokenizer tokens(macstr, sep);
	stringstream ss;

	for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
	   	ss << *tok_iter;

	boost::array<unsigned char, 6> ret;

	try
	{
		ret = GNetTool::parseMAC(ss.str());
	}
	catch(...)
	{
		throw;
	}

	return ret;
}

void MainFunction::clear_data()
{
	fill(m_dstmac.begin(), m_dstmac.end(), (char)0);

	VITE ite = m_victims.begin();
	while(ite != m_victims.end())
	{
		ite->second->stopKiller();
		ite++;
	}
	m_victims.clear();
}

void MainFunction::append_data(const boost::array<unsigned char, 6> & macbin, const string & ifname,
								const string & desc)
{
	VictimEntry *v = new VictimEntry();
	string s;

	v->setMac(macbin);
	s = getMACString(macbin);
	v->setInterfaceName(ifname);
	v->setLastSeenDate(GTime::GetTimeString());
	v->setDesc(desc);
	v->setAutoKill(false);

	m_victims.insert(s, v);
}

string MainFunction::getliststr(const VictimEntry & entry)
{
	string macstr;
	string ret;

	macstr = getMACString(entry.getMac());
	ret = macstr + " ";
	ret += entry.getLastSeenDate();

	if(GNetTool::isLocalMAC(entry.getMac()) == true)
		ret += " [我的電腦]";

	ret += " ";
	ret += entry.getDesc();
	
	return ret;
}

void MainFunction::padi_detected(const unsigned char* packet, int len)
{
	padi_srcmac_detected(packet+6);
}

void MainFunction::padi_srcmac_detected(const unsigned char* srcmac)
{
	string mac;
	boost::array<unsigned char, 6> macbin;
	VITE ite;
	boost::mutex::scoped_lock *lock;

	copy(srcmac, srcmac+6, macbin.begin());
	mac = getMACString(macbin);
	lock = new boost::mutex::scoped_lock(m_mutex);
	
	ite = m_victims.find(mac);
	if(ite == m_victims.end())
	{
		append_data(macbin, m_func_ifname);
		m_maclist->Append(this->getliststr(m_victims[mac]));
	}
	else
	{
		ite->second->setLastSeenDate(GTime::GetTimeString());
		wxString macstr = getMACString(ite->second->getMac());
		
		for(unsigned int i = 0; i < m_maclist->GetCount(); i++)
		{
			if(m_maclist->GetString(i).find(macstr) != string::npos)
			{
				m_maclist->SetString(i, this->getliststr(*ite->second));
				break;
			}
		}
	}

	delete lock;
}

void MainFunction::pado_detected(const unsigned char* packet, int len)
{
	boost::array<unsigned char, 6> macbin;
	
	copy(packet+6, packet+6+6, macbin.begin());
	string s = getMACString(macbin);
	m_ispmac->SetLabel(s);
	m_dstmac = macbin;
}

void MainFunction::arpreq_detected(const unsigned char* packet, int len)
{
	boost::array<unsigned char, 6> macbin;
	unsigned long srcip;
	struct in_addr ip_addr;
	string mac, ipstr;
	VITE ite;
	
	copy(packet+6, packet+6+6, macbin.begin());
	mac = getMACString(macbin);
	memcpy(&srcip, packet+28, 4);
	//srcip = ntohs(srcip);
	ip_addr.s_addr = srcip;
	ipstr = inet_ntoa(ip_addr);

	boost::mutex::scoped_lock lock(m_mutex);

	ite = m_victims.find(mac);
	if(ite == m_victims.end())
	{
		append_data(macbin, m_func_ifname, ipstr);
		m_maclist->Append(this->getliststr(m_victims[mac]));
	}
	else
	{
		ite->second->setLastSeenDate(GTime::GetTimeString());
		string desc = ite->second->getDesc();
		if(!(ba::find_first(desc, ipstr)))
		{
			desc += (" " + ipstr);
			ite->second->setDesc(desc);
		}
			
		wxString macstr = getMACString(ite->second->getMac());
		
		for(unsigned int i = 0; i < m_maclist->GetCount(); i++)
		{
			if(m_maclist->GetString(i).find(macstr) != string::npos)
			{
				m_maclist->SetString(i, this->getliststr(*ite->second));
				break;
			}
		}
	}
	
}

void MainFunction::pc_padi_detect(wxToggleButton *btn)
{
	if(btn->GetValue() == true)
	{
		int index = m_cards->GetSelection();
		if(index == wxNOT_FOUND)
		{
			btn->SetValue(false);
			return;
		}
		string *ifname = (string*)(m_cards->GetClientData(index));
		if(ifname == NULL)
		{
			btn->SetValue(false);
			return;
		}

		m_padi_dtr = new GPacketDetector("ether[0]=255 and ether proto 0x8863", *ifname);
		m_padi_dtr->AddReactor(boost::bind(&MainFunction::padi_detected, this, _1, _2));
		m_pado_dtr = new GPacketDetector("ether[15]=7 and ether proto 0x8863", *ifname);
		m_pado_dtr->AddReactor(boost::bind(&MainFunction::pado_detected, this, _1, _2));
		m_arpreq_dtr = new GPacketDetector("arp and ether[20:2]=0x1", *ifname);
		m_arpreq_dtr->AddReactor(boost::bind(&MainFunction::arpreq_detected, this, _1, _2));
		m_padi_dtr->start();
		m_pado_dtr->start();
		m_arpreq_dtr->start();

		m_func_ifname = *ifname;
		btn->SetLabel(wxT("偵測中"));
		m_cards->Enable(false);
	}
	else
	{
		m_padi_dtr->waitStop();
		delete m_padi_dtr;
		m_padi_dtr = NULL;

		m_pado_dtr->waitStop();
		delete m_pado_dtr;
		m_pado_dtr = NULL;

		m_arpreq_dtr->waitStop();
		delete m_arpreq_dtr;
		m_arpreq_dtr = NULL;

		m_func_ifname = "";
		btn->SetLabel(wxT("偵測"));
		m_cards->Enable(true);
	}
}

void MainFunction::pc_kill()
{
	int index = m_maclist->GetSelection();
	if(index == wxNOT_FOUND)
	{
		::wxMessageBox(wxT("Select a target"));
		return;
	}

	if(count(m_dstmac.begin(), m_dstmac.end(), (char)0) == 6)
	{
		::wxMessageBox(wxT("無法取得ISP MAC"));
		return;
	}

	wxString liststr = m_maclist->GetString(index);
	if(liststr.find(wxT("我的電腦")) != string::npos)
	{
		::wxMessageBox(wxT("多想一分鐘!你可以不必自殺!"));
		return;
	}

	wxString wxstr;
	wxstr.assign(liststr, 0, 17);
	string macstr = wxstr.To8BitData();

	VITE ite = m_victims.find(macstr);
	if(ite == m_victims.end())
	{
		::wxMessageBox(wxT("Critical error, restart the program"));
		return;
	}

	if(ite->second->isKillerAlive())
	{
		ite->second->stopKiller();
		m_kill_btn->SetLabel(_T("殺掉"));
		m_autokill_btn->Enable();
	}
	else
	{
		boost::shared_ptr<Killer> killer = boost::shared_ptr<Killer>(
								new ManualKiller(ite->second->getMac(), m_dstmac,
									ite->second->getInterfaceName(), m_packet_interval));
		ite->second->setKiller(killer);
		ite->second->startKiller();
		m_kill_btn->SetLabel(_T("中止"));
		m_autokill_btn->Disable();
	}
}

void MainFunction::pc_autokill(wxToggleButton *btn)
{
	int index = m_maclist->GetSelection();
	if(index == wxNOT_FOUND)
	{
		::wxMessageBox(wxT("Select a target"));
		btn->SetValue(false);
		return;
	}

	if(count(m_dstmac.begin(), m_dstmac.end(), (char)0) == 6)
	{
		::wxMessageBox(wxT("無法取得ISP MAC"));
		btn->SetValue(false);
		return;
	}

	wxString liststr = m_maclist->GetString(index);
	if(liststr.find(wxT("我的電腦")) != string::npos)
	{
		::wxMessageBox(wxT("多想一分鐘!你可以不必自殺!"));
		btn->SetValue(false);
		return;
	}

	wxString wxstr;
	wxstr.assign(liststr, 0, 17);
	string macstr = wxstr.To8BitData();
	VITE ite = m_victims.find(macstr);
	if(ite == m_victims.end())
	{
		::wxMessageBox(wxT("Critical error, restart the program"));
		return;
	}

	if(btn->GetValue() == true)
	{

		boost::shared_ptr<AutoKiller> killer = boost::shared_ptr<AutoKiller>(
											new AutoKiller(ite->second->getMac(),
														m_dstmac,
														ite->second->getInterfaceName(),
														m_packet_interval));
		killer->AddReactor(boost::bind(&MainFunction::padi_srcmac_detected, this, _1));
		ite->second->setKiller(killer);
		ite->second->startKiller();
		m_kill_btn->Disable();
	}
	else
	{
		ite->second->stopKiller();
		m_kill_btn->Enable();
	}
}

void MainFunction::pc_mark()
{
	int index = m_maclist->GetSelection();
	if(index == wxNOT_FOUND)
	{
		::wxMessageBox(wxT("Select a target"));
		return;
	}

	wxString liststr = m_maclist->GetString(index);
	wxString wxstr;
	wxstr.assign(liststr, 0, 17);
	string macstr = wxstr.To8BitData();

	VITE ite = m_victims.find(macstr);
	if(ite == m_victims.end())
	{
		::wxMessageBox(wxT("Critical error, restart the program"));
		return;
	}

	string desc = ite->second->getDesc();

	wxString input = ::wxGetTextFromUser(_T("輸入想要標記的描述"), _T("輸入"), desc);
	ite->second->setDesc(string(input.c_str()));

	for(unsigned int i = 0; i < m_maclist->GetCount(); i++)
	{
		if(m_maclist->GetString(i).find(wxString(macstr.c_str())) != string::npos)
		{
			m_maclist->SetString(i, this->getliststr(*ite->second));
			break;
		}
	}
}

void MainFunction::pc_list_selected()
{
	boost::try_mutex::scoped_try_lock *lock;

	try
	{
		lock = new boost::try_mutex::scoped_try_lock(m_listsel_mutex);
	}
	catch(const boost::lock_error & e)
	{
		return;
	}
	
	int index = m_maclist->GetSelection();
	if(index == wxNOT_FOUND)
	{
		delete lock;
		return;
	}

	wxString liststr = m_maclist->GetString(index);
	if(liststr.find(wxT("我的電腦")) != string::npos)
	{
		m_kill_btn->SetLabel(_T("殺掉"));
		m_autokill_btn->SetValue(false);
	}
	else	
	{
		//::wxMessageBox(liststr);
		wxString wxstr;
		wxstr.assign(liststr, 0, 17);
		string macstr = wxstr.To8BitData();
		
		VITE ite = m_victims.find(macstr);

		if(ite == m_victims.end())
		{
			::wxMessageBox(wxT("Critical error, restart the program"));
			return;
		}

		
		if(ite->second->isKillerAlive())
		{
			if(ite->second->getKillerID() == ManualKiller::KILLER_ID)
			{
				m_kill_btn->SetLabel(_T("中止"));
				m_kill_btn->Enable();
				m_autokill_btn->SetValue(false);
				m_autokill_btn->Disable();
			}
			else
			{
				m_kill_btn->SetLabel(_T("殺掉"));
				m_kill_btn->Disable();
				m_autokill_btn->SetValue(true);
				m_autokill_btn->Enable();
			}
		}
		else
		{
			m_kill_btn->SetLabel(_T("殺掉"));
			m_kill_btn->Enable();
			m_autokill_btn->SetValue(false);
			m_autokill_btn->Enable();
		}
	}

	delete lock;
}

template<class Archive>
void MainFunction::save(Archive & ar, const unsigned int version) const
{
	for(int i = 0; i < 6; i++)
		ar & m_dstmac[i];
	ar & m_victims;
	ar & m_packet_interval;
}

template<class Archive>
void MainFunction::load(Archive & ar, const unsigned int version)
{
	/**
	* Version 0 config
	*
	* 1. The MAC of ISP
	* 2. The information of target victims
	*/
	for(int i = 0; i < 6; i++)
		ar & m_dstmac[i];
	ar & m_victims;

	/**
	* Version 1 config
	* 
	* 1. The transmitting interval of PADT packets
	*
	*/
	if(version > 0)
		ar & m_packet_interval;
}

template< class T >
inline T const& as_const( T const& r )
{
    return r;
}

void MainFunction::pc_save()
{
	ofstream ofs("pppoe.sav");
	
	try
	{
		boost::archive::text_oarchive oa(ofs);
		oa << as_const(*this);
	}
	catch(const boost::archive::archive_exception & e)
	{
		::wxMessageBox(wxT(e.what()));
	}
}

bool MainFunction::pc_load()
{
	ifstream ifs("pppoe.sav", ios::binary);

	if(ifs.good() == false)
	{
		return false;
	}

	clear_data();
	m_maclist->Clear();

	boost::archive::text_iarchive ia(ifs);

	try
	{
		ia >> *this;
	}
	catch(const boost::archive::archive_exception & e)
	{
		::wxMessageBox(wxT(e.what()));
		return false;
	}

	boost::mutex::scoped_lock lock(m_mutex);

	if(count(m_dstmac.begin(), m_dstmac.end(), (char)0) != 6)
	{
		string s = getMACString(m_dstmac);
		m_ispmac->SetLabel(s);
	}

	VITE iter = m_victims.begin();
	while(iter != m_victims.end())
	{
		m_maclist->Append(this->getliststr(*iter->second));
		iter++;
	}

	return true;
}

void MainFunction::pc_entermac()
{
	wxString s = ::wxGetTextFromUser(wxT("請依照格式輸入MAC ex. 00:AA:BB:CC:DD:EE"));

	if(s.IsEmpty())
		return;

	boost::array<unsigned char, 6> mac;

	try
	{
		mac = parseMAC(string(s.To8BitData()));
	}
	catch(...)
	{
		::wxMessageBox(wxT("輸入錯誤"), wxT("Error"), wxOK|wxICON_ERROR);
		return;
	}
	

	boost::mutex::scoped_lock lock(m_mutex);
	string macstr = getMACString(mac);
	VITE ite = m_victims.find(macstr);
	if(ite == m_victims.end())
	{
		append_data(mac, m_func_ifname);
		m_maclist->Append(this->getliststr(m_victims[macstr]));
	}
	else
		::wxMessageBox(wxT("MAC已存在"), wxT("Error"), wxOK|wxICON_ERROR);
}

void MainFunction::pc_enterispmac()
{
	wxString s = ::wxGetTextFromUser(wxT("請依照格式輸入MAC ex. 00:AA:BB:CC:DD:EE\n(一般來說不用手動輸入ISP MAC, 最簡單的方法是按下偵測,\n然後將自己的PPPoE連線重連就可以偵測到ISP MAC了)"));
	if(s.IsEmpty())
		return;

	boost::array<unsigned char, 6> mac;

	try
	{
		mac = parseMAC(string(s.To8BitData()));
	}
	catch(...)
	{
		::wxMessageBox(wxT("輸入錯誤"), wxT("Error"), wxOK|wxICON_ERROR);
		return;
	}

	string macstr = getMACString(mac);
	m_ispmac->SetLabel(macstr);
	m_dstmac = mac;
}

bool MainFunction::ProcessEvent(wxEvent& e)
{
	if(e.GetId() == PKID_DETECT && e.GetEventType() == wxEVT_COMMAND_TOGGLEBUTTON_CLICKED)
	{
		wxToggleButton *btn = (wxToggleButton*)e.GetEventObject();
		pc_padi_detect(btn);
	}
	else if(e.GetId() == PKID_KILL && e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED)
	{
		pc_kill();
	}
	else if(e.GetId() == PKID_AUTOKILL && e.GetEventType() == wxEVT_COMMAND_TOGGLEBUTTON_CLICKED)
	{
		wxToggleButton *btn = (wxToggleButton*)e.GetEventObject();
		::wxBeginBusyCursor();
		pc_autokill(btn);
		::wxEndBusyCursor();
	}
	else if(e.GetId() == PKID_MARK && e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED)
	{
		pc_mark();
	}
	else if(e.GetId() == PKID_MACLIST && e.GetEventType() == wxEVT_COMMAND_LISTBOX_SELECTED)
		pc_list_selected();
	else if(e.GetId() == PKID_SAVE && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_save();
	else if(e.GetId() == PKID_LOAD && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
	{
		if(pc_load() == false)
			::wxMessageBox(_T("無法讀取檔案"));
	}
	else if(e.GetId() == PKID_ENTERMAC && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_entermac();
	else if(e.GetId() == PKID_ENTERISPMAC && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_enterispmac();
	
	e.Skip();
	return false;
}
