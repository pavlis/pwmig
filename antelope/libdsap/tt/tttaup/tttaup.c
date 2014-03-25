
#include "tt.h"
#include "stock.h"
#include "coords.h"
#include "tttaup.h"

#define	SIZE_PHCD	16
#define	MAXPHASES		60

typedef struct Taup_hook {
    double depth ;
    char *model ;
    char *phase_code ; 
} Taup_hook ; 

static void
taup_free_hook ( Taup_hook *old )
{
    free(old->model) ;
    free(old->phase_code) ; 
    free(old) ; 
}

int tttaup (
    char *model,
    char *phase_code,
    int mode,
    TTGeometry *geometry,
    Tbl **timesp,
    Hook **hookp
)
{
    Taup_hook *old ;
    int maxphases=MAXPHASES, nphase ; 
    float delta ;
    float tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
    char phcd[MAXPHASES][SIZE_PHCD];
    int k ;
    TTTime atime ;
    int result ;
    double convert ;
    int setupcode;

    if (*hookp == NULL) {
	*hookp = (Hook *)new_hook ( taup_free_hook ) ; 
	allot ( Taup_hook *, old, 1) ;
	(*hookp)->p = old ;
	old->depth = -1.0 ;

	old->model = strdup(model) ; 
	old->phase_code = strdup(phase_code) ;
        setupcode=taup_setup (model, phase_code);
        if(setupcode) {
		free(old->model) ;
		old->model = strdup("NONESUCH") ;
		return setupcode ; 
	}

	old->depth = geometry->source.z ; 
	tt_taup_set_event_depth (geometry->source.z) ;
    } else {
	int reread ;

	old = (Taup_hook *) (*hookp)->p ;
	reread = 0 ;

	if ( strcmp ( old->model, model) != 0 ) {
	    free(old->model) ;
	    old->model = strdup(model) ;
	    reread = 1 ;
	}

	if ( strcmp ( old->phase_code, phase_code) != 0 ) {
	    free(old->phase_code) ;
	    old->phase_code = strdup(phase_code) ;
	    reread = 1 ;
	}

	if (reread) {
            setupcode=taup_setup (model, phase_code);
            if(setupcode) {
		    free(old->model) ;
		    old->model = strdup("NONESUCH") ;
		    return setupcode ; 
	    }
	}

	if (old->depth != geometry->source.z || reread ) {
	    old->depth = geometry->source.z ; 
	    tt_taup_set_event_depth (geometry->source.z) ;
	}

    }

    delta = ddistance ( 
	geometry->source.lat, geometry->source.lon,
	geometry->receiver.lat, geometry->receiver.lon ) ;

    if ( mode & TT_CORRECTION_ONLY ) {
	nphase = 0 ;
    } else {
	trtm_ (&delta, &maxphases, &nphase, 
	    tt, dtdd, dtdh, dddp, phcd, SIZE_PHCD);
    }

    atime.deriv[0] = 0.0 ; 
    atime.deriv[1] = 0.0 ; 
    atime.deriv[2] = 0.0 ; 

    convert = 1.0/deg2km(1.0) ; /* convert is degrees/kilometer */
    for (k=0 ; k< nphase ; k++ ) {
	fs2cs ( atime.phase, phcd[k], SIZE_PHCD ) ; 
	if ( mode & TT_SLOWNESS ) {
	    atime.value = dtdd[k] * convert ; 
	    if (mode & TT_DERIVATIVES) {
		atime.deriv[2] = 0.0 ; 
						    /* dddp is in radian^2/sec */
		atime.deriv[0] = atime.deriv[1] =  convert * convert / deg(deg(dddp[k])) ; 
	    } 
	} else {
	    atime.value = tt[k] ; 
	    if (mode & TT_DERIVATIVES) {
		atime.deriv[2] = dtdh[k] ; 
		atime.deriv[0] = atime.deriv[1] = dtdd[k] * convert ; 
	    } 
	}
	settbl(*timesp, k, &atime) ;
    }

    if ( mode & TT_SLOWNESS ) 
	result = 3 ; /* no station corrections, no derivatives */
    else
	result = 2 /* no station corrections available */ ; 

    return result ;
}

/* $Id: tttaup.c,v 1.1.1.1 1997/04/12 04:18:48 danq Exp $ */
