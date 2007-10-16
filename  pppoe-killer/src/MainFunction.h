#ifndef _MAINFUNCTION_H
#define _MAINFUNCTION_H

#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <glib/GLogger.h>

class GPacketDetector;
class AutoKiller;
class VictimEntry;
class boost::mutex;
class wxGrid;

class MainFunction : public wxEvtHandler
{
public:
	MainFunction(wxComboBox* cards, wxListBox* list, wxStaticText *ispmac, wxToggleButton *autokill_btn);
	~MainFunction();
	bool ProcessEvent(wxEvent& e);

	bool loadConfig() {return pc_load();}
	void saveConfig() {pc_save();}

	unsigned int getPacketInterval() {return m_packet_interval;}
	void setPacketInterval(unsigned int value) {m_packet_interval = value;}
	
private:
	GPacketDetector *m_padi_dtr;
	GPacketDetector *m_pado_dtr;

	wxComboBox *m_cards;
	wxListBox *m_maclist;
	wxStaticText *m_ispmac;
	wxToggleButton *m_autokill_btn;

	boost::mutex m_mutex;
	
	/**
	* The data that will be saved
	*/
	boost::array<char, 6> m_dstmac;
	boost::ptr_map<std::string, VictimEntry> m_victims;
	unsigned int m_packet_interval;
	
	typedef boost::ptr_map<std::string, VictimEntry>::iterator VITE;

	std::string m_func_ifname;

	glib::GBaseLogger *m_logger;

	std::string getMACString(const boost::array<char, 6> & mac);
	boost::array<char, 6> parseMAC(const std::string & macstr);
	void clear_data();
	void append_data(const boost::array<char, 6> & macbin, std::string & ifname);
	//VITE get_iterator(const std::string & macstr);
	std::string getliststr(const VictimEntry & entry);

	//void load_old_savefile();

	void padi_detected(const unsigned char* packet, int len);
	void pado_detected(const unsigned char* packet, int len);
	void padi_srcmac_detected(const unsigned char* srcmac);

	void pc_padi_detect(wxToggleButton *btn);
	void pc_kill();
	void pc_autokill(wxToggleButton *btn);
	void pc_list_selected();
	void pc_save();
	bool pc_load();
	void pc_entermac();
	
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;
	template<class Archive>
	void load(Archive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(MainFunction, 1)
	
#endif
