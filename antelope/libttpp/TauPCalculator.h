#ifndef _PWMIG_TAUPCALCULATOR_H_
#define _PWMIG_TAUPCALCULATOR_H_
#include <vector>
#include <string>
#include "TravelTimeCalculator.h"
#define MAXPHASES 60
#define SIZE_PHCD 16
#define MAXPATHLEN 512
namespace PWMIG
{
using namespace std;
using namespace SEISPP;
using namespace PWMIG;
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
private:
    /* The following two attributes are stored in the "Hook" in antelope
     as char * variables*/
    string model;
    string last_phase_code_used;   
    /* Cached in object - depth change initates action */
    double depth; 
    /* These are work arrays used by taup routines */
    int maxphases;
    float tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
    char phcd[MAXPHASES][SIZE_PHCD];
    char tblpath[MAXPATHLEN];
    /* More that were static variables in the taup_setup function.  
   Here much better to make them part of private area of the object */
    int ttopen;
    int prnt[3];
    int nn;
    char pcntl[MAXPHASES][SIZE_PHCD];
    char phass[1024];
    char old_phases[1024];
    /* a mystery static variable passed to depset fortran subroutine */
    float usrc[2];
    /* used in all constructors */
    void initialize_Taup(string mod);
    /* methods converted from tt_taup.c */
    void tt_taup_set_table(const char *table_path);
    int tt_taup_set_phases(string phases);
    /* an ugly routine that returns modname */
    string taup_get_modname();
    void taup_setup(const char *model, const char *phases);
    void tt_taup_set_event_depth(double edepth);
    vector<TauPComputedTime> tt_taup(double del);
};
extern "C" {
void tabin_(int *,char *);
void brnset_(int *,char *,int *,int);
void depset_(float *,float *);
}


} // End PWMIG namespace encapsulation
#endif
