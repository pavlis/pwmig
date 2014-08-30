#ifndef __tt__
#define __tt__ 

#include "stock.h" 

#define TTNAME_SIZE	16
#define TTPHASE_SIZE	16

#define TT_SLOWNESS 		1	/* return slowness, not travel time*/
#define TT_DERIVATIVES 		2	/* return derivatives also */
#define TT_APPLY_CORRECTIONS	4	/* apply (station) corrections */
#define TT_CORRECTION_ONLY	8	/* return (station) corrections only */

typedef struct TTPoint {
    double lat, lon ; 	     /* latitude and longitude in degrees */
    double z ;		     /* depth in kilometers (- elevation for stations)*/
    double time ;	     /* epoch time in seconds: null is 0.0  */
    char name[TTNAME_SIZE] ; /* location (usually station name) 
				(null terminated) */
} TTPoint ; 

typedef struct TTGeometry {
    TTPoint	source ; 
    TTPoint	receiver ;
} TTGeometry ; 

typedef struct TTTime {
    double value ;		/* seconds or seconds/kilometer */
    double deriv[3] ; 		/* seconds/kilometer or seconds/kilometer^2 */
    char phase[TTPHASE_SIZE] ; 	/* phase name (null terminated) */
} TTTime ;

extern int ttcalc ( 	/* generic travel time calculator */
	     char *method,      /* taup, uniform, locsat, cvlayer */
	     char *model, 	/* iasp91, kyrghyz, ... */
	     char *phase_code,	/* specific phase
				   P	- first p phase
				   P+	- all phases beginning with p or P
	     			   S	- first s phase
				   S+	- all phases beginning with s or S
				   all	- all phases
				   comma separated list of any of above
				*/
	     int mode, 		/* Mode flag: or of following:
			
					TT_SLOWNESS
					TT_DERIVATIVES
					TT_APPLY_CORRECTION
					TT_CORRECTION_ONLY
				*/
	     TTGeometry *geometry, /* source and receiver specification */
	     Tbl **times, 	/* list of TTTime structures */
	     Hook **hook 	/* hook to hold ttcalc internal state
				    for initialization and freeing
				    *hook should be zero for the first call 
				    call free_hook ( Hook **hook) to free
				    the hook 
				*/
) ;

/*
Return codes:

    0 for complete success

    < 0 indicates failure
    	-1 no phases
	-2 parameter error
	-3 no method library
	-4 no model

    > 0 indicates some trouble

    	1  no derivatives, even though requested derivatives
	2  no (station) corrections available


In every case where ttcalc returns a non-zero error code, the caller
should expect a more verbose error message on the error log.
*/

#endif

/* $Id: tt.h,v 1.1.1.1 1997/04/12 04:19:08 danq Exp $ */
