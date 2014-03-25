
/* This shell sort is more or less directly from Algorithms in Modula 3
 * by Robert Sedgewick, pp 109.
 */
#include <memory.h>
#include "stock.h"


void shellsort ( a, n, size, compare, private )
char *a ; 
int n ; 
int size ; 
int (*compare)() ; 
void *private ; 
{
    int h, i, j ; 
    char *s ; 

    allot ( char *, s, size ) ; 

    for ( h=1 ; h<n ; h=3*h+1 ) ; 

    do { 
	h = h / 3 ; 

	for ( i=h ; i<n ; i++ ) 
	    {
	    j = i ; 
	    memcpy ( s, a+i*size, size ) ; 
	    while ( j >= h 
		&& (*compare) ( a+(j-h)*size, s, private ) > 0 )
		{
		memcpy ( a+j*size, a+(j-h)*size, size ) ; 
		j -= h ; 
		}
	    memcpy ( a+j*size, s, size ) ; 
	    }
	} while ( h > 1 ) ; 
    free ( s ) ; 
}


/* $Id: shellsort.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
