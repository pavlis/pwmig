
#include <stdio.h>
#include <ctype.h>

void dump ( a, l )
char *a ;
int l ;
{
    int i ;
    char ascii[17], *ap ;

    ap = ascii ;
    for ( i=0 ; i<l ; i++)
        {
        if ( i % 16 == 0 ) fprintf ( stderr, "%08x ", a ) ;
        if ( i % 2 == 0 ) fprintf ( stderr, " " ) ;
        fprintf ( stderr, "%02x", *a ) ;
        *ap++ = isprint (*a) ? *a : '_' ;
        if ( i % 16 == 15 )
            {
            *ap = 0 ;
            fprintf ( stderr, " %s\n", ascii );
            ap = ascii ;
            }
        a++ ;
        }
    fprintf ( stderr, "\n" ) ; 
}      
       

/* $Id: dump.c,v 1.1.1.1 1997/04/12 04:19:00 danq Exp $ */
