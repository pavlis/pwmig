
/* This object is the core of the implementation of this synthetic generator */
#include <iostream>
#include <cmath>
#include "coords.h"
#include "tt.h"
#include "interpolator1d.h"
#include "seispp.h"
#include "SphericalRayPathArray.h"

//#include "PointSourcePSSynthetic.h" 
//#include "vectorcls.h"

//#include <Hypocenter.h>
namespace SEISPP {
using namespace std;
using namespace SEISPP;
const double FLT_EPSILON=1e-19;

SphericalRayPathArray::SphericalRayPathArray(VelocityModel_1d& vmod, vector<double>& rayp, double zminin, double zmax,  double dzin)
{
    const string base_error("SphericalRayPathArray constructor:  ");
    vector<double>::iterator rayptr;
    int nrays=rayp.size();
    if(nrays<=1) throw SeisppError(base_error
                +"input ray parameter vector is empty");
    for(rayptr=rayp.begin()+1;rayptr!=rayp.end();++rayptr)
        if((*rayptr)-(*(rayptr-1))<0.0) throw SeisppError(base_error
                + "ray parameter vector must be an increasing sequence");
    /* rayp vector must be an increasing sequence from 0 */
    if(fabs(rayp[0])>FLT_EPSILON) throw SeisppError(base_error
                + "First point in ray parameter vector must be 0");
    zmin=zminin;
    dz=dzin;
    rays.reserve(nrays);
    try { 
        const string mode("z");
        const double vfortmax(2.0);  // needs to be smaller than anything realistic 
        double tmax=zmax/vfortmax;
        slow=rayp;
        for(rayptr=rayp.begin();rayptr!=rayp.end();++rayptr)
        {
            rays.push_back(RayPathSphere(vmod,*rayptr,zmax,tmax,dz,mode));
        }
        /* Next need to calculate ray theory amplitudes -- need to consult references.
        first step is to determine nz as the minimum depth found for all rays */
        vector<RayPathSphere>::iterator raypaths,nextpath;
        raypaths=rays.begin();
        nz=raypaths->npts;
        ++raypaths;
        for(;raypaths!=rays.end();++raypaths)
        {
            if( (raypaths->npts)<nz) nz=raypaths->npts;
        }
        zfloor=dz*static_cast<double>(nz-1);
        if(SEISPP_verbose) cout << "SphericalRayPathArray:  using grid of "
                            << nz << " points spaced at "<<dz
                                <<" for total depth of "<<zfloor<<endl;
        /* Now set the amplitudes using ray geometric spreading */
        amps=dmatrix(nz,nrays);
        amps.zero();
        int i,j;
        double z,tani0,dx,dpdx;
        const double nullamp(-99999.);
        /* nrays -1 is the loop range because we use a forward 
        difference formula for amplitude calculation.  Last two
        columns will be identical.  Not ideal, but adequate
        for planned use of this */
        for(j=0,raypaths=rays.begin();j<(nrays-1);++j,++raypaths)
        {// raypaths is the iterator, while rays is the vector object
            for(i=0;i<nz;++i)
            {
                z=raypaths->depth(i);
                /* flag amp values above zmin with a negative value */
                if(z<zmin)
                    amps(i,j)=nullamp;
                else
                {
                    nextpath=raypaths;
                    ++nextpath;
                    dpdx=(nextpath->p) - (raypaths->p);
                    // here dx is delta increment
                    dx=(nextpath->delta[i])-(raypaths->delta[i]);
                    dx*=raypaths->r[0];
                    if(dx<=0.0) throw SeisppError(base_error
                            + "Ray grid error.  incremental delta .le. 0.0");
                    dpdx/=dx;
                    // now dx is used for local tangent computation 
                    if(i==(nz-1))
                        dx=raypaths->delta[nz-1]-raypaths->delta[nz-2];
                    else
                        dx=raypaths->delta[i+1]-raypaths->delta[i];
                    dx*=raypaths->r[0];
                    if(j==0)
                        amps(i,j)=dpdx;
                    else
                    {
                        tani0=dz/dx;
                        amps(i,j)=tani0*dpdx/(raypaths->delta[i]*raypaths->r[0]);
                    }
                }
            }
        }
        /* copy last column amplitude */
        for(i=0;i<nz;++i) amps(i,nrays-1)=amps(i,nrays-2);
        /* Amplitude right now is energy.  Normalize to p=0 value at first
           valid value and convert to amplitude by square root factor */
        double ampnormalizer;
        for(int i=0;i<nz;++i)
            if(amps(i,0)>0.0)
            {
                ampnormalizer=amps(i,0);
                break;
            }
        for(j=0;j<nrays;++j)
            for(i=0;i<nz;++i)
                if(amps(i,j)>0.0)
                    amps(i,j)=sqrt(amps(i,j)/ampnormalizer);

    } catch(...){throw;};
}
SphericalRayPathArray::SphericalRayPathArray(VelocityModel_1d& vmod, 
        double p0, double dp, int np, 
        double zminin, double zmax,  double dzin)
{
    vector<double> pvector;
    pvector.reserve(np);
    double p;
    int i;
    for(i=0,p=0.0;i<np;++i,p+=dp)pvector.push_back(p);
    *this=SphericalRayPathArray(vmod,pvector,zminin,zmax,dzin);
}

pair<double,double> SphericalRayPathArray::time_and_amp(double delta, double z)
{
// try{
    const string base_error("SphericalRayPathArray::time_and_amp method:  ");
    if(z<zmin) throw SeisppError(base_error
            + "Requested point source depth is smaller than zmin.\n"
          + "Edit pf file.");
    if(z>zfloor) throw SeisppError(base_error
            + "Requested depth is below depth floor for this ray grid ");
    double time,amp;
    // scan the constant z position immediately above z to bracket the delta requested
     
    vector<RayPathSphere>::iterator raypaths,rayupper,raylower;
    int iupper,ilower;
    int iz=static_cast<int>(z/dz);
    for(iupper=0,raypaths=rays.begin();raypaths!=rays.end();++raypaths,++iupper)
        if(raypaths->delta[iz]>delta) break;
//    rayupper=raypaths-1;
     rayupper=raypaths;
   //try{
//    if(raypaths==rays.end()-1) throw SeisppError("distance delta exceeds limit"); 
    if(rayupper==rays.end())  return(pair<double,double> (0,0));//exceeds the upper limit..   
    
   //}
/*   catch(SeisppError& serr){
      throw serr;
    return pair<double,double> (0,0);
   } */

    for(ilower=0,raypaths=rays.begin();raypaths!=rays.end();++raypaths,++ilower)
        if(raypaths->delta[iz+1]>delta) break;

    if(ilower!=0)// test if the rayparameter is smaller than the 1.st member;
	 {   raylower=raypaths-1; ilower--;}
    else
            raylower=raypaths;

      //That gives raylower->delta[iz+1]<=delta

    double zE=(iz+1)*dz-z;

    if(raylower==rayupper){//use linear interpolation.
	cout<<"raylower equals rayupper"<<iupper<<endl;
	double wt1[2];
	wt1[1]=zE/dz; wt1[0]=1-wt1[1];
	time=wt1[0]*rayupper->t[iz+1]+wt1[1]*rayupper->t[iz];
	amp=wt1[0]*amps(iz+1, iupper)+wt1[1]*amps(iz, iupper);
	return(pair<double,double>(time,amp));

    }
	const double radius0=6371.0;//in km, earth radius.
    double rd=radius0-iz*dz;//the radius vector rc at desired depth
    double rc=radius0-(iz+1)*dz;//the radius vector rc at desired depth
    double lambda=rc*(rayupper->delta[iz+1]-raylower->delta[iz+1])-
			rd*(rayupper->delta[iz]-raylower->delta[iz]);
    lambda/=dz;
    double delX1=rc*(rayupper->delta[iz+1]-rayupper->delta[iz]);
    double xE=rc*(rayupper->delta[iz+1]-delta);
//    double zE=(iz+1)*dz-z;
    double xEp=xE+(lambda-delX1/dz)*zE+(-lambda*dz);
    double xx1=rd*(rayupper->delta[iz]-raylower->delta[iz]);
    //zEp=zE;//because we assume the local subsurface is flat. 
    double wt[2][2];// bilinear interpolation coefficients.
    double  adx;
    wt[0][1]=wt[1][1]=zE/dz;
    wt[0][0]=wt[1][0]=1.0-wt[0][1];
    adx=xEp/xx1;
    wt[0][0]*=(1-adx); wt[0][1]*=(1-adx);
    wt[1][0]*=(adx); wt[1][1]*=(adx);
    double sumwt(0.0);
    for(int i=0;i<2;++i)
       for(int j=0;j<2;++j) sumwt+=wt[i][j];
    cout<<"sum of weight: "<<sumwt<<endl;
    time=0.0;
    time+=wt[0][0]*rayupper->t[iz+1];
    time+=wt[0][1]*rayupper->t[iz];
    time+=wt[1][0]*raylower->t[iz+1];
    time+=wt[1][1]*raylower->t[iz];
    amp=0.0;
    amp+=wt[0][0]*amps(iz+1, iupper);
    amp+=wt[0][1]*amps(iz, iupper);
    amp+=wt[1][0]*amps(iz+1, ilower);
    amp+=wt[1][1]*amps(iz, ilower);
    return(pair<double,double>(time,amp));

    
/*    a=1.0/(((rayupper+1)->delta[iz]-(rayupper)->delta[iz]));
    dx=delta-(rayupper->delta[iz]);
    adx=a*dx;
    wt[0][0]=1.0-adx;
    wt[0][1]=adx;
    a=1.0/(((raylower+1)->delta[iz+1]-raylower->delta[iz+1]));
    dx=delta-(raylower->delta[iz+1]);
    adx=a*dx;
    wt[1][0]=1.0-adx;
    wt[1][1]=adx;
    a=1.0/dz;
    dx=z-static_cast<double>(iz-1)*dz;
    adx=a*dx;
    wt[0][0]*=(1.0-adx);
    wt[0][1]*=(1.0-adx);
    wt[1][0]*=adx;
    wt[1][1]*=adx;
    double sumwt(0.0);
    for(int i=0;i<2;++i)
       for(int j=0;j<2;++j) sumwt+=wt[i][j];
    time=0.0;
    time+=wt[0][0]*rayupper->t[iz];
    time+=wt[0][1]*(rayupper+1)->t[iz];
    time+=wt[1][0]*raylower->t[iz+1];
    time+=wt[1][1]*(raylower+1)->t[iz+1];
    amp=0.0;
    amp+=wt[0][0]*amps(iz, iupper);
    amp+=wt[0][1]*amps(iz, iupper);
    amp+=wt[1][0]*amps(iz+1,ilower);
    amp+=wt[1][1]*amps(iz+1,ilower);

    return(pair<double,double>(time,amp));*/

/* catch(SeisppError& serr){
      throw serr;
      return pair<double,double> (0,0);
 }*/

//return pair<double,double> (0,0);
}
  
SphericalRayPathArray::SphericalRayPathArray(const SphericalRayPathArray& parent)
{
    nrays=parent.nrays;
    rays=parent.rays;
    slow=parent.slow;
    zmin=parent.zmin;
    nz=parent.nz;
    dz=parent.dz;
    amps=parent.amps;
    zfloor=parent.zfloor;
}
SphericalRayPathArray& SphericalRayPathArray::operator=(
        const SphericalRayPathArray& parent)
{
    if(this!= &parent)
    {
	    nrays=parent.nrays;
	    rays=parent.rays;
	    slow=parent.slow;
            zmin=parent.zmin;
	    nz=parent.nz;
	    dz=parent.dz;
	    amps=parent.amps;
	    zfloor=parent.zfloor;
    }
    return(*this);
}


}//end of namespace SEISPP
