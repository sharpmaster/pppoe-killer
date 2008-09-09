cmake_minimum_required(VERSION 2.6)

PROJECT (pppoe_killer)
SET (CMAKE_BUILD_TYPE Release)

# Boost C++ Libraries v1.34.1 or later
SET (Boost_LIB_SUFFIX ${BOOST_COMPILER}-mt-${BOOST_VERSION})
SET (Boost_FIND_COMPONENTS program_options filesystem serialization system thread regex)
INCLUDE (cmake/FindBoost.cmake)

# Libnet
FIND_PATH (LIBNET_INCLUDE NAMES libnet.h)
IF (LIBNET_INCLUDE) 
	MESSAGE (STATUS "Libnet include path is found - ${LIBNET_INCLUDE}")
ENDIF (LIBNET_INCLUDE)

FIND_LIBRARY (LIBNET_LIB NAMES libnet)
IF (LIBNET_LIB)
	MESSAGE (STATUS "Libnet library path is found - ${LIBNET_LIB}")
ENDIF (LIBNET_LIB)

# Pcap
FIND_PATH (PCAP_INCLUDE NAMES pcap.h)
IF (PCAP_INCLUDE) 
	MESSAGE (STATUS "Pcap include path is found - ${PCAP_INCLUDE}")
ENDIF (PCAP_INCLUDE)

FIND_LIBRARY (PCAP_LIB NAMES pcap wpcap)
IF (PCAP_LIB)
	MESSAGE (STATUS "Pcap library path is found - ${PCAP_LIB}")
ENDIF (PCAP_LIB)
IF (WIN32)
	FIND_LIBRARY (PCAP_PACKET_LIB NAMES packet)
	IF (PCAP_PACKET_LIB)
		MESSAGE (STATUS "WinPcap packet library path is found - ${PCAP_PACKET_LIB}")
	ENDIF (PCAP_PACKET_LIB)
ENDIF (WIN32)

# wxWidgets v2.8
FIND_PATH (WX_INCLUDE NAMES wx/wx.h)
IF (WX_INCLUDE) 
	MESSAGE (STATUS "wxWidgets include path is found - ${WX_INCLUDE}")
ENDIF (WX_INCLUDE)

FIND_LIBRARY (WX_BASE_LIB NAMES wxbase28)
IF (WX_BASE_LIB)
	MESSAGE (STATUS "wxBase library path is found - ${WX_BASE_LIB}")
ENDIF (WX_BASE_LIB)

IF(WIN32)
	FIND_LIBRARY (WX_MSWCORE_LIB NAMES wxmsw28_core)
	IF (WX_MSWCORE_LIB)
		MESSAGE (STATUS "wxMsw library path is found - ${WX_MSWCORE_LIB}")
	ENDIF (WX_MSWCORE_LIB)

	FIND_LIBRARY (WX_MSWADV_LIB NAMES wxmsw28_adv)
	IF (WX_MSWADV_LIB)
		MESSAGE (STATUS "wxMswAdv library path is found - ${WX_MSWADV_LIB}")
	ENDIF (WX_MSWADV_LIB)
ENDIF(WIN32)


INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS} ${LIBNET_INCLUDE} ${PCAP_INCLUDE} 
${WX_INCLUDE} lib/hippolib)
ADD_DEFINITIONS (-DNDEBUG -DHAVE_REMOTE)
FILE (GLOB SOURCES src/*.cpp)

IF (WIN32)
	ADD_EXECUTABLE (pkiller WIN32 ${SOURCES})
ELSE (WIN32)
	ADD_EXECUTABLE (pkiller ${SOURCES})
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(pkiller ${LIBNET_LIB} ${PCAP_LIB} ${WX_BASE_LIB})
IF (WIN32)
	TARGET_LINK_LIBRARIES(pkiller
				${PCAP_PACKET_LIB}
				comctl32
				${WX_MSWCORE_LIB} ${WX_MSWADV_LIB})
ENDIF (WIN32)
