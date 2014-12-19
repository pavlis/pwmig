#ifndef _PWMIGTRAVELTIMECALCULATOR_H_
#define _PWMIGTRAVELTIMECALCULATOR_H_
#include "slowness.h"
#include "gclgrid.h"
namespace PWMIG
{
using namespace std;
using namespace PWMIG;
using namespace SEISPP;
/*! \brief Travel time calculator pure virtual base interface definition.

Travel time calculators are a basic concept in seismology.  This is a C++
interface definition to define a generic travel time interface.   The 
intent is we can wrap multiple travel time engines to this interface.
This can and hopefully will eventually include Antelope's ttcalc 
interface hidden behind this OOP mechanism.

Note the interface intentionally only specifies a generic phase method.
Most calculators will want to define simplified method for P and S phases.
I did not force this into the interface since there may be calculators
that may not allow both P and S for a given instance.

In contrast this definition is (perhaps) too dogmatic about 
requiring derivatives.   The presumption is if a particular calculator
is implemented that has cannot readily calculate derivates or
slowness vectors the required methods will be stubbed to just throw
and error if called.

\author Gary L. Pavlis
*/
class TravelTimeCalculator
{
public:
    /*! \brief Compute travel time for a specified phase.

      In seismology a phase is an impulsive arrival with a well defined
      arrival time.   A calculator needs to be able to calculate the
      predicted time of at least one phase. 

      This is a pure virtual method that must be implemented by any
      child of this base class.

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 

      \return travel time in seconds of phase
      */
    virtual double phasetime(Geographic_point& source, 
		Geographic_point& receiver,const char *phase)=0;
    /*! \brief Compute slowness vector for a specified phase.

      In seismology a phase is an impulsive arrival with a well defined
      arrival time.   A calculator needs to be able to calculate the
      predicted slowness vector of at least one phase.  A slowness 
      vector is defined by the SEISPP::SlownessVector object 
      (see that documentation for the definition and assumed units.)

      This is a pure virtual method that must be implemented by any
      child of this base class.

      \param source defines source coordinates with angles in radians and r in km
      \param receiver defines receiver coordinates with angles in radians and r in km
      \param phase defines the seismological name of the required phase. 
      \return predicted slowness vector for the specified phase.
      */
    virtual SlownessVector phaseslow(Geographic_point& source,
		Geographic_point& receiver,const char *phase)=0;
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
    virtual dvector TimePhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)=0;
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
    virtual dmatrix SlownessPhaseDerivatives(Geographic_point& source,
                Geographic_point& receiver,const char* phase)=0;
};
} // END PWMIG Namespace Encapsulation
#endif
