#include "tttaup.h"
#include "stock.h"


int arrtimes ( delta, depth, phases, times ) 
double delta, depth ; 
char ***phases ;
double **times ;
{
    int nphases ;

    if ( tt_taup_set_phases ("all") == 0 ) {
	nphases = 0 ;
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, times, phases) ;
    }
    return nphases ;
}

double phasetime ( phase, delta, depth ) 
char *phase ;
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times ; 
    int nphases ; 
    int i ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times, &phases) ; 
	arrival = -1.0 ; 
	for ( i=0 ; i<nphases ; i++ ) 
	    if (strcmp(phases[i],phase) == NULL) {
		arrival = times[i] ;
		break ;
	    }
    }
    return arrival ;
}

double pphasetime ( delta, depth ) 
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times ; 
    int i, nphases ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {

	tt_taup_set_event_depth (depth) ; 
	tt_taup (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times,&phases);

	if ( nphases < 1 ) arrival = 0.0 ; 
	else if ( delta <= 105.0 ) {
	    arrival = times[0] ;
	} else { 
	    arrival = 0.0 ; 
	    /* eliminate diffracted phases */
	    for ( i=0 ; arrival == 0.0 && i<nphases ; i++ ) {
		if ( strstr(phases[i], "diff") == NULL ) {
		    switch ( *(phases[i]) ) {
			case 'P':
			case 'p':
			    arrival = times[i] ; 
#if 0
			    if ( i != 0 ) {
				int j ;
				printf ( "\n%5.1f  %5.1f\n", delta, depth ) ;
				for ( j=0 ; j<= i ; j++ ) 
				    printf ( "%-8s %6.3f\n", phases[j], times[j]) ;
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

double sphasetime ( delta, depth ) 
double delta, depth ; 
{
    double arrival ; 
    char **phases ; 
    double *times ; 
    int nphases ; 
    int i ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {
	tt_taup_set_event_depth (depth) ; 
	tt_taup (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times, &phases) ; 
	if ( nphases < 1 ) arrival = -1.0 ; 
	else {
	    arrival = 0.0 ; 
	    for ( i=0 ; arrival == 0.0 && i<nphases ; i++ ) {
		if ( strchr(phases[i], 'p') == 0 
		    && strchr(phases[i], 'P') == 0 ) {
		    arrival = times[i] ;

#if 0
		    if ( i != 0 ) {
			int j ;
			printf ( "\n%5.1f  %5.1f\n", delta, depth ) ;
			for ( j=0 ; j<= i ; j++ ) 
			    printf ( "%-8s %6.3f\n", phases[j], times[j]) ;
		    }
#endif

		    break ;
		}
	    }
	}
    }
    return arrival ;
}
