#ifndef _PWMIG_TAUPCALCULATOR_H_
#define _PWMIG_TAUPCALCULATOR_H_
#include <vector>
#include <string>
#include "TravelTimeCalculator.h"
/* Formerly in _taup.h - not sure all of these are necessary */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
/* Size definitions in f2c of subs.f - dsap and brtt versions had these in a
different include file */
#define MAXPHASES 60
#define SIZE_PHCD 16
#define MAXPATHLEN 512

namespace PWMIG
{
using namespace std;
using namespace SEISPP;
using namespace PWMIG;
/* These structs were originally in _taup.h. They are translations of 
common block variables from f2c of the old Bulland code (subs.f).   
They live in the private area of this object */
struct Brkc {
    double          zs,
                    pk[30],
                    pu[702] /* was [351][2] */ ,
                    pux[200] /* was [100][2] */ ,
                    tauu[700] /* was [350][2] */ ,
                    xu[200] /* was [100][2] */ ,
                    px[200] /* was [100][2] */ ,
                    xt[200] /* was [100][2] */ ,
                    taut[2250],
                    coef[11250] /* was [5][2250] */ ,
                    tauc[350],
                    xc[100],
                    tcoef[1000] /* was [5][100][2] */ ,
                    tp[200]
     /* was [100][2] */ ;
    float           odep,
                    fcs[90] /* was [30][3] */ ;
    int             nin,
                    nph0,
                    int0[2],
                    ki,
                    msrc[2],
                    isrc[2],
                    nseg,
                    nbrn,
                    ku[2],
                    km[2]
                   ,
                    nafl[90] /* was [30][3] */ ,
                    indx[60] /* was [30][2] */ ,
                    kndx[60] /* was [30][2] */ ,
                    iidx[30],
                    jidx[100],
                    kk[30];
};

struct Pcdc {
    char            phcd[800];
};

struct Prtflc {
    int             segmsk[30],
                    prnt[2];
};

struct Umdc {
    double          pm[300] /* was [150][2] */ ,
                    zm[300] /* was [150][2] */ ;
    int             ndex[300] /* was [150][2] */ ,
                    mt[2];
};

struct Tabc {
    double          us[2],
                    pt[2250],
                    tau[9000] /* was [4][2250] */ ,
                    xlim[4500] /* was [2][2250] */ ,
                    xbrn[300] /* was [100][3] */ ,
                    dbrn[200] /* was [100][2] */ ;
    float           xn,
                    pn,
                    tn,
                    dn,
                    hn;
    int             jndx[200] /* was [100][2] */ ,
                    idel[300] /* was [100][3] */ ,
                    mbr1,
                    mbr2;
};

struct Pdec {
    double          ua[10] /* was [5][2] */ ,
                    taua[10] /* was [5][2] */ ;
    float           deplim;
    int             ka;
};
/* \brief Multiple phase travel time structure.

This struct is a more rational way to return data from the primary
function here called tt_taup.    Bullands taup calculator computes a fortran
array results for multiple phases in a single call.  tt_taup now 
internally returns a vector of this struct. */
class TauPComputedTime {
public:
	double ttime;
	string phase;
};

/*! \brief Travel time calculator using Bulland's Tau-P calculator.

This is a C++ interface to Ray Bulland's travel time calculator as
modified originally in the Datascope library and supported for years
in the Antelope package.  This is object is essentially a set of 
wrapper that simply use of the Tau-P calculators as an alternative
to the ttimes approach used in Antelope.   This object, however, was
produced because of a need to remove Antelope dependencies so 
the Antelope issue is mainly an ancestory issue.

This object is a concrete implementation of pure virtual base
class TravelTimeCalculator object.  The intent is to allow the
travel time engine to take advantage of polymorphism.  This is an alterative
to the ttimes approach in Antelope that has lot of very awkward constructs,
at least in the authors opinion.
\author Gary L. Pavlis
*/
class TauPCalculator : public TravelTimeCalculator
{
public:
    /*! Default constructor.   
      
      Assumes model is iasp91.
     
     \throw SeisppError if the constructor files.
     */

    TauPCalculator();
    /*! \brief Constructor specifying alternative model.

      This implmentation uses a descendent of Ray Bulland's 
      ancient fortran taup calculator as found in the old open
      source datascope package that is an ancestory to antelope.
      That algorithm uses precomputed tables that define a 
      model name.  The default is iasp91, but it is possible to
      compute tables for other models (best done with antelope
      programs and transferring the files to the machine where
      this object is to be used).  Point is the model ultimate
      must link to a set of files the constructor looks for 
      based on the PWMIG implementation of the datafile procedure
      in dsap stock (see man datafile(3)).

      \param model - alternative model name to default iasp91

      \throw - SeisppError if the constructor files */

    TauPCalculator(const char *model);
    /*! Standard copy constructor. */
    TauPCalculator(const TauPCalculator& parent);
    ~TauPCalculator();
    /*! \brief Compute travel time for a specified phase.

      This computes a travel time in seconds between specified source
      and receiver positions on the earth.   

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 

      \return travel time in seconds of phase
      */
    double phasetime(Geographic_point& source, 
		Geographic_point& receiver,const char *phase);
    /*! Simplified version of phasetime with phase set as P*/
    double Ptime(Geographic_point& source,Geographic_point& receiver);
    /*! Simplified version of phasetime with phase set as S*/
    double Stime(Geographic_point& source,Geographic_point& receiver);
    /*! \brief Compute P time given only epicentral distance.

      The Tau-P method assumes a radially symmetric earth model 
      so the result depends only on distance and depth. This 
      method is useful to avoid overhead of unnecessary great
      circle path calculations that might appear in some cases 
      otherwise (e.g. computing a travel time curve).   

      Note the Tau-P calculator always assumes the datum for the
      receiver is sea level so there source elevation is not 
      an argument.

      \param delta - epicentral distance in radians
      \param depth - source depth in km.   

      \return travel time in seconds.
      */
    double Ptime(double delta, double depth);
    /*! Comparable to Ptime with same signature for S. */
    double Stime(double delta, double depth);
    /*! Comparable to Ptime with same signature for variable phase. 
      \param delta - epicentral distance in radians
      \param depth - source depth in km.   
      \param phase - seismic phase for which time is to be computed.

      \return travel time in seconds.
     */
    double phasetime(double delta, double depth, const char *phase);
    /*! \brief Compute slowness vector for a specified phase.

      In seismology a phase is an impulsive arrival with a well defined
      arrival time.   A calculator needs to be able to calculate the
      predicted slowness vector of at least one phase.  A slowness 
      vector is defined by the SEISPP::SlownessVector object 
      (see that documentation for the definition and assumed units.)

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 
      \return predicted slowness vector for the specified phase.
      */
    SlownessVector phaseslow(Geographic_point& source,
		Geographic_point& receiver,const char *phase);
    /*! \brief Simplified slowness vector calculation for 1D model.

      The taup calculator requires a radially symmetric earth model.  Hence
      the extra parameters that describe both source and receiver coordinates
      are not necessary.  Frequently we have already computed epicentral 
      distance and here is no reason for the extra overhead of computing it
      again inside the fully parameterized version of this method.  In 
      fact, in this implementation the fully parameterized version computes
      distance and calls this method anyway.

      \param rdelta - epicentral distance (in radians)
      \param azimuth - great circle path computed propagation azimuth(radians)
      \param z_source - source depth in km.
      \param phase defines the seismological name of the required phase. 
      
      \return predicted slowness vector for the specified phase.
      */
    SlownessVector phaseslow(double rdelta, double azimuth,
    	double z_source,const char *phase);
    /*! Simplified phaseslow with phase set to P. */
    SlownessVector Pslow(Geographic_point& source,Geographic_point& receiver);
    /*! P slowness calculation giving only distance  and depth.
    
      \param rdelta - epicentral distance (in radians)
      \param azimuth - great circle path computed propagation azimuth(radians)
      \param z_source - source depth in km.
      
      \return predicted slowness vector first arrival P phase.    
     */
    SlownessVector Sslow(double rdelta, double azimuth, double z_source);
    /*! S slowness calculation giving only distance  and depth.
    
      \param rdelta - epicentral distance (in radians)
      \param azimuth - great circle path computed propagation azimuth(radians)
      \param z_source - source depth in km.
      
      \return predicted slowness vector of earliest S phase.    
     */
    SlownessVector Pslow(double rdelta, double azimuth, double z_source);
    /*! Simplified phaseslow with phase set to S. */
    SlownessVector Sslow(Geographic_point& source,Geographic_point& receiver);
    

    /*! \brief Compute travel time derivatives for a specified phase.

      In seismology a phase is an impulsive arrival with a well defined
      arrival time.   A calculator needs to be able to calculate the
      predicted time of at least one phase.   This method is a 
     complementary method that calculates partial derivatives.   

      This is a pure virtual method that must be implemented by any
      child of this base class.

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 

      \return 4-vector of partial derivatives of time wrt to each source
      coordinate.  The interface does not need to specify details of 
      order or even how the source coordinate is defined.
      */
    dvector TimePhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase);
    /*! \brief Compute travel time for a specified phase.

      In seismology a phase is an impulsive arrival with a well defined
      arrival time.   A calculator needs to be able to calculate the
      predicted slowness of at least one phase. 

      This method should return partial derivatives of a computed 
      slowness vector wrt to source coordinates.   

      This is a pure virtual method that must be implemented by any
      child of this base class.

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 

      \return 2X3 matrix of slowness vector partial derivatives. 
            The interface is not dogmatic about how these are defined
            in detail, but it does require that the rows of the matrix 
            are for the two parameters defining the slowness vector and 
            the columns are derivatives wrt to three space coordinates.
            (Note slowness vectors have no dependence upon origin time.)
      */
    dmatrix SlownessPhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase);
    /*! Assignment operator */
    TauPCalculator& operator=(const TauPCalculator& parent);
private:
    /* The following two attributes are stored in the "Hook" in antelope
     as char * variables*/
    string model;
    string last_phase_code_used;   
    /* Cached in object - depth change initates action */
    double depth; 
    /* These are work arrays used by taup interface routines */
    int maxphases;
    float tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
    char phcd[MAXPHASES][SIZE_PHCD];
    char tblpath[MAXPATHLEN];
    /* More that were static variables in the taup_setup function.  
   Here much better to make them part of private area of the object */
    bool ttopen;
    int prnt[3];
    /* This variable holds current number of phases */
    int nn;
    /* a mystery static variable passed to depset fortran subroutine */
    float usrc[2];
    /* These were originally static external variables in the _taup.h include
    file.    We place then in the object here to allow assimilation of the 
    f2c code as private members of this object. This is the data read from
    tables*/
	struct Brkc brkc_;
	struct Pcdc pcdc_;
	struct Prtflc prtflc_;
	struct Umdc umdc_;
	struct Tabc tabc_;
	struct Pdec pdec_;
	/* These are table size parameters loaded by tabin_.   */
	char    *tbl_data;
	int      tbl_data_size;
	int      tbl_maxrec;
	int      tbl_record_len;
	/* END of this section of the abomination.*/
	   
    /* used in all constructors */
    void initialize_Taup(string mod);
    /* This is called by copy constructor and operator = to copy the fortran 
    common block data (next abomination below) from parent to copy */
    void copy_f2c_common_block_data(const TauPCalculator& parent);
    /* methods converted from tt_taup.c */
    void tt_taup_set_table(const char *table_path);
    int tt_taup_set_phases(string phases);
    /* an ugly routine that returns modname */
    string taup_get_modname();
    void taup_setup(const char *model, const char *phases);
    void tt_taup_set_event_depth(double edepth);
    /* This is a debug routine - probably should be deleted when working */
    void print_private();
    vector<TauPComputedTime> tt_taup(double del);
    /* These began life as f2c code from taup library.   An abomination, but
    here they are made members of this object.
    This first group of procedures are referred to in TauPCalculator.cc code public
    methods.*/
	void tabin_(int *,char *);
	int brnset_(int *,char *,int *,int);
	int depset_(float *,float *);
	int trtm_(float *delta,int *mxphs,int *nphs, float *tt,
        float *dtdd,float *dtdh,float *dddp,char *phnm,int phnm_len); 
    /* These are procedures in the f2c conversion for subs.f that need access
    to fortran common variables, but which are not explicitly referred to in
    the C++ interface code.   Underscores are retained here to avoid hunting for
    multiple occurences. */
    int bkin_ (int *lu, int *nrec, int *len, double *buf);
    double umod_0_(int n__,double *zs,int *isrc,int *nph,double *uend,int *js);
    /* These two functions are wrappers to umod_0 created by original two entry
    points to original fortran function*/
    double umod_(double *zs,int *isrc,int *nph);
    double zmod_(double *zs,int *isrc,int *nph);
    int depcor_(int *nph);
    int pdecu_(int *i1, int *i2, double *x0, double *x1, double *xmin,
								int *int__, int *len);
	int spfit_(int *jb, int *int__);
	int findtt_(int *jb, double *x0, int *max__, int *n, float *tt, float *dtdd,
        float *dtdh, float *dddp, char *phnm, int phnm_len); 
};

} // End PWMIG namespace encapsulation
#endif
