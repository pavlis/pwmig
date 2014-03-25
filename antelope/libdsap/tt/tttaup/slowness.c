#include "tttaup.h"
#include "stock.h"
#include "coords.h"

#define	MAXPHASES		60
#define	SIZE_PHCD	16


int
tt_taup_slowness (elat, elon, slat, slon, selev, vp_cor, vs_cor, 
		nph, times, slowness, zslowness, dslowness, phs)

double     elat;
double           elon;
double                 slat;
double                       slon;
double                             selev;
double                                    vp_cor;
double                                            vs_cor;
int *          nph;
double **           times;
double **	    slowness ;
double **	    zslowness ;
double **	    dslowness ;
char ***                    phs;

{
	static int maxphases;
	static int nphase;
	static float del, tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
	static char phcd[MAXPHASES][SIZE_PHCD];
	static char *phss[MAXPHASES];
	static double tts[MAXPHASES], dtdds[MAXPHASES], dtdhs[MAXPHASES], dddps[MAXPHASES];
	int k, l;

	*nph = 0;
	maxphases = MAXPHASES;

	del = ddistance (elat, elon, slat, slon);
	trtm_ (&del, &maxphases, &nphase, &tt[(*nph)], &dtdd[(*nph)],
			&dtdh[(*nph)], &dddp[(*nph)], &(phcd[(*nph)]), SIZE_PHCD);
	if (nphase <= 0) return (1);
	for (k=(*nph); k<(*nph)+nphase; k++) {
		for (l=0; l<SIZE_PHCD-1; l++) if (phcd[k][l] == ' ') break;
		phcd[k][l] = '\0';
	}
	(*nph) += nphase;
	for (k=0; k<(*nph); k++) {
		phss[k] = &(phcd[k][0]);
		tts[k] = tt[k];
		dtdds[k] = dtdd[k] ; 
		dtdhs[k] = dtdh[k] ; 
		dddps[k] = dddp[k] ; 
	}
	*times = tts;
	*slowness = dtdds ; 
	*zslowness = dtdhs ;
	*dslowness = dddps ;
	*phs = phss;
	return (1);
}

int arr_slowness ( delta, depth, phases, slowness ) 
double delta, depth ; 
char ***phases ;
double **slowness ;
{
    double *times, *zslowness, *dslowness ;
    int nphases ;

    if ( tt_taup_set_phases ("all") == 0 ) {
	nphases = 0 ;
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup_slowness (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, 
		&times, slowness, &zslowness, &dslowness, phases) ;
    }
    return nphases ;
}

double phase_slowness ( phase, delta, depth ) 
char *phase ;
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times, *slowness, *zslowness, *dslowness ; 
    int nphases ; 
    int i ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup_slowness (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times, 
		&slowness, &zslowness, &dslowness, &phases) ;
	arrival = -1.0 ; 
	for ( i=0 ; i<nphases ; i++ ) 
	    if (strcmp(phases[i],phase) == NULL) {
		arrival = slowness[i] ;
		break ;
	    }
    }
    return arrival ;
}

double pphase_slowness ( delta, depth ) 
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times, *slowness, *zslowness, *dslowness ; 
    int i, nphases ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {

	tt_taup_set_event_depth (depth) ; 
	tt_taup_slowness (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times, 
		&slowness,  &zslowness,  &dslowness, &phases);

	if ( nphases < 1 ) 
	    arrival = 0.0 ;
	else if ( delta <= 105.0 ) {
	    arrival = slowness[0] ;
	} else { 
	    /* eliminate diffracted phases beyond 105 degrees */
	    arrival = 0.0 ;
	    for ( i=0 ; arrival == 0.0 && i<nphases ; i++ ) {
		if ( strstr(phases[i], "diff") == NULL ) {
		    switch ( *(phases[i]) ) {
			case 'P':
			case 'p':
			    arrival = slowness[i] ; 

#if 0
			    if ( i != 0 ) {
				int j ;
				printf ( "\n%5.1f  %5.1f\n", delta, depth ) ;
				for ( j=0 ; j<= i ; j++ ) 
				    printf ( "%-8s %6.3f\n", phases[j], slowness[j]) ;
			    }
#endif 

			    break ;
		    }
		}
	    } 
	}
    }
    return arrival ;
}

double sphase_slowness ( delta, depth ) 
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times, *slowness, *zslowness, *dslowness ; 
    int nphases ; 
    int i ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup_slowness (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times, 
		&slowness, &zslowness, &dslowness, &phases) ;
	if ( nphases < 1 ) 
	    arrival = -1.0 ;
	else {
	    /* eliminate diffracted phases beyond 105 degrees */
	    arrival = 0.0 ; 
	    for ( i=0 ; arrival == 0.0 && i<nphases ; i++ ) {
		if ( strchr(phases[i], 'p') == NULL 
		    && strchr(phases[i], 'P') == NULL ) {
			arrival = slowness[i] ; 

#if 0
			if ( i != 0 ) {
			    int j ;
			    printf ( "\n%5.1f  %5.1f\n", delta, depth ) ;
			    for ( j=0 ; j<= i ; j++ ) 
				printf ( "%-8s %6.3f\n", phases[j], slowness[j]) ;
			}
#endif 

			break ;
		}
	    }
	}
    }
    return arrival ;
}
