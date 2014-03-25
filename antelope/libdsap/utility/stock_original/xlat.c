/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */

#include "stock.h"

char *
xlatnum ( int num, Xlat *xlat, int nxlat )
{
    int i ;
    for ( i=0 ; i<nxlat ; i++ ) {
        if ( xlat[i].num == num ) {
            return xlat[i].name ;
        }
    }
    return 0 ;
}
 
int
xlatname ( char *name, Xlat *xlat, int nxlat )
{
    int i ;
    for (i=0 ; i<nxlat ; i++ ) {
        if ( strcmp(name,xlat[i].name) == 0 ) {
            return xlat[i].num ;
        }
    }   
    return -1 ;
}


