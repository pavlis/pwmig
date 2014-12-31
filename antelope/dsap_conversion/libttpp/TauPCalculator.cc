#include <unistd.h>
#include "slowness.h"
#include "gclgrid.h"
#include "TravelTimeCalculator.h"
#include "TauPCalculator.h"
#define TBL_DIR        "tables/taup_ttimes/"
#define TBL_TBL         "iasp91"
#define	SIZE_PHCD	16
#define	MAXPHASES		60

namespace PWMIG
{
using namespace std;
using namespace SEISPP;
using namespace PWMIG;

/* This private method does all the real work for both constructors.   
The default constructor just uses a default model */
void TauPCalculator::initialize_Taup(string mod)
{
	/* this used to be done in a static int C style initialization and used
	defines as a alias for 1 =true and 0=false.   Converted to numeric
	equivalent to avoid the confusing curly brackets of the orignal initialzition */
	for(int i=0;i<30;++i) prtflc_.segmsk[i]=1;
	prtflc_.prnt[0]=0;   prtflc_.prnt[1]=0;  
    maxphases=MAXPHASES; 
    string base_error("TauPCalculator constructor:  ");
    this->model=mod;
    depth=0.0;   // initial depth for initialize
    // Not sure this is needed but original code initialized these
    for(int k=0;k<3;++k) prnt[k]=0;
    // This must be initialized before calling tt_taup_set_phases
    tblpath[0]='\0';
    ttopen=false;
    // Set initial phase to all
    try {
        taup_setup (model.c_str(), "all");
        tt_taup_set_event_depth(depth);
    }catch(...){throw;};
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
/* This procedure copies all the struct data
   from f2c common blocks from parent to local copy. Put in a single 
   procedure to improve maintainability and to avoid duplicate code
   in copy constructor and operator =.   
   */
void TauPCalculator::copy_f2c_common_block_data(const TauPCalculator& parent)
{
    int i;
    /* First copy Brkc data */
    brkc_.zs = parent.brkc_.zs;
    for(i=0;i<30;++i)
        brkc_.pk[i]=parent.brkc_.pk[i];
    for(i=0;i<702;++i)
        brkc_.pu[i]=parent.brkc_.pu[i];
    for(i=0;i<200;++i)
        brkc_.pux[i]=parent.brkc_.pux[i];
    for(i=0;i<700;++i)
        brkc_.tauu[i]=parent.brkc_.tauu[i];
    for(i=0;i<200;++i)
        brkc_.xu[i]=parent.brkc_.xu[i];
    for(i=0;i<200;++i)
        brkc_.px[i]=parent.brkc_.px[i];
    for(i=0;i<200;++i)
        brkc_.xt[i]=parent.brkc_.xt[i];
    for(i=0;i<2250;++i)
        brkc_.taut[i]=parent.brkc_.taut[i];
    for(i=0;i<11250;++i)
        brkc_.coef[i]=parent.brkc_.coef[i];
    for(i=0;i<350;++i)
        brkc_.tauc[i]=parent.brkc_.tauc[i];
    for(i=0;i<100;++i)
        brkc_.xc[i]=parent.brkc_.xc[i];
    for(i=0;i<1000;++i)
        brkc_.tcoef[i]=parent.brkc_.tcoef[i];
    for(i=0;i<200;++i)
        brkc_.tp[i]=parent.brkc_.tp[i];
    brkc_.odep=parent.brkc_.odep;
    for(i=0;i<90;++i)
        brkc_.fcs[i]=parent.brkc_.fcs[i];
    brkc_.nin=parent.brkc_.nin;
    brkc_.nph0=parent.brkc_.nph0;
    brkc_.ki=parent.brkc_.ki;
    brkc_.nseg=parent.brkc_.nseg;
    brkc_.nbrn=parent.brkc_.nbrn;
    for(i=0;i<2;++i)
    {
        brkc_.int0[i]=parent.brkc_.int0[i];
        brkc_.msrc[i]=parent.brkc_.msrc[i];
        brkc_.isrc[i]=parent.brkc_.isrc[i];
        brkc_.ku[i]=parent.brkc_.ku[i];
        brkc_.km[i]=parent.brkc_.km[i];
    }
    for(i=0;i<90;++i)
        brkc_.nafl[i]=parent.brkc_.nafl[i];
    for(i=0;i<60;++i)
        brkc_.indx[i]=parent.brkc_.indx[i];
    for(i=0;i<60;++i)
        brkc_.kndx[i]=parent.brkc_.kndx[i];
    for(i=0;i<30;++i)
        brkc_.iidx[i]=parent.brkc_.iidx[i];
    for(i=0;i<100;++i)
        brkc_.jidx[i]=parent.brkc_.jidx[i];
    for(i=0;i<30;++i)
        brkc_.kk[i]=parent.brkc_.kk[i];
    /* Pcdc struct - only a single char * */
    strcpy(pcdc_.phcd,parent.pcdc_.phcd);
    /* Prtflc struct  copy */
    for(i=0;i<30;++i)
        prtflc_.segmsk[i]=parent.prtflc_.segmsk[i];
    for(i=0;i<2;++i)
        prtflc_.prnt[i]=parent.prtflc_.prnt[i];
    /* Now copy Umdc struct */
    for(i=0;i<300;++i)
    {
        umdc_.pm[i]=parent.umdc_.pm[i];
        umdc_.zm[i]=parent.umdc_.zm[i];
        umdc_.ndex[i]=parent.umdc_.ndex[i];
    }
    for(i=0;i<2;++i)
        umdc_.mt[i]=parent.umdc_.mt[i];
    /* Not copy Tabc struct */
    for(i=0;i<2;++i)
        tabc_.us[i]=parent.tabc_.us[i];
    for(i=0;i<2250;++i)
        tabc_.pt[i]=parent.tabc_.pt[i];
    for(i=0;i<9000;++i)
        tabc_.tau[i]=parent.tabc_.tau[i];
    for(i=0;i<4500;++i)
        tabc_.xlim[i]=parent.tabc_.xlim[i];
    for(i=0;i<300;++i)
        tabc_.xbrn[i]=parent.tabc_.xbrn[i];
    for(i=0;i<200;++i)
        tabc_.dbrn[i]=parent.tabc_.dbrn[i];
    tabc_.xn=parent.tabc_.xn;
    tabc_.pn=parent.tabc_.pn;
    tabc_.tn=parent.tabc_.tn;
    tabc_.dn=parent.tabc_.dn;
    tabc_.hn=parent.tabc_.hn;
    for(i=0;i<2;++i)
        tabc_.jndx[i]=parent.tabc_.jndx[i];
    for(i=0;i<2;++i)
        tabc_.idel[i]=parent.tabc_.idel[i];
    for(i=0;i<2;++i)
        tabc_.us[i]=parent.tabc_.us[i];
    tabc_.mbr1=parent.tabc_.mbr1;
    tabc_.mbr2=parent.tabc_.mbr2;
    /* Finally copy Pdec struct  data */
    for(i=0;i<10;++i)
    {
        pdec_.ua[i]=parent.pdec_.ua[i];
        pdec_.taua[i]=parent.pdec_.taua[i];
    }
    pdec_.deplim=parent.pdec_.deplim;
    pdec_.ka=parent.pdec_.ka;
}
TauPCalculator::TauPCalculator(const TauPCalculator& parent)
{
    model=parent.model;
    last_phase_code_used=parent.last_phase_code_used;
    depth=parent.depth;
    maxphases=parent.maxphases;
    ttopen=parent.ttopen;
    nn=parent.nn;
    usrc[0]=parent.usrc[0];
    usrc[1]=parent.usrc[1];
    int i,j;
    for(i=0;i<parent.nn;++i)
    {
        /* These probably do not really need to be copied */
        tt[i]=parent.tt[i];
        dtdd[i]=parent.dtdd[i];
        dtdh[i]=parent.dtdh[i];
        dddp[i]=parent.dddp[i];
        /* not sure these are null terminated so copy the entire
           block */
        for(j=0;j<SIZE_PHCD;++j)
        {
            phcd[i][j]=parent.phcd[i][j];
        }
    }
    strncpy(tblpath,parent.tblpath,MAXPATHLEN);
    /* This private method copies all the obnoxious variables created
       from the original fortran common blocks.   Made a private 
       method to allow it's use in operator= to avoid duplicate code.
       Also useful to encapsulate this in one place to make it easier
       to maintain. */
    this->copy_f2c_common_block_data(parent);
}
TauPCalculator& TauPCalculator::operator=(const TauPCalculator& parent)
{
    if(this!=&parent)
    {
        model=parent.model;
        last_phase_code_used=parent.last_phase_code_used;
        depth=parent.depth;
        maxphases=parent.maxphases;
        ttopen=parent.ttopen;
        nn=parent.nn;
        usrc[0]=parent.usrc[0];
        usrc[1]=parent.usrc[1];
        int i,j;
        for(i=0;i<parent.nn;++i)
        {
            /* These probably do not really need to be copied */
            tt[i]=parent.tt[i];
            dtdd[i]=parent.dtdd[i];
            dtdh[i]=parent.dtdh[i];
            dddp[i]=parent.dddp[i];
            /* not sure these are null terminated so copy the entire
               block */
            for(j=0;j<SIZE_PHCD;++j)
            {
                phcd[i][j]=parent.phcd[i][j];
            }
        }
        strncpy(tblpath,parent.tblpath,MAXPATHLEN);
        this->copy_f2c_common_block_data(parent);
    }
    return(*this);
}
TauPCalculator::~TauPCalculator()
{
    /* This is a placeholder that does nothing for now.   Not clear
       at this point how file open and close is handled with 
       table reader - think it reads and closes but may need that
       here.   Pretty sure we need to unmap and close the ttables */
}


double TauPCalculator::Ptime(double rdelta, double evdepth)
{
    const string base_error("TauPCalculator::Ptime:  ");
    /* taup wants delta in degrees we use radians */
    double delta=deg(rdelta);
    double arrival ; 
    char **phases ; 
    double *times ; 
    int i, nphases ; 
    try {
        if(last_phase_code_used != "P")
        {
            //DEBUG
            i=tt_taup_set_phases("ALL");
            //i=tt_taup_set_phases("P");
            if(i!=0)
                throw SeisppError(base_error
                        + "tt_taup_set_phases failed defining P phase");
            last_phase_code_used=string("P");
        }

	if(evdepth!=depth)
	{
	    depth=evdepth;
	    tt_taup_set_event_depth (depth) ; 
	}
        vector<TauPComputedTime>ttall=tt_taup(delta);
        /* Hunt for the result tagged pure P.   For now we throw an
           error if that isn't found */
        //DEBUG
        cout << "Source depth="<<depth<<" epicentral distance="<<delta<<endl;
        vector<TauPComputedTime>::iterator ttptr;
        for(ttptr=ttall.begin();ttptr!=ttall.end();++ttptr)
        {
            //DEBUG
            cout << "phase="<<ttptr->phase<<" time="<<ttptr->ttime<<endl;
            if(ttptr->phase == "P") return ttptr->ttime;
        }
        /* if we land here a simple P was not found.  For now
        use same code as phasetime - expectation is we will need 
        additional error recovery here once I do an empirical test
        of what the taup calculator actually does in this situation */
        stringstream sserr;
        sserr << base_error
            << "tt_taup did not return anything tagged as simple P"<<endl
            << "List of returned values"<<endl;
        for(ttptr=ttall.begin();ttptr!=ttall.end();++ttptr)
            sserr <<ttptr->phase<<" "<<ttptr->ttime<<endl;
        throw SeisppError(sserr.str());
    }catch(...){throw;};

}
double TauPCalculator::Stime(double delta, double depth)
{
    try {
	double arrival;
	/* delta in here is radians because phasetime will convert to degrees*/
	arrival=this->phasetime(delta,depth,"S");
	return(arrival);
    }catch(...){throw;};
}
double TauPCalculator::phasetime(double rdelta, double edepth, const char *phase)
{
    const string base_error("TauPCalculator::phasetime:  ");
    /* taup wants delta in degrees we use radians */
    double delta=deg(rdelta);
    double arrival ; 
    char **phases ; 
    double *times ; 
    int nphases ; 
    int i ; 
    try {
        if(last_phase_code_used != phase)
        {
            tt_taup_set_phases(phase);
            tt_taup_set_event_depth(edepth);
        }
        else if(edepth!=this->depth)
            tt_taup_set_event_depth(edepth);
        vector<TauPComputedTime> ttall;
        ttall=tt_taup(delta);
        vector<TauPComputedTime>::iterator ttptr;
        int n=ttall.size();
        /* Intentionall do not trap a zero n.   Reason is that 
           tt_taup throws an error in this condition.   This is
           a maintenance issue, however, so be careful if this
           changes. 
         
           For efficiency if n is one we simply assume the
           phase code matches*/
        if(n==1)
            return(ttall[0].ttime);
        else
        {
            vector<TauPComputedTime>::iterator ttptr;
            for(ttptr=ttall.begin();ttptr!=ttall.end();++ttptr)
            {
                if(ttptr->phase == phase)
                    return(ttptr->ttime);
            }
        }
        /* For now throw and error if we land here.   This may require
           some recovery code as it how this works with multiple phases
           is not clear to me as I'm composing this rewrite. */
        stringstream sserr;
        sserr << base_error
            << "tt_taup did not return requested phase = "<<phase<<endl
            << "List of returned values"<<endl;
        for(ttptr=ttall.begin();ttptr!=ttall.end();++ttptr)
            sserr <<ttptr->phase<<" "<<ttptr->ttime<<endl;
        throw SeisppError(sserr.str());
    }catch(...){throw;};
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
        /* Note conversion to degrees necessary here.   I use radians universall
           but ddistance assumes degrees */
        distance=ddistance(deg(source.lat),deg(source.lon),
                deg(receiver.lat),deg(receiver.lon));
	double depth;
	depth=r0_ellipse(source.lat)-source.r;
	return(this->phasetime(distance,depth,phase));	
}
double TauPCalculator::Ptime(Geographic_point& s,Geographic_point& r)
{
	double distance;
        /* Note conversion to degrees necessary here.   I use radians universall
           but ddistance assumes degrees */
        distance=ddistance(deg(s.lat),deg(s.lon),deg(r.lat),deg(r.lon));
	double depth;
	depth=r0_ellipse(s.lat)-s.r;
	return(this->Ptime(distance,depth));	
}
double TauPCalculator::Stime(Geographic_point& source,Geographic_point& receiver)
{
	double distance;
        /* Note conversion to degrees necessary here.   I use radians universall
           but ddistance assumes degrees */
        distance=ddistance(deg(source.lat),deg(source.lon),
                deg(receiver.lat),deg(receiver.lon));
	double depth;
	depth=r0_ellipse(source.lat)-source.r;
	return(this->Stime(distance,depth));	
}

SlownessVector TauPCalculator::phaseslow(Geographic_point& s,
		Geographic_point& r,const char *phase)
{
    double distance,baz,azimuth;
    dist(r.lat,r.lon,s.lat,s.lon,&distance,&baz);
    /* dist returns a back azimuth but for a slowness vector we need 
    direction of propagation. */
    azimuth=baz+M_PI;
    if(azimuth>(2.0*M_PI)) azimuth -= (2.0*M_PI);
    try{
        double z=r0_ellipse(s.lat) - s.r;
    	return(this->phaseslow(distance,azimuth,z,phase));
    } catch(...){throw;};
}
SlownessVector TauPCalculator::phaseslow(double distance,double azimuth,
		double edepth, const char *phase)
{
    const string base_error("TauPCalculator::phaseslow:  ");
    double delta=deg(distance);
    int nphases ; 
    int i ; 
    try {
        if(last_phase_code_used != phase)
        {
            tt_taup_set_phases(phase);
            tt_taup_set_event_depth(edepth);
        }
        else if(edepth!=this->depth)
        {
            tt_taup_set_event_depth(edepth);
        }
        /* In phasetime this call is behind an interface function.   here
           I chose to do this all in one and have Pslow and Sslow just call
           this function */
        float fdel=(float)delta;
	trtm_ (&fdel, &maxphases, &nphases, &(this->tt[0]),&(this->dtdd[0]),
			&(this->dtdh[0]), &(this->dddp[0]), 
                        (char *)&(this->phcd[0]),SIZE_PHCD);
        /* Hunt for the requested phase.  For now throw and error if an exact
           match is not found.  As noted elsewhere this will probably require some
           modification after I find out how Bulland's code deals with ambiguous
           phase codes */
        double umag;  
        bool found(false);
        for(i=0;i<nphases;++i)
        {
            if(!strcmp(phase,phcd[i]))
            {
                umag=dddp[i];
                found = true;
                break;
            }
        }
        if(found)
        {
            umag *= EQUATORIAL_EARTH_RADIUS;
            return(SlownessVector(umag*sin(azimuth),umag*cos(azimuth)));
        }
        else
        {
            stringstream sserr;
            sserr << base_error
                << "taup calculator did not return a result for phase="
                << phase<<endl
                << "List of phase names and slowness values:"<<endl;
            for(i=0;i<nphases;++i)
                sserr <<phcd[i]<<" "<<dddp[i]*EQUATORIAL_EARTH_RADIUS<<endl;
            throw SeisppError(sserr.str());
        }
    }catch(...){throw;};
}
SlownessVector TauPCalculator::Pslow(Geographic_point& source,Geographic_point& receiver)
{
    try {
        SlownessVector result(this->phaseslow(source,receiver,"P"));
        return result;
    }catch(...){throw;};
}
SlownessVector TauPCalculator::Sslow(Geographic_point& source,Geographic_point& receiver)
{
    try {
        SlownessVector result(this->phaseslow(source,receiver,"S"));
        return result;
    }catch(...){throw;};
}
SlownessVector TauPCalculator::Pslow(double rdelta,double az,double z)
{
    try {
        SlownessVector result(this->phaseslow(rdelta,az,z,"P"));
        return result;
    }catch(...){throw;};
}
SlownessVector TauPCalculator::Sslow(double rdelta,double az,double z)
{
    try {
        SlownessVector result(this->phaseslow(rdelta,az,z,"S"));
        return result;
    }catch(...){throw;};
}

dvector TauPCalculator::TimePhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)
{
    throw SeisppError("TauPCalculator::TimePhaseDerivatives has not been implemented");
}
dmatrix TauPCalculator::SlownessPhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)
{
    throw SeisppError("TauPCalculator::SlownessPhaseDerivatives has not been implemented");
}


/* Note "table" in this context is actually a model specification.
   e.g. iasp91 is the default */
void TauPCalculator::tt_taup_set_table(const char *table_path)
{
    string base_error("TauPCalculator::tt_taup_set_table:  ");
    char fname[MAXPATHLEN], *tablebase, tablename[MAXPATHLEN];

    // test for absolute path 
    if ( (strlen(table_path) >  0) && (*table_path == '/' )) 
	strcpy ( tblpath, table_path ) ; 
    else {
        /* This was the original test at the to of this block 
	if ( table_path != 0 && *table_path != 0 ) 
        */
        if(table_path!=NULL)
        {
            if(strlen(table_path)>0)
                tablebase = strdup(table_path);
        }
        else if(strlen(table_path)>0)
            tablebase = strdup(table_path);
	else if ( ( tablebase= getenv("TAUP_TABLE")) == NULL ) 
	    tablebase = strdup(TBL_TBL) ; 
	if ( *tablebase != '/' ) 
	    strcpy ( tablename, TBL_DIR ) ; 
	strcat ( tablename, tablebase ) ; 
	strcat ( tablename, ".hed" ) ; 
        free(tablebase);
        char *pathtmp;
	pathtmp = datafile ( "TAUP_PATH", tablename ) ;
        /* Note tblpath is part of the object so is saved here */
	if ( pathtmp != NULL ) {
	    strcpy ( tblpath, pathtmp ) ; 
	    tblpath[strlen(tblpath)-4] = 0 ; 
	    free(pathtmp) ; 
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
        --note I change the name to last_phase_code_used for clarity--
       */

	char *phase;
	int k;
        int one(1);   // obnoxious FORTRAN requirement from pass by reference only
        /* initialize last_phase_code_used = phases */
        last_phase_code_used=string(phases);

        /* Input phases array is a comma separated list.  This section 
         breaks these tokens up and puts them in the phcd array of char*/
        string phtmp(phases);
        phtmp=phtmp+",";
        /* This is needed because strtok alters the contents of 
           its arg and string objects are immutable */
        char *phass;
        phass=strdup(phtmp.c_str());
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		//if (!phase[0]) continue;
		if (phase[0]=='\0') continue;
		strcpy (&phcd[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) phcd[nn][k] = ' ';
		nn++;
	}
        free(phass);
        try {
            tt_taup_set_table(model);
	} catch(SeisppError& serr){throw serr;};
	if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_setup:  ")
			+ "travel time table path variable was not initialized");

	tabin_ (&one, tblpath ) ; 
        /* Unclear if this is the right syntax, but I think this will work 
           in looking at the way f2c translated this.   Assumes phcd is
           a fortran style matrix with char values. This passes a pointer to
           the first element of that array and the fortran routine will assume
           it expands to SIZE_PHCD X MAXPHASES]  */
	brnset_ (&nn, phcd[0], prnt, SIZE_PHCD);
	ttopen = true;
        this->print_private();
}


/* This method returns 0 for success, nonzero means an error */
int TauPCalculator::tt_taup_set_phases(string phases)
{
        /*
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
        int one(1);
        // Return immediately if the desired phase hasn't changed
        if(last_phase_code_used==phases) return 0;
        /* Input phases array is a comma separated list.  This section 
         breaks these tokens up and puts them in the phcd array of char*/
        string phtmp(phases);
        phtmp=phtmp+",";
        /* This is needed because strtok alters the contents of 
           its arg and string objects are immutable */
        char *phass;
        phass=strdup(phtmp.c_str());
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		if (phase[0]=='\0') continue;
		strcpy (&phcd[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) phcd[nn][k] = ' ';
		nn++;
	}
        free(phass);
        /* Indicate failure if the tblpath variable has not been set */
        if(strlen(tblpath)<=0)
        {
            tt_taup_set_table(NULL);
            return 1;
        }

	//tabin_ (&one, tblpath) ;
	brnset_ (&nn, phcd[0], prnt, SIZE_PHCD);
	ttopen = true;
	return (0);
}


void TauPCalculator::tt_taup_set_event_depth(double edepth)
{
	/*
	static float depth;
	static float usrc[2];
	*/
        if(!ttopen) 
            throw SeisppError(string("TauPCalculator::tt_taup_set_event_depth:  ")
                + "usage error.   Calling set depth before initialization not allowed");

	depth = edepth;
	if (depth < 0.001) depth = 0.001;
        float fdepth=(float)(depth);
	depset_ (&fdepth, usrc);
}
/* Warning this function assumes del is degrees to mate with Bulland's 
   code */
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
        const string base_error("TauPCalculator::tt_taup:  ");
        if(!ttopen)
            throw SeisppError(base_error
                    + "Object has not been initialized - check source code");
	int i, k, l;
	int nphases;   // static above, but does not seem necessary 
        float fdel=(float)del;
	trtm_ (&fdel, &maxphases, &nphases, &(this->tt[0]),&(this->dtdd[0]),
		&(this->dtdh[0]), &(this->dddp[0]),
		(char *) &(this->phcd[0]),SIZE_PHCD);
	/* Maintenance issue warning:  callers of this private method 
           can assume this error is trapped and must have a hander.
           For efficiency they do not repeat the test, but be careful if
           this safety is removed */
	if(nphases <= 0) 
	   throw SeisppError(base_error 
		+ "taup trtm function returned no results from phase list "
		+ last_phase_code_used);
	vector<TauPComputedTime> result;
	result.reserve(nphases);
	TauPComputedTime ttk;
	for(k=0;k<nphases;++k)
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
/* This is a debug routine */
void TauPCalculator::print_private()
{
    cout << "model="<<model<<endl;
    cout << "last_phase_code_used="<<last_phase_code_used<<endl;
    cout << "depth="<<depth<<endl;
    cout << "maxphases="<<maxphases<<endl;
    cout << "tt dtdd dtdh dddp"<<endl;
    int i;
    for(i=0;i<MAXPHASES;++i)
    {
        cout << tt[i]<<" "
            << dtdd[i]<<" "
            << dtdh[i]<<" "
            << dddp[i]<<endl;
    }
    cout << "phcd char matrix"<<endl;
    for(i=0;i<MAXPHASES;++i)
    {
        cout << phcd[i]<<endl;
    }
    cout << "tblpath="<<tblpath<<endl;
    cout << "ttopen="<<ttopen<<endl;
    cout << "nn="<<nn<<endl;
    cout << "prnt_0="<<prnt[0]<<endl;
    cout << "prnt_1="<<prnt[1]<<endl;
    cout << "prnt_2="<<prnt[2]<<endl;
    cout << "usrc_0="<<usrc[0]<<endl;
    cout << "usrc_1="<<usrc[1]<<endl;
}

}  // END PWMIG Namespace encapsulation
