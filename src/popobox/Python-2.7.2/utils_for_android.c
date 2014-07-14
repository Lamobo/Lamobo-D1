#include "locale_for_android.h"
#include <grp.h>
#include <netdb.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// for _socket.so

int gethostbyaddr_r(const void * param1, int param2, int param3, 
	struct hostent * param4, char * param5, size_t param6, 
	struct hostent ** param7, int * param8)
{
    printf("<%s,%d>enter myutils.gethostbyaddr_r\n", __FILE__, __LINE__);
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
// 

struct group *getgrent(void)
{
    return 0;
}

void setgrent(void)
{
}

void endgrent(void)
{
}

struct lconv *localeconv(void)
{
    static struct lconv lc;   
#define CHAR_MAX     127
#define CHAR_MAX_S  "127"
    struct lconv* lcp = &lc;
   
    lcp->decimal_point = ".";   
    lcp->thousands_sep = "";   
    lcp->grouping = CHAR_MAX_S; 
    lcp->int_curr_symbol = "";   
    lcp->currency_symbol = "";   
    lcp->mon_decimal_point = "";   
    lcp->mon_thousands_sep = "";   
    lcp->mon_grouping = "";   
    lcp->positive_sign = "";   
    lcp->negative_sign = "";   
    lcp->int_frac_digits = CHAR_MAX;   
    lcp->frac_digits = CHAR_MAX;   
    lcp->p_cs_precedes = CHAR_MAX;   
    lcp->p_sep_by_space = CHAR_MAX;   
    lcp->n_cs_precedes = CHAR_MAX;   
    lcp->n_sep_by_space = CHAR_MAX;   
    lcp->p_sign_posn = CHAR_MAX;   
    lcp->n_sign_posn = CHAR_MAX;   
#undef CHAR_MAX   
#undef CHAR_MAX_S
    return lcp;
}

struct spwd *getspnam(const char *name)
{
    return 0;
}

struct spwd *getspent(void)
{
    return 0;
}

void setspent(void)
{
}

void endspent(void)
{
}

int tcdrain(int fd)
{
    return -1;
}

char * crypt(const char * param1, const char * param2)
{
    return 0;
}
