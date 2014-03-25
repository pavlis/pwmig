#include "stock.h"
#include "tttaup.h"

int arrtimes_ ( f_delta, f_depth, f_phases, f_times, n_phases )
double *f_delta ;
double *f_depth ;
char *f_phases ;
double *f_times ;
int n_phases ;
{
    char **phases ; 
    int i, nphases ;
    double *times ;

    nphases = arrtimes(*f_delta, *f_depth, &phases, &times );
    for ( i=0 ; i<nphases ; i++ ) {
	cs2fs ( f_phases+i*n_phases, phases[i], n_phases ) ;
	f_times[i] = times[i] ; 
    }
    return nphases ;
}

double phasetime_ ( f_phase, f_delta, f_depth, n_phase )
char *f_phase ;
double *f_delta ;
double *f_depth ;
int n_phase ;
{
    double retcode;
    char *phase = newcs ( f_phase, n_phase) ; 
    retcode = phasetime(phase, *f_delta, *f_depth );
    free ( phase ) ; 
    return retcode;
}

double pphasetime_ ( f_delta, f_depth )
double *f_delta ;
double *f_depth ;
{
    return pphasetime(*f_delta, *f_depth );
}

double sphasetime_ ( f_delta, f_depth )
double *f_delta ;
double *f_depth ;
{
    return sphasetime(*f_delta, *f_depth );
}


double ptime_ ( f_delta, f_depth )
double *f_delta ;
double *f_depth ;
{
    return pphasetime(*f_delta, *f_depth );
}

double stime_ ( f_delta, f_depth )
double *f_delta ;
double *f_depth ;
{
    return sphasetime(*f_delta, *f_depth );
}


/* $Id: ptime_.c,v 1.1.1.1 1997/04/12 04:18:47 danq Exp $ */
