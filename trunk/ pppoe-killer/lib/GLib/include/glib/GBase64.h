#ifndef _GBASE64_H
#define _GBASE64_H
#include <glib/config.h>
#include <string>

namespace glib
{
	class GLIB_EXPORT GBase64
	{
	public:
		static std::string Encode(const std::string & src);
		static std::string Decode(const std::string & src);
	private:
		static size_t b64_encode(void const *src, size_t srcSize, char *dest, size_t destLen);
		static size_t b64_encode_(  unsigned char const *src
                        ,   size_t              srcSize
                        ,   char *const         dest
                        ,   size_t              destLen
                        ,   unsigned            lineLen);
		static size_t b64_decode(char const *src, size_t srcLen, void *dest, size_t destSize);
		static size_t b64_decode_(  char const      *src
                        ,   size_t          srcLen
                        ,   unsigned char   *dest
                        ,   size_t          destSize
                        ,   unsigned        flags
                        ,   char const      **badChar);
	};
}

#endif

