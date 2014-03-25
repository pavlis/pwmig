
#include <string.h>
#include "stock.h"

int blank ( s )
char *s ;
{
    while ( *s == ' ' ) s++;
    return *s == 0 ;
}

int whitespace ( s )
char *s ;
{
    while ( *s == ' ' || *s == '\t' || *s == '\n' ) s++;
    return *s == 0 ;
}

Tbl *split ( s, c ) 
char *s ; 
int c ; 
{
    Tbl *tbl ; 

    tbl = newtbl(5) ;

    while ( *s != 0 )
	{
	while ( *s == c ) s++ ;
	if ( *s != 0 ) 
	    pushtbl ( tbl, s ) ; 
	while ( *s != c && *s != 0 ) 
	    s++ ; 
	if ( *s == c ) *s++ = 0 ; 
	}
    
    return tbl ; 
}

/* copy source to dest, padding with blanks, no trailing zero */
void 
sncopy(dest, source, n)
char           *dest,
               *source;
int             n;
{
    int             i = 0;

    while (i++ < n && *source != 0)
	*dest++ = *source++;
    for (--i ; i < n; i++)
	*dest++ = ' ';
}

/* copy source to dest, add trailing zero */
void 
szcopy(dest, source, n)
char           *dest,
               *source;
int             n;
{
    int             i = 0;

    for (i = 0; i < n; i++)
	*dest++ = *source++;
    *dest = 0;
}

/* copy source to dest, add trailing zero, strip leading, trailing blanks */
void 
copystrip(dest, source, n)
char           *dest,
               *source;
int             n;
{
    int i, start, last ; 

    for (start=0 ; start<n && source[start] == ' ' ; start++ ) 
	;
    for (last=n-1 ; last>=0 && source[last] == ' ' ; last-- ) 
	;
    last++ ;

    for(i=start ; i<last ; i++ ) 
	*(dest++) = source[i] ; 

    *dest = 0;
}

void *
memdup ( void *a, int n ) 
{
    char *b ;
    allot ( char *, b, n ) ; 
    memcpy(b,a,n) ; 
    return (void *) b ;
}


/* $Id: strings.c,v 1.2 1997/08/08 17:06:16 danq Exp $ */
