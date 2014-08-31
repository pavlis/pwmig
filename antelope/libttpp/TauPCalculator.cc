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
/***** THIS IS DEFINITELY WRONG */
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


#define TBL_DIR        "tables/taup_ttimes/"
#define TBL_TBL         "iasp91"
#define	SIZE_PHCD	16
#define	MAXPHASES		60

static char tblpath[MAXPATHLEN] = "";

void TauPCalculator::tt_taup_set_table(const char *table_path)
{
    string base_error("TauPCalculator::tt_taup_set_table:  ");
    char fname[MAXPATHLEN], *tablebase, tablename[MAXPATHLEN];

    if ( table_path !=  0 && *table_path == '/' ) 
	strcpy ( tblpath, table_path ) ; 
    else {
	if ( table_path != 0 && *table_path != 0 ) 
	    tablebase = table_path ; 
	else if ( ( tablebase= getenv("TAUP_TABLE")) == NULL ) 
	    tablebase = TBL_TBL ; 
	if ( *tablebase != '/' ) 
	    strcpy ( tablename, TBL_DIR ) ; 
	strcat ( tablename, tablebase ) ; 
	strcat ( tablename, ".hed" ) ; 
	table_path = datafile ( "TAUP_PATH", tablename ) ;
	if ( table_path != 0 ) {
	    strcpy ( tblpath, table_path ) ; 
	    tblpath[strlen(tblpath)-4] = 0 ; 
	    free(table_path) ; 
	} else { 
            throw SeisppError(base_error
			+ "Cannot find travel time table for "
			+ tablename);
	}
    }

    strcpy ( fname, tblpath ) ; 
    strcat ( fname, ".hed" ) ; 
    if (access(fname, R_OK))
	throw SeisppError(base_error+"Permission denied for travel time table file "
		+ fname);
}

string TauPCalculator::taup_get_modname()
{
	int i;

        if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_get_modname:  ")
			+ "Travel time data path not set.  Initialization problem");
	for (i=strlen(tblpath); i>=0; i--) if (tblpath[i] == '/') {i++;break;}
	return(string(&(tblpath[i])));
}


void TauPCalculator::taup_setup(const char *model, const char *phases)
{
	/* This was the original C approach - now these are part of the object.
	static int ttopen=0;
	static int one=1;
	static int prnt[3] = {0, 0, 0};
	static int nn;
	static char pcntl[MAXPHASES][SIZE_PHCD];
	static char phass[512];
	static char old_phases[512] ;
       */

	char *phase;
	int k;

        /* Input phases array is a comma separated list.  This section 
         breaks these tokens up and puts them in the pcntl array of char*/
	strcpy (phass, phases);
	strcat (phass, ",");
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		//if (!phase[0]) continue;
		if (phase[0]!='\0') continue;
		strcpy (&pcntl[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) pcntl[nn][k] = ' ';
		nn++;
	}
        try {
            nn=tt_taup_set_table(model);
	} catch(SeisppError& serr){throw serr;};
	if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_setup:  ")
			+ "travel time table path variable was not initialized");

	tabin_ (&one, tblpath ) ; 
	brnset_ (&nn, pcntl, prnt, SIZE_PHCD);
	ttopen = 1;
}
/* Not sure this routine is required.   Current code only uses 
a test for sending it "all", which is not worth implementing anyway*/

/*

int
tt_taup_set_phases (phases)

char *    phases;

{
	static int ttopen=0;
	static int one=1;
	static int prnt[3] = {0, 0, 0};
	static int nn;
	static char pcntl[MAXPHASES][SIZE_PHCD];
	static char phass[512];
	static char old_phases[512] ;
	char *phase;
	int k;

	if ( strcmp(old_phases, phases) == NULL ) 
	    return 1 ;
	strcpy ( old_phases, phases) ; 
	strcpy (phass, phases);
	strcat (phass, ",");
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		if (!phase[0]) continue;
		strcpy (&pcntl[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) pcntl[nn][k] = ' ';
		nn++;
	}
	if (!tblpath[0]) 
	    tt_taup_set_table ((char *) 0) ;
	if (!tblpath[0])
	    return 0 ;

	tabin_ (&one, tblpath) ;
	brnset_ (&nn, pcntl, prnt, SIZE_PHCD);
	ttopen = 1;
	return (1);
}
*/


void TauPCalculator::tt_taup_set_event_depth(double edepth)
{
	/*
	static float depth;
	static float usrc[2];
	*/

	depth = edepth;
	if (depth < 0.001) depth = 0.001;
	depset_ (&depth, usrc);
}
vector<TauPComputedTime>  TauPCalculator::tt_taup(double del)
{
/*
	static int maxphases;
	static int nphase;
	static float del, tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
	static char phcd[MAXPHASES][SIZE_PHCD];
	static char *phss[MAXPHASES];
	static double tts[MAXPHASES];
	int k, l;

	*nph = 0;
	maxphases = MAXPHASES;

	del = ddistance (elat, elon, slat, slon);
	trtm_ (&del, &maxphases, &nphase, &tt[(*nph)], &dtdd[(*nph)],
			&dtdh[(*nph)], &dddp[(*nph)], &(phcd[(*nph)]), SIZE_PHCD);
*/
	int nphases;   // static above, but does not seem necessary 
	trtm_ (&del, &maxphases, &nphases, &(this->tt[0]),&(this->dtdd[0]),
		&(this->dtdh[0]), &(this->dddp[0]), &(this->dpcd[0]),
		SIZE_PHCD);
	//if (nphase <= 0) return (1);
	if(nphase < 0) 
	   throw SeisppError(base_error 
		+ "taup trtm function returned no results from phase list "
		+ phass);
	vector<TauPComputedTime> result;
	result.reserve(nphase);
	TauPComputedTime ttk;
	for(k=0;i<nphase;++k)
	{
	    /* It seems phcd tokens are blank terminated so we have to scan
	    for that instead of a null */
	    char phstmp[SIZE_PHCD];
	    for(l=0;l<SIZE_PHCD;++l)
	        if( (phcd[k][l] == ' ') || (l==SIZE_PHCD-1) )
		{
		    phcd[k][l]='\0';
		    break;
		}
	   ttk.phase=string(phcd[k]);
	   ttk.ttime=tt[k];
	   result.push_back(ttk);
	}
	return(result);
}

