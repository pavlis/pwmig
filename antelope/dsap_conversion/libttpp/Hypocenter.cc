// These are methods for the Hypocenter object to compute common standard things.`
// These are mostly interface routines to Antelopes coords library.
// Note all returns are distances and angles are returned in units of radians.  

#include "coords.h"
#include "stock.h"
#include "TauPCalculator.h"
#include "Hypocenter.h"
using namespace std;
using namespace PWMIG;
namespace PWMIG
{

// copy constructor and = operators commonly are implicitly needed
Hypocenter::Hypocenter(const Hypocenter& h0) : ttcalc(h0.ttcalc)
{
	lat = h0.lat;
	lon = h0.lon;
	z = h0.z;
	time = h0.time;
	method = h0.method;
	model = h0.model;
}
Hypocenter& Hypocenter::operator=(const Hypocenter& h0)
{
    if(this!=&h0)
    {
	lat = h0.lat;
	lon = h0.lon;
	z = h0.z;
	time = h0.time;
	method = h0.method;
	model = h0.model;
	ttcalc=h0.ttcalc;
    }
    return(*this);
}
// This constructor creates a Hypocenter object using parameters
// read from a Metadata object.  It would be useful to have a 
// mechanism to not have a frozen set of names used to fetch
// the required parameters but I'm not sure how to do that
// without making the interface clumsy. 
//
// Note that things like TimeSeries objects can be used to create
// a Hypocenter through this mechanism through the use of a dynamic_cast
//
Hypocenter::Hypocenter(Metadata& md)
{
	try {
		lat=md.get_double("source_lat");
		lon=md.get_double("source_lon");
		z=md.get_double("source_depth");
		time=md.get_double("source_time");
	} catch (MetadataError& mderr) {throw mderr;};
	// We run a separate try block here and recover from
	// model and method not being defined -- a common thing
	// we will probably need
	//
	//method=string("tttaup");
	model = string("iasp91");
	try {
		ttcalc=TauPCalculator(model.c_str());
	} catch (...){throw;};
}
Hypocenter::Hypocenter(double lat0, double lon0, double z0, double t0,
		string meth0, string mod0) : ttcalc(mod0.c_str())
{
	lat=lat0;
	lon=lon0;
	z=z0;
	time=t0;
	/* Dummy kept in the interface for consistency.   Write
	a warning error if it is anything but tttaup */
	if(meth0!="tttaup") 
	   cerr << "Hypocenter constuctor:   ignoring method argument "
		<< meth0<<endl
		<< "This implementation only supports the tttaup "
		<< "implemented in the TauPCalculator C++ object"<<endl;
	model=mod0;
        /*
	try {
	    ttcalc=TauPCalculator(mod0.c_str());
	} catch(...){throw;};
        */
}
		


double Hypocenter::distance(double lat0, double lon0)
{
	double epidist, az;

	dist(lat,lon,lat0,lon0,&epidist, &az);
	return(epidist);
}

double Hypocenter::esaz(double lat0, double lon0)
{
	double epidist, az;

	dist(lat,lon,lat0,lon0,&epidist, &az);
	return(az);
}
double Hypocenter::seaz(double lat0, double lon0)
{
	double epidist, az;

	dist(lat0,lon0,lat,lon,&epidist, &az);
	return(az);
}

void Hypocenter::tt_setup(string meth, string mod)
{
   /* In this case silently ignore meth */
  try {
	ttcalc=TauPCalculator(mod.c_str());
	// I think this may be redundant
	model=mod;
  } catch(...){throw;};
}

			

double Hypocenter::phasetime(double lat0, double lon0, double elev, string phase)
		throw(SeisppError)
{
    double delta;
    delta = this->distance(lat0,lon0);
    try {
    	return(this->ttcalc.phasetime(delta,z,phase.c_str()));
    } catch(...){throw;};
}

double Hypocenter::ptime(double lat0, double lon0, double elev)
		throw(SeisppError)
{
    double delta;
    delta = this->distance(lat0,lon0);
    try {
    	return(this->ttcalc.Ptime(delta,z));
    } catch(...){throw;};
}
SlownessVector  Hypocenter::phaseslow(double lat0, double lon0, double elev, string phase)
		throw(SeisppError)
{
    double epidist, az;
    dist(lat0,lon0,lat,lon,&epidist, &az);
    double baz=az+M_PI;
    try{
        return(this->ttcalc.phaseslow(epidist,baz,z,phase.c_str()));
    } catch(...){throw;};
}
SlownessVector Hypocenter::pslow(double lat0, double lon0, double elev)
		throw(SeisppError)
{
	string phs="P";

	try{
                SlownessVector u;
		u = this->phaseslow(lat0,lon0,elev,phs);
                return(u);
	} catch (...){throw;};
}
} // Termination of namespace PWMIG definitions

