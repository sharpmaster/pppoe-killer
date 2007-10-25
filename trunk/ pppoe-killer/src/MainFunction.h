#ifndef _MAINFUNCTION_H
#define _MAINFUNCTION_H

#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/thread/mutex.hpp>
#include <log4cxx/logger.h>

class GPacketDetector;
class AutoKiller;
class VictimEntry;
class wxGrid;
class wxToggleButton;
class wxStaticText;
class wxListBox;
class wxComboBox;

class MainFunction : public wxEvtHandler
{
public:
	MainFunction(wxComboBox* cards, wxListBox* list, wxStaticText *ispmac, wxToggleButton *autokill_btn, wxButton *kill_btn);
	~MainFunction();
	bool ProcessEvent(wxEvent& e);

	bool loadConfig() {return pc_load();}
	void saveConfig() {pc_save();}

	unsigned int getPacketInterval() {return m_packet_interval;}
	void setPacketInterval(unsigned int value) {m_packet_interval = value;}
	void refreshButton();
	
private:
	GPacketDetector *m_padi_dtr;
	GPacketDetector *m_pado_dtr;
	GPacketDetector *m_arpreq_dtr;

	wxComboBox *m_cards;
	wxListBox *m_maclist;
	wxStaticText *m_ispmac;
	wxButton *m_kill_btn;
	wxToggleButton *m_autokill_btn;

	boost::mutex m_mutex;
	boost::try_mutex m_listsel_mutex;
	/**
	* The data that will be saved
	*/
	boost::array<unsigned char, 6> m_dstmac;
	boost::ptr_map<std::string, VictimEntry> m_victims;
	unsigned int m_packet_interval;
	
	typedef boost::ptr_map<std::string, VictimEntry>::iterator VITE;

	std::string m_func_ifname;

	log4cxx::LoggerPtr m_logger;

	std::string getMACString(const boost::array<unsigned char, 6> & mac);
	boost::array<unsigned char, 6> parseMAC(const std::string & macstr);
	void clear_data();
	void append_data(const boost::array<unsigned char, 6> & macbin, const std::string & ifname,
				const std::string & desc = "");
	std::string getliststr(const VictimEntry & entry);

	//void load_old_savefile();

	void padi_detected(const unsigned char* packet, int len);
	void pado_detected(const unsigned char* packet, int len);
	void arpreq_detected(const unsigned char* packet, int len);
	void padi_srcmac_detected(const unsigned char* srcmac);

	void pc_padi_detect(wxToggleButton *btn);
	void pc_kill();
	void pc_autokill(wxToggleButton *btn);
	void pc_mark();
	void pc_list_selected();
	void pc_save();
	bool pc_load();
	void pc_entermac();
	void pc_enterispmac();
	
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;
	template<class Archive>
	void load(Archive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(MainFunction, 1)
	
#endif
