#include <glib/GBase64.h>
#include <assert.h>

using namespace glib;
using namespace std;

string GBase64::Encode(const string & src)
{
	string ret;
	size_t bsize;
	char *bdata;

	ret = "";
	
	if(src.size() == 0)
		return "";

	bsize = b64_encode(NULL, src.size(), NULL, 0);
	bdata = new char[bsize];
	
	if(b64_encode(src.data(), src.size(), bdata, bsize) == 0)
	{
		delete bdata;
		throw bad_alloc();
	}
	
	ret.assign(bdata, bsize);
	delete bdata;
	return ret;
}

string GBase64::Decode(const string & src)
{
	string ret;
	size_t bsize;
	char *bdata;

	if(src.size() == 0)
		return "";

	bsize = b64_decode(NULL, src.size(), NULL, 0);
	bdata = new char[bsize];
	
	if(b64_decode(src.data(), src.size(), bdata, bsize) == 0)
	{
		delete bdata;
		throw bad_alloc();
	}
	
	ret.assign(bdata, bsize);
	delete bdata;
	return ret;
}

enum B64_FLAGS
{
        B64_F_LINE_LEN_USE_PARAM    =   0x0000  /*!< Uses the lineLen parameter to b64_encode2(). Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_INFINITE     =   0x0001  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is infinite. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_64           =   0x0002  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is 64. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_76           =   0x0003  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is 76. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_MASK         =   0x000f  /*!< Mask for testing line length flags to b64_encode2(). Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_NOTHING       =   0x0000  /*!< Decoding ignores all invalid characters in the input data. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_UNKNOWN_CHAR  =   0x0100  /*!< Causes decoding to break if any non-Base-64 [a-zA-Z0-9=+/], non-whitespace character is encountered. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_UNEXPECTED_WS =   0x0200  /*!< Causes decoding to break if any unexpected whitespace is encountered. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_BAD_CHAR      =   0x0300  /*!< Causes decoding to break if any non-Base-64 [a-zA-Z0-9=+/] character is encountered. Ignored by b64_encode2(). */
};

# define NUM_PLAIN_DATA_BYTES        (3)
# define NUM_ENCODED_DATA_BYTES      (4)
static const char  b64_chars[] =   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const signed char b64_indexes[]   =   
{
    /* 0 - 31 / 0x00 - 0x1f */
        -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1
    /* 32 - 63 / 0x20 - 0x3f */
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, 62, -1, -1, -1, 63  /* ... , '+', ... '/' */
    ,   52, 53, 54, 55, 56, 57, 58, 59  /* '0' - '7' */
    ,   60, 61, -1, -1, -1, -1, -1, -1  /* '8', '9', ... */
    /* 64 - 95 / 0x40 - 0x5f */
    ,   -1, 0,  1,  2,  3,  4,  5,  6   /* ..., 'A' - 'G' */
    ,   7,  8,  9,  10, 11, 12, 13, 14  /* 'H' - 'O' */
    ,   15, 16, 17, 18, 19, 20, 21, 22  /* 'P' - 'W' */
    ,   23, 24, 25, -1, -1, -1, -1, -1  /* 'X', 'Y', 'Z', ... */
    /* 96 - 127 / 0x60 - 0x7f */
    ,   -1, 26, 27, 28, 29, 30, 31, 32  /* ..., 'a' - 'g' */
    ,   33, 34, 35, 36, 37, 38, 39, 40  /* 'h' - 'o' */
    ,   41, 42, 43, 44, 45, 46, 47, 48  /* 'p' - 'w' */
    ,   49, 50, 51, -1, -1, -1, -1, -1  /* 'x', 'y', 'z', ... */

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
};

size_t GBase64::b64_encode(void const *src, size_t srcSize, char *dest, size_t destLen)
{
    return b64_encode_((unsigned char const*)src, srcSize, dest, destLen, 0);
}

size_t GBase64::b64_encode_(  unsigned char const *src
                        ,   size_t              srcSize
                        ,   char *const         dest
                        ,   size_t              destLen
                        ,   unsigned            lineLen)
{
    size_t  total   =   ((srcSize + (NUM_PLAIN_DATA_BYTES - 1)) / NUM_PLAIN_DATA_BYTES) * NUM_ENCODED_DATA_BYTES;

    if(lineLen > 0)
    {
        unsigned    numLines    =   (total + (lineLen - 1)) / lineLen;
        total += 2 * (numLines - 1);
    }

    if(NULL == dest)
        return total;
    else if(destLen < total)
        return 0;
    else
    {
        char    *p      =   dest;
        char    *end    =   dest + destLen;
        size_t  len     =   0;

        for(; NUM_PLAIN_DATA_BYTES <= srcSize; srcSize -= NUM_PLAIN_DATA_BYTES)
        {
            char    characters[NUM_ENCODED_DATA_BYTES];

            /* 
             * 
             * |       0       |       1       |       2       |
             *
             * |               |               |               |
             * |       |       |       |       |       |       |
             * |   |   |   |   |   |   |   |   |   |   |   |   |
             * | | | | | | | | | | | | | | | | | | | | | | | | |
             * 
             * |     0     |     1     |     2     |     3     |
             * 
             */

            /* characters[0] is the 6 left-most bits of src[0] */
            characters[0] = (char)((src[0] & 0xfc) >> 2);
            /* characters[0] is the right-most 2 bits of src[0] and the left-most 4 bits of src[1] */
            characters[1] = (char)(((src[0] & 0x03) << 4) + ((src[1] & 0xf0) >> 4));
            /* characters[0] is the right-most 4 bits of src[1] and the 2 left-most bits of src[2] */
            characters[2] = (char)(((src[1] & 0x0f) << 2) + ((src[2] & 0xc0) >> 6));
            /* characters[3] is the right-most 6 bits of src[2] */
            characters[3] = (char)(src[2] & 0x3f);

#ifndef __WATCOMC__
            assert(characters[0] >= 0 && characters[0] < 64);
            assert(characters[1] >= 0 && characters[1] < 64);
            assert(characters[2] >= 0 && characters[2] < 64);
            assert(characters[3] >= 0 && characters[3] < 64);
#endif /* __WATCOMC__ */

            src += NUM_PLAIN_DATA_BYTES;
            *p++ = b64_chars[(unsigned char)characters[0]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[1]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[2]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[3]];
            assert(NULL != strchr(b64_chars, *(p-1)));

            if( ++len == lineLen &&
                p != end)
            {
                *p++ = '\r';
                *p++ = '\n';
                len = 0;
            }
        }

        if(0 != srcSize)
        {
            /* Deal with the overspill, by boosting it up to three bytes (using 0s)
             * and then appending '=' for any missing characters.
             *
             * This is done into a temporary buffer, so we can call ourselves and
             * have the output continue to be written direct to the destination.
             */

            unsigned char   dummy[NUM_PLAIN_DATA_BYTES];
            size_t          i;

            for(i = 0; i < srcSize; ++i)
            {
                dummy[i] = *src++;
            }

            for(; i < NUM_PLAIN_DATA_BYTES; ++i)
            {
                dummy[i] = '\0';
            }

            b64_encode_(&dummy[0], NUM_PLAIN_DATA_BYTES, p, NUM_ENCODED_DATA_BYTES * (1 + 2), 0);

            for(p += 1 + srcSize; srcSize++ < NUM_PLAIN_DATA_BYTES; )
            {
                *p++ = '=';
            }
        }

        return total;
    }
}

size_t GBase64::b64_decode(char const *src, size_t srcLen, void *dest, size_t destSize)
{
    char const  *badChar_;
    return b64_decode_(src, srcLen, (unsigned char*)dest, destSize, B64_F_STOP_ON_NOTHING, &badChar_);
}

size_t GBase64::b64_decode_(  char const      *src
                        ,   size_t          srcLen
                        ,   unsigned char   *dest
                        ,   size_t          destSize
                        ,   unsigned        flags
                        ,   char const      **badChar)
{
    const size_t    wholeChunks     =   (srcLen / NUM_ENCODED_DATA_BYTES);
    const size_t    remainderBytes  =   (srcLen % NUM_ENCODED_DATA_BYTES);
    size_t          maxTotal        =   (wholeChunks + (0 != remainderBytes)) * NUM_PLAIN_DATA_BYTES;
    unsigned char   *dest_          =   dest;

    ((void)remainderBytes);

    assert(NULL != badChar);

    *badChar    =   NULL;

    if(NULL == dest)
    {
        return maxTotal;
    }
    else if(destSize < maxTotal)
    {
        return 0;
    }
    else
    {
        /* Now we iterate through the src, collecting together four characters
         * at a time from the Base-64 alphabet, until the end-point is reached.
         *
         * 
         */

        char const          *begin      =   src;
        char const  *const  end         =   begin + srcLen;
        size_t              currIndex   =   0;
        size_t              numPads     =   0;
        signed char         indexes[NUM_ENCODED_DATA_BYTES];    /* 4 */

        for(; begin != end; ++begin)
        {
            const char  ch  =   *begin;

            if('=' == ch)
            {
                assert(currIndex < NUM_ENCODED_DATA_BYTES);

                indexes[currIndex++] = '\0';

                ++numPads;
            }
            else
            {
                signed char index   =   b64_indexes[(unsigned char)ch];

                if(-1 == index)
                {
                    switch(ch)
                    {
                        case    ' ':
                        case    '\t':
                        case    '\b':
                        case    '\v':
                            if(B64_F_STOP_ON_UNEXPECTED_WS & flags)
                            {
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                /* Fall through */
                            }
                        case    '\r':
                        case    '\n':
                            continue;
                        default:
                            if(B64_F_STOP_ON_UNKNOWN_CHAR & flags)
                            {
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                continue;
                            }
                    }
                }
                else
                {
                    numPads = 0;

                    assert(currIndex < NUM_ENCODED_DATA_BYTES);

                    indexes[currIndex++] = index;
                }
            }

            if(NUM_ENCODED_DATA_BYTES == currIndex)
            {
                unsigned char   bytes[NUM_PLAIN_DATA_BYTES];        /* 3 */

                bytes[0] = (unsigned char)((indexes[0] << 2) + ((indexes[1] & 0x30) >> 4));

                currIndex = 0;

                *dest++ = bytes[0];
                if(2 != numPads)
                {
                    bytes[1] = (unsigned char)(((indexes[1] & 0xf) << 4) + ((indexes[2] & 0x3c) >> 2));

                    *dest++ = bytes[1];

                    if(1 != numPads)
                    {
                        bytes[2] = (unsigned char)(((indexes[2] & 0x3) << 6) + indexes[3]);

                        *dest++ = bytes[2];
                    }
                }
                if(0 != numPads)
                {
                    break;
                }
            }
        }

        return (size_t)(dest - dest_);
    }
}

