
#ifndef __UNICODE_API_H
#define __UNICODE_API_H


typedef unsigned short      WCHAR;

/*#define WC_DISCARDNS         0x0010
#define WC_SEPCHARS          0x0020
#define WC_DEFAULTCHAR       0x0040
#define WC_COMPOSITECHECK    0x0200
#define WC_NO_BEST_FIT_CHARS 0x0400

#define MB_PRECOMPOSED              0x01
#define MB_COMPOSITE                0x02
#define MB_USEGLYPHCHARS            0x04
#define MB_ERR_INVALID_CHARS        0x08*/

/* code page info common to SBCS and DBCS */
struct cp_info
{
    unsigned int          codepage;          /* codepage id */
    unsigned int          char_size;         /* char size (1 or 2 bytes) */
    WCHAR                 def_char;          /* default char value (can be double-byte) */
    WCHAR                 def_unicode_char;  /* default Unicode char value */
    const char           *name;              /* code page name */
};

struct sbcs_table
{
    struct cp_info        info;
    const WCHAR          *cp2uni;            /* code page -> Unicode map */
    const unsigned char  *uni2cp_low;        /* Unicode -> code page map */
    const unsigned short *uni2cp_high;
};

struct dbcs_table
{
    struct cp_info        info;
    const WCHAR          *cp2uni;            /* code page -> Unicode map */
    const unsigned char  *cp2uni_leadbytes;
    const unsigned short *uni2cp_low;        /* Unicode -> code page map */
    const unsigned short *uni2cp_high;
    unsigned char         lead_bytes[12];    /* lead bytes ranges */
};

union cptable
{
    struct cp_info    info;
    struct sbcs_table sbcs;
    struct dbcs_table dbcs;
};


typedef enum _CODEPAGE_ {
    CP_037 = 0,
    CP_424,
    CP_437,
    CP_500,
    CP_737,
    CP_775,
    CP_850,
    CP_852,
    CP_855,
    CP_856,
    CP_857,
    CP_860,
    CP_861,
    CP_862,
    CP_863,
    CP_864,
    CP_865,
    CP_866,
    CP_869,
    CP_874,
    CP_875,
    CP_878,
    CP_932,
    CP_936,
    CP_949,
    CP_950,
    CP_1006,
    CP_1026,
    CP_1250,
    CP_1251,
    CP_1252,
    CP_1253,
    CP_1254,
    CP_1255,
    CP_1256,
    CP_1257,
    CP_1258,
    CP_10000,
    CP_10006,
    CP_10007,
    CP_10029,
    CP_10079,
    CP_10081,
    CP_20127,
    CP_20866,
    CP_20932,
    CP_21866,
    CP_28591,
    CP_28592,
    CP_28593,
    CP_28594,
    CP_28595,
    CP_28596,
    CP_28597,
    CP_28598,//
    CP_28599,//
    CP_28600,//
    CP_28603,//CP_28603
    CP_28604,//CP_28604
    CP_28605,//CP_28605
    CP_28606,//CP_28606
    CODE_PAGE_NUM
} T_CODE_PAGE;

const union cptable *cp_get_table( unsigned int codepage );//cp_get_table

int cp_wcstombs( const union cptable *table, int flags,
                      const WCHAR *src, int srclen,
                      char *dst, int dstlen, const char *defchar, int *used );//cp_wcstombs
int cp_mbstowcs( const union cptable *table, int flags,
                      const char *s, int srclen,
                      WCHAR *dst, int dstlen , const unsigned short *defchar);

int utf8_mbstowcs( int flags, const char *src, int srclen, WCHAR *dst, int dstlen );//utf8_mbstowcs
int utf8_wcstombs( const WCHAR *src, int srclen, char *dst, int dstlen );//utf8_wcstombs


#endif
