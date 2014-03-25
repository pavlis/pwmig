
#include <string.h>
#include "stock.h"

char *
expand_env ( s ) 
char *s ; 
{
    char *result, *replacement, *s2 ; 
    char env[STRSZ] ; 
    int size ;
    int i, j, n, ncopy ; 
    char *first, *match ;

    if ( strchr ( s, '$' ) == 0 ) 
	return strdup(s) ; 

    size = MAX(STRSZ, 2*strlen(s)) ;
    allot ( char *, result, size ) ; 
    i = 0 ; 

    while ( *s != 0 ) {
	while ( *s != 0 && *s != '$' ) 
	    result[i++] = *s++ ; 
	if ( *s == '$' ) {
	    first = s ;
	    if ( *(++s) == '{' ) {
		s++ ; 
		match = strchr(s,'}') ; 
		if (match == 0) {
		    n = strlen(s) ;
		    s2 = s + n ; 
		} else {
		    n = match - s ;
		    s2 = s + n + 1 ;
		}
	    } else { 
		match = s ; 
		while ( *match != 0 && *match != ':' && *match != '/' ) 
		    match++ ; 
		n = match - s ; 
		s2 = s + n ;
	    }
	    ncopy = MIN(STRSZ-1, n) ;
	    strncpy ( env, s, ncopy ) ; 
	    env[ncopy] = 0 ; 
	    s = s2  ;
	    if ( (replacement = getenv(env)) != 0 ) {
		if (i+strlen(replacement) >= size ) {
		    size = i + strlen(replacement) + strlen(s) + 2 ;
		    reallot ( char *, result, size ) ; 
		}
		j = 0 ;
		while ( replacement[j] != 0 ) 
		    result[i++] = replacement[j++] ; 
	    } else { /* punt, and give back the original string */
		while ( first < s ) 
		    result[i++] = *first++ ; 
	    }
	}
    }
    result[i] = 0 ; 
    return result ;
}



	    



/* $Id: expand_env.c,v 1.1 1998/02/17 22:39:38 danq Exp $ */
