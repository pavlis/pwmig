
/* These routines simplify the passing of character strings between 
 * Fortran  and c 
 */
#include "stock.h"

int fs2cs ( dst, src, nmax ) 
char *dst, *src ; 
int nmax ; 
{ 
    char *orig ;
    int i ; 
    orig = dst ;
    for ( i = 0 ; i<nmax ; i++ ) 
	dst[i] = src[i] ;
    for ( i=nmax-1 ; i>= 0 && dst[i] == ' ' ; i-- ) 
	;
    dst[i+1] = 0 ; 
    return 0 ; 
}

int cs2fs ( dst, src, nmax ) 
char *dst, *src ; 
int nmax ; 
{
    int i, n ; 
    n = MIN ( nmax, strlen(src) ) ; 
    for ( i=0 ; i<n ; i++ ) 
	*dst++ = *src++ ; 
    for ( ; i<nmax ; i++ ) 
	*dst++ = ' ' ; 
    return 0 ; 
}

char *newcs ( fs, n )
char *fs ;
int n ;
{
    char *s ;
    if ( *fs == 0 ) {
	allot ( char *, s, 2 ) ; 
	*s = 0 ; 
    } else { 
	allot ( char *, s, n+1 ) ;
	strncpy ( s, fs, n ) ;
	while ( n-- > 0 && s[n] == ' ')  ;
	s[n+1] = 0 ;
    }
    return s ;
}

int strcpy_pad ( dst, dstmax, src, srcmax, pad ) 
char *dst, *src ; 
int dstmax, srcmax ;
char pad ; 
{ 
    int i, nmax ; 
    nmax = MIN ( dstmax, srcmax ) ;
    for ( i = 0 ; i<nmax ; i++ ) 
	*dst++ = *src++ ; 
    for ( ; i<dstmax ; i++ ) 
	*dst++ = pad ; 
    return 0 ; 
}

/* $Id: fs2cs.c,v 1.1.1.1 1997/04/12 04:19:00 danq Exp $ */
