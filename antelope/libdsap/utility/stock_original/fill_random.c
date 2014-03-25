/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */

#include <string.h>

#include "stock.h"

static char *Image = 0 ; 
static int  Icount = 0 ;
static int  Random_buffer_size = 32768 ;

static void
require ( int nbytes ) {

/* straight from the srandom man page */
    static unsigned long state1[32] = {
       3,
       0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
       0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
       0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
       0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
       0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
       0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
       0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
       0xf5ad9d0e, 0x8999220b, 0x27fb47b9
       };

    static unsigned seed = 1;
    static int n = 128;
    unsigned int *saved ;
    int need, count ;

    if ( Icount == 0 ) {
	initstate(seed, state1, n);
	setstate(state1);
    }

    need = MAX(nbytes/4 + 1, Random_buffer_size)  ;
    saved = (unsigned int *) Image ;
    count = Icount / sizeof(int) ;
    if ( need > Icount ) {
	if ( Icount == 0 ) {
	    allot ( unsigned int *, saved, need ) ; 
	} else {
	    reallot ( unsigned int *, saved, need ) ; 
	}
	for ( ; count < need ; count++ )
	    saved[count] = random() ; 
	Image = (char *) saved ; 
	Icount = sizeof(int) * count ; 
    }
}

int
set_random (int intcount)
{
    int n = Random_buffer_size ; 
    Random_buffer_size = intcount ; 
    return n; 
}

void
fill_random ( char *packet, int nchar, int *pp ) 
{
    int i ;
    int p ; 

    p = *pp ;
    require (nchar) ; 
    insist ( nchar > 4 ) ;
    memcpy ( packet, pp, 4 ) ;
    for (i=4 ; i<nchar ; i++ ) {
	packet[i] = Image[p++] ; 
	p %= Icount ;
    }
    *pp = p ;
}

int
check_random ( char *packet, int nchar ) 
{
    int i ; 
    int p=0 ;
    int bad = 0 ;

    require (nchar) ;
    memcpy ( &p, packet, 4 ) ;
    for (i=4 ; i<nchar ; i++ ) {
	if ( packet[i] != Image[p++] && ! bad) {
	    bad = i+1 ; 
	}
	p %= Icount ;
    }
    return bad ; 
}

