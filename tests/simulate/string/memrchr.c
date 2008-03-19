/* $Id$	*/

#ifndef __AVR__
# define _GNU_SOURCE	/* to include memrchr()	*/
# define PRINTFLN(line, fmt, ...)	\
    printf("\nLine %d: " fmt "\n", line, ##__VA_ARGS__)
# define EXIT(code)	exit ((code) < 255 ? (code) : 255)
# define memcmp_P	memcmp
#else
# if defined(__AVR_ATmega128__)
  /* ATmega128 has enough RAM for sprintf(), print to 0x2000 in XRAM. */
#  define PRINTFLN(line, fmt, ...)	\
    sprintf ((char *)0x2000, "\nLine %d: " fmt "\n", line, ##__VA_ARGS__)
# else
   /* small AVR */
#  define PRINTFLN(args...)
# endif
# define EXIT	exit
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "progmem.h"

void Check (int line, const char *s, int c, size_t len, int expect)
{
    char t[320];
    char *p;
    
    if (len > sizeof(t))
	exit (1);
    memcpy_P (t, s, len);
    p = memrchr (t, c, len);

    if (expect < 0) {
	if (p) {
	    PRINTFLN (line, "non zero pointer is returned");
	    EXIT (line);
	}
    } else {
	if (p != t + expect) {
	    PRINTFLN (line, "expect= %d  result= %d", expect, p - t);
	    EXIT (line + 1000);
	}
    }
    if (memcmp_P (t, s, len)) {
	PRINTFLN (line, "string is changed");
	EXIT (line + 2000);
    }
}

#define CHECK(s, c, len, expect)	do {	\
    Check (__LINE__, PSTR(s), c, len, expect);	\
} while (0)

int main ()
{
    /* Not found	*/
    CHECK ("", 0, 0, -1);
    CHECK ("", 255, 0, -1);
    CHECK ("ABCDEF", 'a', 6, -1);
    
    /* Found	*/
    CHECK ("\000", 0, 1, 0);
    CHECK ("\001", 1, 1, 0);
    CHECK ("\377", 255, 1, 0);
    CHECK ("987654321", '7', 9, 2);

    /* '\0' has't a special sense	*/
    CHECK ("12345", 0, 6, 5);
    CHECK (".\000.", 0, 3, 1);
    CHECK ("\000a\000b", 'b', 4, 3);

    /* Last occurance	*/
    CHECK ("abcdabcd", 'b', 8, 5);
    CHECK ("........", '.', 8, 7);
    
    /* 'c' converted to a char	*/
    CHECK ("ABCDEF", 'A'+0x100, 6, 0);
    CHECK ("ABCDE\377", ~0, 6, 5);
    
    /* Very long string	*/
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "...............................................................*"
	   "................................................................",
	   '*', 320, 255);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   "*...............................................................",
	   '*', 320, 256);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   ".*..............................................................",
	   '*', 320, 257);

    return 0;
}
