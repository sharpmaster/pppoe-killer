#include <glib/net/GNetTool.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
//#include <apr.h>
//#include <apr_general.h>
//#include <apr_xml.h>
#include <fstream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <glib/GProperties.h>
#include <glib/GLogger.h>
#include <glib/GBase64.h>
#include <glib/GTime.h>
#include "Resource.h"
#include "AutoKiller.h"
#include "MainFunction.h"
#include "GPacketDetector.h"
#include "VictimEntry.h"
#include "PADTGenerator.h"


using namespace std;
using namespace glib;
using namespace glib::net;

MainFunction::MainFunction(wxComboBox* cards, wxListBox *list, wxStaticText *ispmac,
						   wxToggleButton *autokill_btn)
{
	m_cards = cards;
	m_maclist = list;
	m_ispmac = ispmac;
	m_autokill_btn = autokill_btn;

	m_padi_dtr = NULL;
	m_pado_dtr = NULL;

	fill(m_dstmac.begin(), m_dstmac.end(), (char)0);

	int index = m_cards->GetSelection();
	if(index == wxNOT_FOUND)
		m_func_ifname = "";
	else
		m_func_ifname = *((string*)(m_cards->GetClientData(index)));
	

	m_logger = GLogger::getLogger("main");
}

MainFunction::~MainFunction()
{
	if(m_padi_dtr != NULL)
	{
		if(m_padi_dtr->isAlive() == true)
			m_padi_dtr->stop();
		while(m_padi_dtr->isAlive() == true)
			GThread::sleep(10);
		delete m_padi_dtr;
		m_padi_dtr = NULL;
	}

	if(m_pado_dtr != NULL)
	{
		if(m_pado_dtr->isAlive() == true)
			m_pado_dtr->stop();
		while(m_pado_dtr->isAlive() == true)
			GThread::sleep(10);
		delete m_pado_dtr;
		m_pado_dtr = NULL;
	}

	clear_data();
	delete m_logger;
}

string MainFunction::getMACString(const boost::array<char, 6> & mac)
{
	stringstream ss;
	string s = GNetTool::getMACString(mac);

	ss << s.substr(0, 2) << ":" << s.substr(2, 2) << ":" <<
		 s.substr(4, 2) << ":" << s.substr(6, 2) << ":" <<
		 s.substr(8, 2) << ":" << s.substr(10, 2);

	return ss.str();
}

boost::array<char, 6> MainFunction::parseMAC(const std::string & macstr)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(":");
	tokenizer tokens(macstr, sep);
	stringstream ss;

	for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
	   	ss << *tok_iter;

	boost::array<char, 6> ret;

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
		ite->second->stopAutoKiller();
		while(ite->second->isAutoKillerAlive())
			GThread::sleep(10);
		ite++;
	}
	m_victims.clear();
}

void MainFunction::append_data(const boost::array<char, 6> & macbin, std::string & ifname)
{
	VictimEntry *v = new VictimEntry();
	string s;

	v->setMac(macbin);
	s = getMACString(macbin);
	v->setInterfaceName(ifname);
	v->setLastSeenDate(GTime::GetTimeString());
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

	return ret;
}

void MainFunction::padi_detected(const unsigned char* packet, int len)
{
	padi_srcmac_detected(packet+6);
}

void MainFunction::padi_srcmac_detected(const unsigned char* srcmac)
{
	string mac;
	boost::array<char, 6> macbin;
	boost::ptr_map<string, VictimEntry>::iterator ite;
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
		
		GDEBUG(*m_logger)("Refreshing time %s", macstr.c_str());
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
	boost::array<char, 6> macbin;
	
	copy(packet+6, packet+6+6, macbin.begin());
	string s = getMACString(macbin);
	m_ispmac->SetLabel(s);
	m_dstmac = macbin;
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
		m_padi_dtr->start();
		GThread::sleep(100);
		m_pado_dtr->start();

		m_func_ifname = *ifname;
		btn->SetLabel(wxT("偵測中"));
		m_cards->Enable(false);
	}
	else
	{
		m_padi_dtr->stop();
		while(m_padi_dtr->isAlive() == true)
			GThread::sleep(10);
		delete m_padi_dtr;
		m_padi_dtr = NULL;

		m_pado_dtr->stop();
		while(m_pado_dtr->isAlive() == true)
			GThread::sleep(10);
		delete m_pado_dtr;
		m_pado_dtr = NULL;

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

	PADTGenerator padt(ite->second->getInterfaceName(), (unsigned char*)m_dstmac.data(), (unsigned char*)ite->second->getMac().data());
	padt.start();
	::wxBeginBusyCursor();
	while(padt.isAlive() == true)
		GThread::sleep(500);
	::wxEndBusyCursor();
	
}

void MainFunction::pc_autokill(wxToggleButton *btn)
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
		if(ite->second->isAutoKillerExisted() == false)
		{
			boost::shared_ptr<AutoKiller> killer = boost::shared_ptr<AutoKiller>(
												new AutoKiller(
												(unsigned char*)ite->second->getMac().data(),
												(unsigned char*)m_dstmac.data(),
												ite->second->getInterfaceName()
												));
			killer->AddReactor(boost::bind(&MainFunction::padi_srcmac_detected, this, _1));
			ite->second->setAutoKiller(killer);
		}
		ite->second->startAutoKiller();
	}
	else
	{
		ite->second->stopAutoKiller();
		while(ite->second->isAutoKillerAlive() == true)
			GThread::sleep(100);
	}
}

void MainFunction::pc_list_selected()
{
	bool value;
	int index = m_maclist->GetSelection();
	if(index == wxNOT_FOUND)
		return;

	wxString liststr = m_maclist->GetString(index);
	if(liststr.find(wxT("我的電腦")) != string::npos)
		value = false;
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

		
		if(ite->second->isAutoKillerAlive() == true)
			value = true;
		else
			value = false;

	}

	m_autokill_btn->SetValue(value);
}


template<class Archive>
void MainFunction::serialize(Archive & ar, const unsigned int version)
{
	for(int i = 0; i < 6; i++)
		ar & m_dstmac[i];
	ar & m_victims;
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

void MainFunction::pc_load()
{
	ifstream ifs("pppoe.sav", ios::binary);

	if(ifs.good() == false)
	{
		::wxMessageBox(wxT("無法開啟檔案"));
		return;
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
		return;
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
}

/*
void MainFunction::pc_save()
{
	
	VITE ite = m_victims.begin();
	if(ite == m_victims.end())
	{
		::wxMessageBox(wxT("無資料"));
		return;
	}

	std::fstream fs;
	fs.open("pppoe.sav", std::ios_base::out | std::ios_base::trunc);

	fs << "<?xml version=\"1.0\" ?>" << endl << "<root dstmac=\"";

	if(count(m_dstmac.begin(), m_dstmac.end(), (char)0) != 6)
	{
		string s;
		s.assign(m_dstmac.begin(), m_dstmac.end());
		s = GBase64::Encode(s);
		fs << s.c_str();
	}
	fs << "\">" << endl;
	
	while(ite != m_victims.end())
	{
		string str = ite->second->GetXMLElementStr();
		fs << str.c_str() << endl;
		ite++;
	}

	fs << "</root>" << endl;
	fs.close();
	
}

void MainFunction::pc_load()
{
	apr_pool_t *pool;
	apr_file_t *fd;
	apr_xml_parser *parser;
	apr_xml_doc *doc;
	apr_xml_elem* elem;
	apr_status_t rv;

	boost::ptr_map<std::string, VictimEntry> tmp_victims;
	string tmp_dstmac;
	boost::mutex::scoped_lock *lock;

	(void) apr_initialize();
	rv = apr_pool_create(&pool, NULL);
	if(rv != APR_SUCCESS)
	{
		::wxMessageBox(wxT("Memory pool error"));
		return;
	}
	
	rv = apr_file_open(&fd, "pppoe.sav", APR_READ, APR_OS_DEFAULT, pool);
	if (rv != APR_SUCCESS)
	{
		::wxMessageBox(wxT("Cannot load"));
		apr_pool_destroy(pool);
		return;
	}

	rv = apr_xml_parse_file(pool, &parser, &doc, fd, 2000);
	if (rv != APR_SUCCESS)
	{
		apr_file_close(fd);
		apr_pool_destroy(pool);

		//load_old_savefile();
		return;
	}

	if(doc->root == NULL)
	{
		::wxMessageBox(wxT("檔案內容錯誤"));
		apr_file_close(fd);
		apr_pool_destroy(pool);
		return;
	}

	elem = doc->root->first_child;
	while(elem)
	{
		VictimEntry *v = new VictimEntry();
		if(v->SetXMLElement(elem))
		{
			string s = getMACString(v->getMac());
			tmp_victims.insert(s, v);
		}
		else
			delete v;

		elem = elem->next;
	}
	string tmpmac_str = aprXmlGetPropString(doc->root, "dstmac", "");
	if(tmpmac_str != "")
		tmp_dstmac= GBase64::Decode(tmpmac_str);


	if(tmp_victims.size() == 0)
	{
		::wxMessageBox(wxT("檔案內容錯誤, 沒有資料"));
		apr_file_close(fd);
		apr_pool_destroy(pool);
		return;
	}

	lock = new boost::mutex::scoped_lock(m_mutex);

	clear_data();
	m_maclist->Clear();

	if(tmpmac_str != "")
	{
		copy(tmp_dstmac.begin(), tmp_dstmac.end(), m_dstmac.begin());
		string s = getMACString(m_dstmac);
		m_ispmac->SetLabel(wxString::From8BitData(s.c_str()));
	}
	m_victims.transfer(tmp_victims);
	VITE ite = m_victims.begin();
	while(ite != m_victims.end())
	{
		m_maclist->Append(wxString::From8BitData(this->getliststr(*ite->second).c_str()));
		ite++;
	}

	delete lock;
	
	apr_file_close(fd);
	apr_pool_destroy(pool);
	apr_terminate();
}
*/

void MainFunction::pc_entermac()
{
	wxString s = ::wxGetTextFromUser(wxT("請依照格式輸入MAC ex. 00:AA:BB:CC:DD:EE"));

	if(s.IsEmpty())
		return;

	boost::array<char, 6> mac;

	try
	{
		mac = parseMAC(string(s.To8BitData()));
	}
	catch(...)
	{
		::wxMessageBox(wxT("輸入錯誤"), wxT("Error"), wxOK|wxICON_ERROR);
		return;
	}
	

	boost::mutex::scoped_lock *lock = new boost::mutex::scoped_lock(m_mutex);
	string macstr = getMACString(mac);
	boost::ptr_map<string, VictimEntry>::iterator ite = m_victims.find(macstr);
	if(ite == m_victims.end())
	{
		append_data(mac, m_func_ifname);
		m_maclist->Append(this->getliststr(m_victims[macstr]));
	}
	else
		::wxMessageBox(wxT("MAC已存在"), wxT("Error"), wxOK|wxICON_ERROR);
	
	delete lock;
}

/*
void MainFunction::load_old_savefile()
{
	GProperties prop;
	char buf[16];
	string::size_type oldidx, newidx;
	int index = 0;
	fstream fs;

	boost::ptr_map<std::string, VictimEntry> tmp_victims;
	boost::mutex::scoped_lock *lock;

	fs.open("pppoe.sav", ios_base::in);
	prop.load(fs);
	fs.close();

	GINFO(*m_logger)("Loading start");

	string value = prop.get("dstmac");
	string result = "";
	oldidx = 0;
	unsigned char *tmp_dstmac = new unsigned char[6];
	while(true)
	{
		newidx = value.find_first_of(',', oldidx);
		if(newidx == string::npos)
			break;

		string intv = value.substr(oldidx, newidx);
		*(tmp_dstmac+index++) = atoi(intv.c_str());
		sprintf(buf, "%02X:", atoi(intv.c_str()));
		intv = buf;
		result += intv;
	
		oldidx = newidx+1;
		if(oldidx >= value.size())
			break;
	}
	result = result.substr(0, result.size()-1);
	GINFO(*m_logger)("Loading... dstmac %s loaded", result.c_str());
	m_ispmac->SetLabel(result);

	value = prop.get("srcmac_size");
	int size = atoi(value.c_str());
	GINFO(*m_logger)("Loading... srcmac size: %d", size);
	for(int i = 0; i < size; i++)
	{
		sprintf(buf, "srcmac%d", i);
		string key = buf;
		value = prop.get(key);

		oldidx = 0;
		result = "";
		index = 0;
		unsigned char *mac = new unsigned char[6];

		while(true)
		{
			newidx = value.find_first_of(',', oldidx);
			if(newidx == string::npos)
				break;
			
			string intv = value.substr(oldidx, newidx);
			*(mac+index++) = atoi(intv.c_str());

			sprintf(buf, "%02X:", atoi(intv.c_str()));
			intv = buf;
			result += intv;
		
			oldidx = newidx+1;
			if(oldidx >= value.size())
				break;
		}

		result = result.substr(0, result.size()-1);

		VictimEntry *v = new VictimEntry();
		memcpy(v->m_mac.get(), mac, 6);
		
		sprintf(buf, "srcmac%difname", i);
		key = buf;
		v->m_ifname = prop.get(key);

		tmp_victims.insert(NetworkTool::GetMACString(v->m_mac.get()), v);

		delete mac;
	}
	
	GINFO(*m_logger)("Loading end");

	if(tmp_victims.size() == 0)
	{
		::wxMessageBox("檔案內容錯誤, 沒有資料");
		return;
	}


	lock = new boost::mutex::scoped_lock(m_mutex);
	clear_data();
	m_maclist->Clear();

	m_dstmac = new unsigned char[6];
	memcpy(m_dstmac, tmp_dstmac, 6);
	delete tmp_dstmac;
	m_victims.transfer(tmp_victims);
	VITE ite = m_victims.begin();
	while(ite != m_victims.end())
	{
		m_maclist->Append(this->getliststr(*ite->second));
		ite++;
	}

	delete lock;
}
*/

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
	else if(e.GetId() == PKID_MACLIST && e.GetEventType() == wxEVT_COMMAND_LISTBOX_SELECTED)
		pc_list_selected();
	else if(e.GetId() == PKID_SAVE && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_save();
	else if(e.GetId() == PKID_LOAD && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_load();
	else if(e.GetId() == PKID_ENTERMAC && e.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)
		pc_entermac();
	
	e.Skip();
	return false;
}
