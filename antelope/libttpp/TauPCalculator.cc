#include "slowness.h"
#include "gclgrid.h"
#include "TravelTimeCalculator.h"
namespace PWMIG
{
using namespace std;
using namespace SEISPP;
using namespace PWMIG;

/* This private method does all the real work for both constructors.   
The default constructor just uses a default model */
TauPCalculator::initial_Taup(string mod)
{
    string base_error("TauPCalculator constructor:  ");
    this->model=mod;
    depth=0.0;   // initial depth for initialize
    int setupcode;
    setupcode=taup_setup (model, phase_code);
    if(setupcode != 0) throw SeisppError(base_error
			+ "taup_setup function failed");
    tt_taup_set_event_depth(depth);
}
TauPCalculator::TauPCalculator()
{
    /* default model same as antelope */
    string mod("iasp91");
    this->initialize_Taup(mod);
}
TauPCalculator::TauPCalculator(const char *mod)
{
    this->initialize_Taup(mod);
}
TauPCalculator::~TauPCalculator()
{
}


double TauPCalculator::Ptime(double rdelta, double evdepth)
{
	/* taup wants delta in degrees we use radians */
	double delta=deg(rdelta);
    double arrival ; 
    char **phases ; 
    double *times ; 
    int i, nphases ; 

    if ( tt_taup_set_phases ("all") == 0 ) {
	arrival = -1.0 ; 
    } else {

	if(evdepth!=depth)
	{
	    depth=evdepth;
	    tt_taup_set_event_depth (depth) ; 
	}
	/* old function signature */
	//tt_taup (0.0, 0.0, 0.0, delta, 0.0, 0.0, 0.0, &nphases, &times,&phases);
	/* Replacement for new C++ interface */
	tt_taup(delta,nphases,times,phases);

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
			    break ;
		    }
		}
	    } 
	}
    }
    return arrival ;

}
double TauPCalculator::Stime(double delta, double depth)
{
	double arrival;
	/* delta in here is radians because phasetime will convert to degrees*/
	arrival=this->phasetime(delta,depth,"S");
	return(arrival);
}
double TauPCalculator::phasetime(double rdelta, double depth, const char *phase)
{
	/* taup wants delta in degrees we use radians */
	double delta=deg(rdelta);
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
/* The implementation of the pure virtual methods from base class
here are just wrappers to call those with the delta,depth signature.
This is because tau-p is for radially symmetric earth models by definition
so all that matters is distance and depth.  These functions are painfully
parallel.   All inputs are radians that are converted by lower level methods
to degrees to mesh correctly with taup procedures.*/
double TauPCalculator::phasetime(Geographic_point& source, 
		Geographic_point& receiver,const char *phase)
{
	double distance;
	distance=ddistance(s.lat,s.lon,r.lat,r.lon);
	double depth;
	depth=r0_ellipse(s.lat)-s.r;
	return(this->phasetime(distance,depth,phase));	
}
double TauPCalculator::Ptime(Geographic_point& source,Geographic_point& receiver)
{
	double distance;
	distance=ddistance(s.lat,s.lon,r.lat,r.lon);
	double depth;
	depth=r0_ellipse(s.lat)-s.r;
	return(this->Ptime(distance,depth,phase));	
}
double TauPCalculator::Stime(Geographic_point& source,Geographic_point& receiver)
{
	double distance;
	distance=ddistance(s.lat,s.lon,r.lat,r.lon);
	double depth;
	depth=r0_ellipse(s.lat)-s.r;
	return(this->Stime(distance,depth,phase));	
}

SlownessVector TauPCalculator::phaseslow(Geographic_point& source,
		Geographic_point& receiver,const char *phase)
{
	double distance,baz,azimuth;
	dist(r.lat,r.lon,s.lat,s.lon,&distance,&baz);
	/* dist returns a back azimuth but for a slowness vector we need 
	direction of propagation. */
	azimuth=baz+M_PI;
	if(azimuth>(2.0*M_PI)) azimuth -= (2.0*M_PI);
	double delta=deg(distance);
	/* This section comes from original slowness.c */
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
    /* End original - arrival is a slowness amplitude.  We need to convert this
    to a vector.   A conversion is needed because a SlownessVector object in
    SEISPP has units of s/km while taup returns global for of ray parameter. */
    arrival *= EQUATORIAL_EARTH_RADIUS;
    SlownessVector result(arrival*sin(azimuth),arrival*cos(azimuth));
    return(result);
}
SlownessVector TauPCalculator::Pslow(Geographic_point& source,Geographic_point& receiver)
{
	/* I will follow the orignal version of this code from the datascope library
	that deals with Pdiff correctly.  Parts of this are copied from phaseslow*/
	
	double distance,baz,azimuth;
	dist(r.lat,r.lon,s.lat,s.lon,&distance,&baz);
	/* dist returns a back azimuth but for a slowness vector we need 
	direction of propagation. */
	azimuth=baz+M_PI;
	if(azimuth>(2.0*M_PI)) azimuth -= (2.0*M_PI);
	double delta=deg(distance);
	/* This section comes from original slowness.c */
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
			    break ;
		    }
		}
	    } 
	}
    }
    /* End original - arrival is a slowness amplitude.  We need to convert this
    to a vector.   A conversion is needed because a SlownessVector object in
    SEISPP has units of s/km while taup returns global for of ray parameter. */
    arrival *= EQUATORIAL_EARTH_RADIUS;
    SlownessVector result(arrival*sin(azimuth),arrival*cos(azimuth));
    return(result);	
}
SlownessVector TauPCalculator::Sslow(Geographic_point& source,Geographic_point& receiver)
{
	return(this->phaseslow(source,receiver,"S");
}

dvector TauPCalculator::TimePhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)
{
}
dmatrix TauPCalculator::SlownessPhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)
{
}
