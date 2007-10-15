#ifndef _GLIB_CONFIG_MSVC_H
#define _GLIB_CONFIG_MSVC_H

#ifdef WIN32
/**
* Some Boost libraries have special characters in their comments
* That will raise C4819 warnings, so we turn it off here
*/
#pragma warning(disable : 4250 4251 4786 4290 4996 4819)
#endif

#define _WIN32_WINNT 0x0500
#define _CRT_SECURE_NO_DEPRECATE

#ifdef GLIB_STATIC
#define GLIB_EXPORT
#else // DLL
#ifdef GLIB
	#define GLIB_EXPORT __declspec(dllexport)
#else
	#define GLIB_EXPORT __declspec(dllimport)
#endif
#endif

#ifndef GLIB_AUTO_LINK_NOMANGLE
	#if defined(_MT) || defined(__MT__)
		#pragma comment(lib, "libglib.lib")
	#else
		#pragma comment(lib, "glib.lib")
	#endif
#endif

#endif

