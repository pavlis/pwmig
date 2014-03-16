#include <cstdio>
#include <string>
#include <sstream>
#include <float.h>
#include <memory>
#include "stock.h"
#include "coords.h"
#include "dbpp.h"
#include "dmatrix.h"
#include "gclgrid.h"
#include "ray1d.h"
#include "ensemble.h"
#include "seispp.h"
#include "interpolator1d.h"
#include "PwmigFileHandle.h"
#ifdef MATLABDEBUG
#include "MatlabProcessor.h"
#endif
#ifdef MPI_SET
	#include <mpi.h>
#endif
using namespace std;
using namespace SEISPP;
using namespace INTERPOLATOR1D;
#include "pwmig.h"

//DEBUG  Uncomment if not using matlab for debu
#ifdef MATLABDEBUG
MatlabProcessor mp(stdout);
#endif
void usage()
{
    cerr << "Usage: "<<endl
        << "pwmig listfile outdir [-np np -rank r -V -v -pf pfname]"<<endl
        << "  listfile contains names of output files from pwstack that drive processing"
        << "  Files of results are written to outdir"<<endl
        << "  -np and -rank are used for cluster processing"<<endl;
        exit(-1);
}


/* small companion to below blindly computes distance for ray path (stored in
 dmatrix ray) to current point (ray(0:2,i0)) from previous point (ray(0:2,i0-1)).
This is done blindly with pointer arithmetic and assumes i0 is greater than or
equal to 1.  
*/
double pathdist(dmatrix& ray,int i0)
{
	double *ptr;  // Do this with pointer arithmetic for efficiency
	double dx,r;
	ptr=ray.get_address(0,i0);
	dx=(*ptr)-(*(ptr-3));
	r=dx*dx;
	++ptr;
	dx=(*ptr)-(*(ptr-3));
	r+=dx*dx;
	++ptr;
	dx=(*ptr)-(*(ptr-3));
	r+=dx*dx;
	return(sqrt(r));
}
/* The procedures below using pathintegral can get in trouble
if the model is slightly below the reference ellipsoid.  This
procedure aims to repair that potential thorny problem in a 
systematic way. It uses this algorithm:
1) scan top surface of model and find the maximum point below
   the reference ellipsoid.
2) if the point found in 1 is above the datum or 0, do nothing.
3) otherwise fudge the model upward by the valued computed 
  + the input parameter dz.

Returns the amount the model was displaced.*/

double fudge_model_upward(GCLscalarfield3d& model,double dz)
{
	// A GCLgrid always has the top surface at n3-1
	// so we scan that surface for the lowest point relative
	// to r0_ellipse.
	int i,j;
	double lat,r,dr,drmin;
	int ksurface=model.n3 - 1;
	double offset;
	for(i=0;i<model.n1;++i)
	{
		for(j=0;j<model.n2;++j)
		{
			r=model.r(i,j,ksurface);
			lat=model.lat(i,j,ksurface);
			dr=r-r0_ellipse(lat);
			if((i==0) && (j==0))
			{
				drmin=dr;
			}
			else
			{
				drmin=min(dr,drmin);
			}
		}
	}
	if(drmin>=0.0) 
		offset=0.0;
	else
	{
		offset=(-drmin)+dz;
		Geographic_point geo;
		Cartesian_point p;
		int k;
		for(i=0;i<model.n1;++i)
			for(j=0;j<model.n2;++j)
				for(k=0;k<model.n3;++k)
				{
					geo.lat=model.lat(i,j,k);
					geo.lon=model.lon(i,j,k);
					geo.r=model.r(i,j,k);
					geo.r+=offset;
					p=model.gtoc(geo);
					model.x1[i][j][k]=p.x1;
					model.x2[i][j][k]=p.x2;
					model.x3[i][j][k]=p.x3;
				}

	}
	return(offset);
}
	
/* computes travel times along a ray path defined by the input dmatrix of Cartesian
coordinate (path) using a 3D model defined by the GCLsclarfield3d variable U3d.  
The path is assumed to be congruent with the ray geometry GCLgrid (raygrid)
but not necessarily congruent with the velocity model field.  Thus we use the
not equal operator to test for conguence between raygride and U3d.  
It is assumed the U3d holds SLOWNESS values in units of s/km.

Because most 3D Earth models have finite extents it is highly likely a ray can 
traverse outside the reference model.  We allow for this here and use the 1d model
(V1d) as a background reference.  That is, when the ray moves outside the 3D refernce
model the 1d model is used.  Note this ASSUMES that the input path starts inside the
3d model box and passes outside somewhere along path.  If the path starts outside the
V3d region the 1d model will be silently used.  i.e.  the 3d model will not be used
if the path runs from outside into the box.  This is because the algorithm used is
a simple one which simply checks the length of the vector returned by the pathintegral
library function against the size of path.  This should be generalized if this were
ever made into a library function.

Note path is intentionally passed by value not by reference as remap 
path can redefine it.
*/

vector<double> compute_gridttime(GCLscalarfield3d& U3d, 
	int ix1, int ix2,
		VelocityModel_1d& V1d, 
			GCLgrid3d& raygrid,
				dmatrix& path)
{

	dmatrix thispath(path);
	// test to be sure the grids are congruent or paths will not mesh
	if(dynamic_cast<BasicGCLgrid&>(U3d)
			!=dynamic_cast<BasicGCLgrid&>(raygrid))
	{
		thispath=remap_path(raygrid,path,U3d);
	}
// Discard for subduct model testing. Current 1d model is not consistent
// with 3d model definition.  Bypass for now and use 1d model only 
// for initial result.  
/*  Turned back on for test */
	vector<double> times;
	times=pathintegral(U3d,thispath);
	// This fills the output vector with 1d values if the path wanders outside
	// the 3d grid bounds
	if(times.size()!= thispath.columns())
	{
		double vel,dt;
		int k,kstart;
		// Watch for special case when stations is outside
		// the 3d model completely
		if(times.size()<=0)
			kstart=1;
		else
			kstart=times.size();
		for(k=kstart;k<thispath.columns();++k)
		{
			Cartesian_point p;
			double r,z;
			p.x1=thispath(0,k);
			p.x2=thispath(1,k);
			p.x3=thispath(2,k);
			z=dynamic_cast<BasicGCLgrid&>(raygrid).depth(p);
			vel=V1d.getv(z);
			r=pathdist(thispath,k);
			dt = r/vel;
			times.push_back(times[k-1]+dt);
		}
	}
	return(times);
}
			

	
/* Returns a 3x nx  matrix of ray path unit vectors pointing in
direction of tangent to the path defined by the 3 x nx  input matrix 
passed as ray.  The algorithm is a simple forward difference scheme 
with no checking for possible roundoff problems.  Because we use forward
differences the output vector at point i is computed from the segment 
immediately forward (x[i+1]-x[i]).  Note this means the sign convention
is determined solely from the path variables.  Each vector is normalized
to unity.  The last point two points in the output are equal to keep
sizes consistent and is the only choice with a forward difference scheme.

Note the dmatrix is created with new here and needs to be cleared by caller.

Author:  Gary Pavlis
Written:  August 2003
*/

dmatrix *ray_path_tangent(dmatrix& ray)
{
	int nx;
	double dx[3];
	double nrmdx;
	int i,j;

	nx=ray.columns();
	dmatrix *gptr;
	gptr = new dmatrix(3,nx);
	dmatrix& gamma=*gptr;

	// debug trap
	if(nx<2 || ray.rows()!=3)
	{
		cerr << "ray_path_tangent:  input path size "
			<< ray.rows() 
			<<" by " 
			<< ray.columns() <<endl;
		exit(1);
	}

	for(i=0;i<nx-1;++i)
	{
		dx[0]=ray(0,i+1)-ray(0,i);
		dx[1]=ray(1,i+1)-ray(1,i);
		dx[2]=ray(2,i+1)-ray(2,i);
		nrmdx=dnrm2(3,dx,1);
		// debug trap.  Won't happen if used correctly. Could to be removed eventually
		if(nrmdx<=0.0)
		{
			cerr<<"ray_path_tangent: coding error two points on path are equal."<<endl;
			exit(1);
		} 
		dscal(3,1.0/nrmdx,dx,1);
		for(j=0;j<3;++j) gamma(j,i)=dx[j];
	}
	// copy the last point
	for(j=0;j<3;++j) gamma(j,nx-1)=gamma(j,nx-2);
	return(gptr);
}

/* Computes a dmatrix of gradient S vectors for a ray GCLgrid defining the x3
grid line from x1,x2 grid position ix1,ix2 (convention in this program).
Velocities used to scale the vectors are derived from the 1D velocity model
passed as Vs1d.  This is an approximation but one that should make vitually no
difference in the results for the simplicity and speed it (likely) buys.
*/

dmatrix compute_gradS(GCLgrid3d& raygrid,int ix1, int ix2, VelocityModel_1d& Vs1d)
{
	int k,kk,i;
	dmatrix gradS(3,raygrid.n3);
	double vs;
	double nrm;
	// We start at the surface and work down 
	// gradS vectors should point upward
	// The raygrid lines are oriented from the bottom up so sign is 
	// as below
	for(k=0,kk=raygrid.n3-1;k<raygrid.n3-1;++k,--kk)
	{
		gradS(0,k)=raygrid.x1[ix1][ix2][kk]
				- raygrid.x1[ix1][ix2][kk-1];
		gradS(1,k)=raygrid.x2[ix1][ix2][kk]
				- raygrid.x2[ix1][ix2][kk-1];
		gradS(2,k)=raygrid.x3[ix1][ix2][kk]
				- raygrid.x3[ix1][ix2][kk-1];
		// use the velocity from the upper point
		vs=Vs1d.getv(raygrid.depth(ix1,ix2,kk));
		// gradient is unit vector multiplied by slowness
		nrm=dnrm2(3,gradS.get_address(0,k),1);
		for(i=0;i<3;++i) gradS(i,k)/=(nrm*vs);
	}
	// Final point is just a copy of the second to last as we run out 
	// of points in forward difference
	for(i=0;i<3;++i)gradS(i,raygrid.n3-1)=gradS(i,raygrid.n3-2);
	return(gradS);
}
vector<double> compute_unit_normal_P(GCLgrid3d& raygrid,double x1, double x2, double x3)
{
	// Ray path tangents do not vary rapidly with position.  We just get the nearest
	// neighbor and compute by finite differences
	vector<double>nu; // unit vector here normally pointing generally up and in some direction
	int index[3];
	int err;
	double dx;
	double nrmx;
	double rtest;
	int k;
	Geographic_point geo;

	err=raygrid.lookup(x1,x2,x3);
	switch(err)
	{
		case 1:
		case -1:
			// If the lookup failed we start here.  Since we are using a grid
			// constructed of 1d rays we can cheat and jump to the center of the 
			// grid and try to use something reasonably approximates the corect
			// ray direction
			raygrid.reset_index();
			raygrid.get_index(index);
			geo=raygrid.ctog(x1,x2,x3);
			rtest=geo.r;
			// hunting upward for the first point on the ray through the 
			// origin that is just above the radius defined in geo
			// we set the index position there for the calculation below
			for(k=0;k<raygrid.n3;++k)
			{
				geo=raygrid.geo_coordinates(index[0],index[1],k);
				if(geo.r>rtest) 
				{
					index[2]=k;
					break;
				}
			}
			if(k>=raygrid.n3) index[2]= raygrid.n3 - 2;  // set to one below surface
		case 0:
		case 2:
			raygrid.get_index(index);
			// Standard fix for forward diff.  Back off one if at the top edge
			if(index[2]>=raygrid.n3-1)--index[2];  
			dx=raygrid.x1[index[0]][index[1]][index[2]+1];
			dx-=raygrid.x1[index[0]][index[1]][index[2]];
			nu.push_back(dx);
			dx=raygrid.x2[index[0]][index[1]][index[2]+1];
			dx-=raygrid.x2[index[0]][index[1]][index[2]];
			nu.push_back(dx);
			dx=raygrid.x3[index[0]][index[1]][index[2]+1];
			dx-=raygrid.x3[index[0]][index[1]][index[2]];
			nu.push_back(dx);
			nrmx=dnrm2(3,&(nu[0]),1);
			dscal(3,1.0/nrmx,&(nu[0]),1);
	}
	return(nu);
}

/*! \brief Computes incident wave travel times in pwmig
// Computes a GCLgrid defined field (raygrid) for an incident P wavefield.  
// The result is travel times from a given hypocenter to points inside a GCLgrid3d
// object.  The times are computed from a 1d refernce model to the base of the model
// and then using approximate ray tracing from the base up.  
// Actual algorithm computes times to the surface and then subtracts path integral times
// to get 3d travel times at points in the grid.  The grid geometry is defined by
// incident wave rays traced with a 1d reference model.  That is, they are like the
// S wavefield raygrid used in the main program of pwmig.
//
// Modified July 2009:  added zdecfac parameter for efficiency.  Found that for higher
//   sample rate data this created an unnecessarily huge travel time grid.
// 
// \param pstagrid parent pseudostation grid used to build this grid.
// \param border_pad is defines additional padding used to extend the pseudostation
//     grid on which the data are defined.  This is necessary (essential really) because
//     in pwmig this grid is used for all plane wave components while the S grids are
//     different for each plane wave component.  This should be made large enough that
//     it can contain all scattering points connected to S rays from the pseudostationh
//     grid.  
// \param UP3d P wave slowness defined with a 3d field object
// \param vp1d P wave 1d reference model.  Used to trace traces
// \param h hypocenter of source that created wavefield being imaged
// \param zmax ray trace parameters passed directly to BuildGCLraygrid
// \param tmax ray trace parameters passed directly to BuildGCLraygrid
// \param dt ray trace parameters passed directly to BuildGCLraygrid
// \param zdecfac output ray grid is decimated by this amount in z
//
//\returns auto_ptr object containing the travel time field
//\exception catches all exceptions and passes them along.  Several functions
//      called in this procedure could throw several possible exceptions I don't
//      know well which is the reason for the catch all syntax.
//
*/
auto_ptr<GCLscalarfield3d> ComputeIncidentWaveRaygrid(GCLgrid& pstagrid,
	int border_pad,
		GCLscalarfield3d& UP3d,
			VelocityModel_1d vp1d,
				Hypocenter h,
					double zmax,
						double tmax, 
							double dt,
								int zdecfac)
{
	int i,j,k;
	// Incident wave slowness vector at grid origin
	// Always use 0 elevation datum for this calculation
	// note this isn't always used.  
	SlownessVector u0=h.pslow(pstagrid.lat0,pstagrid.lon0,0.0);
	// First set up the new expanded grid.  The new grid has border_pad cells
	// added on each side of the parent grid (pstagrid).  
	int n1new, n2new;
	int i0new, j0new;
	n1new = pstagrid.n1 + 2*border_pad;
	n2new = pstagrid.n2 + 2*border_pad;
	i0new = pstagrid.i0 + border_pad;
	j0new = pstagrid.j0 + border_pad;
	try {
		// this assumes the pstagrid is a "regular" gclgrid created with this same constructor.
		// Otherwise there may be small disconnect, although it should not really matter
		// provided dx1_nome and dx2_nom approximate the parent well.
		GCLgrid ng2d(n1new, n2new, string("ng2d"),
				pstagrid.lat0,pstagrid.lon0,pstagrid.r0,
				pstagrid.azimuth_y, pstagrid.dx1_nom, pstagrid.dx2_nom, i0new, j0new);
		// We MUST make force ng2d to be in the same coordinate
		// system as the velocity model.
		remap_grid(ng2d,dynamic_cast<BasicGCLgrid&>(UP3d));
		// Use ng2d to compute a 1d reference model travel time grid
		// Note we explicitly do no include elevation in this calculation assuming this
		// will be handled with statics at another point
		GCLscalarfield Tp0(ng2d);
		for(i=0;i<Tp0.n1;++i)
		{
			for(j=0;j<Tp0.n2;++j)
			{
				Tp0.val[i][j]=h.ptime(Tp0.lat(i,j),Tp0.lon(i,j),0.0);
			}
		}
		auto_ptr<GCLgrid3d> IncidentRaygrid(Build_GCLraygrid(false, ng2d, u0,h,vp1d,zmax,tmax,dt));
		GCLgrid3d *Pttgrid=decimate(*IncidentRaygrid,1,1,zdecfac);
		auto_ptr<GCLscalarfield3d> Tp(new GCLscalarfield3d(*Pttgrid));
		delete Pttgrid;
		/* this is complicated by reverse order of the two paths used here.
		Without decimation I used kend=Tp->n3 - 1  This is the form with decimation
		which should work correctly for any zdecfac */
		int kend=Tp->n3;
		int kkend=((Tp->n3)-1)*zdecfac;
		int kk;
		for(i=0;i<Tp->n1;++i)
		{
			for(j=0;j<Tp->n2;++j)
			{
				auto_ptr<dmatrix> path(extract_gridline(*IncidentRaygrid,
							i,j,kkend,3,true));
				// this is the pwmig function using 1d if path is outside 3d model
				vector<double>Ptimes;
				Ptimes=compute_gridttime(UP3d,i,j,vp1d,*IncidentRaygrid,*path);
				//for(k=0,kk=kend;k<=kend;++k,--kk)
				for(k=0,kk=kkend;k<kend;++k,kk-=zdecfac)
				{
					Tp->val[i][j][k] = Tp0.val[i][j] - Ptimes[kk];
				}
			}
		}
		return Tp;
	} catch (...) {throw;}
}

double compute_receiver_Ptime(GCLgrid3d& raygrid, int ix1, int ix2,
		GCLscalarfield3d& TP,Hypocenter& h)
{
	double Tpr;  // result
	int err;
	double x1,x2,x3;
	int nsurface=raygrid.n3 - 1;
	x1=raygrid.x1[ix1][ix2][nsurface];
	x2=raygrid.x2[ix1][ix2][nsurface];
	x3=raygrid.x3[ix1][ix2][nsurface];
	err=TP.lookup(x1,x2,x3);
	switch (err)
	{
		case 0:
		// lookup almost always returns 2 in this case because receiver
		// is on top surface of grid.
		case 2:
			Tpr=TP.interpolate(x1,x2,x3);
			break;
		default:
		// Here we can recover from a lookup failure by reverting to the
		// 1d calculator
			TP.reset_index();
			Tpr=h.ptime(raygrid.lat(ix1,ix2,nsurface),
					raygrid.lon(ix1,ix2,nsurface),0.0);
	}
	return(Tpr);
}
	
/* This function computes the domega term for the inverse Radon transform inversion
formula.  It works according to this basic algorithm.
1.  If u0 is the slowness vector for the current ray path, compute four neighboring
rays at 1/2 grid spacings:  u0+dux1/2, u0-dux1/2, u0+duy1/2, and u0-duy1/2 where the
1/2 means the 1/2 a grid spacing.  In slowness space this forms a rectangle surrounding
the u0 point.
2.  Compute a ray for each of these points using constructors for a RayPathSphere 
object and the routine GCLgrid_Ray_Project.  Note these are computed with the equal
depth option, not equal time.
3.  Compute gradS for each point on each ray.  
4.  Interpolate the input gradP vectors to depths (the input is on an irregular
grid).  
5.  Compute the gradS+gradP sums to build an array of unit vectors for each ray.
6.  compute solid angles between + and - pairs of finite difference rays using
a vector cross product and a small angle approximation (if a and b are unit vectors
axb = |a||b|sin theta = sin theta ~ theta for small angles )  Solid angle is product
of the two angles.

Returns a vector of domega terms on the grid defined by the input ray path.

Arguments:

	u0 - input slowness vector (note passed as SlownessVector object)
		u0 is base slowness for S raygrid passed as g (below)
	dux, duy - base slowness grid increments.
	vmod - 1d velocity model used for ray tracing 
	zmax, dz - ray trace parameters.  trace to depth zmax with depth increment dz
	g - gclgrid for S ray geometry
	ix1, ix2  - index position of receiver point in gclgrid g.
	gradP - parallel matrix to path of P time gradient vectors.
	zP - vector of depths parallel to gradP 
	(Note algorithm assumes gradP,zP are ordered from surface down)

Returns an np length STL vector object.

Author:  Gary Pavlis
Written:  NOvember 2003
*/
	

vector<double> compute_domega_for_path(SlownessVector& u0,double dux, double duy,
	VelocityModel_1d& vmod, 
		double zmax,
			double dz,
				GCLgrid3d& g,int ix1, int ix2,
					dmatrix& gradP,
						vector<double>& zP)
{
	SlownessVector udx1, udx2, udy1, udy2;
	dmatrix *pathdx1, *pathdx2, *pathdy1, *pathdy2;
	dmatrix *gradSdx1, *gradSdx2, *gradSdy1, *gradSdy2;
	int i;
	int npath; // used to define length of valid output 

	udx1 = u0;
	udx1.ux -= dux/2.0;
	udx2=u0;
	udx2.ux += dux/2.0;
	udy1 = u0;
	udy1.uy -= duy/2.0;
	udy2=u0;
	udy2.uy += duy/2.0;
	//
	// Now call the constructors for the basic ray paths for these four rays.  
	// We set the constructor in depth model and ASSUME zmax is small enough that
	// the ray for each u will not turn within vmod.  
	//
	RayPathSphere raydx1(vmod,udx1.mag(),zmax,1.0e99,dz,"z");
	RayPathSphere raydx2(vmod,udx2.mag(),zmax,1.0e99,dz,"z");
	RayPathSphere raydy1(vmod,udy1.mag(),zmax,1.0e99,dz,"z");
	RayPathSphere raydy2(vmod,udy2.mag(),zmax,1.0e99,dz,"z");
/*
mp.load(raydx1.r,string("rdx1"));
mp.load(raydx2.r,string("rdx2"));
mp.load(raydy1.r,string("rdy1"));
mp.load(raydy2.r,string("rdy2"));
mp.load(raydx1.delta,string("deltax1"));
mp.load(raydx2.delta,string("deltax2"));
mp.load(raydy1.delta,string("deltay1"));
mp.load(raydy2.delta,string("deltay2"));
mp.load(raydx1.t,string("tdx1"));
mp.load(raydx2.t,string("tdx2"));
mp.load(raydy1.t,string("tdy1"));
mp.load(raydy2.t,string("tdy2"));
*/
	//
	// project these into the GCLgrid coordinate system
	//
	try {
		pathdx1 = GCLgrid_Ray_project_down(g,raydx1, udx1.azimuth(),ix1,ix2,
				g.n3-1);
		pathdx2 = GCLgrid_Ray_project_down(g,raydx2, udx2.azimuth(),ix1,ix2,
				g.n3-1);
		pathdy1 = GCLgrid_Ray_project_down(g,raydy1, udy1.azimuth(),ix1,ix2,
				g.n3-1);
		pathdy2 = GCLgrid_Ray_project_down(g,raydy2, udy2.azimuth(),ix1,ix2,
				g.n3-1);
/*
mp.load(*pathdx1,string("pdx1"));
mp.load(*pathdx2,string("pdx2"));
mp.load(*pathdy1,string("pdy1"));
mp.load(*pathdy2,string("pdy2"));
mp.process(string("plotrays(pdx1,pdx2,pdy1,pdy2)"));
*/
	}  catch (GCLgrid_error& err)
	{
		err.log_error();
                cerr << "Unrecoverable error:  requires a bug fix"<<endl;
                exit(-1);
	}
	//
	// get gradS values by first computing tangents as unit vector and then
	// scaling them by slowness at that depth. Note 1d slowness is used for
	// this calculation, not a 3d value.  This is an approximation that avoid
	// tremendous headaches that could occur otherwise.  It should not be a
	// significant error.
	//
	gradSdx1=ray_path_tangent(*pathdx1);
	gradSdx2=ray_path_tangent(*pathdx2);
	gradSdy1=ray_path_tangent(*pathdy1);
	gradSdy2=ray_path_tangent(*pathdy2);
	delete pathdx1;  delete pathdx2;  delete pathdy1;  delete pathdy2;
	//
	// Check all these matrices have the same size and truncate the 
	// output if necessary writing a diagnostic in that situation.
	//
	npath = gradSdx1->columns();
	if(npath>gradSdx2->columns()) npath=gradSdx2->columns();
	if(npath>gradSdy1->columns()) npath=gradSdy1->columns();
	if(npath>gradSdy2->columns()) npath=gradSdy2->columns();
	if(npath!= gradSdx1->columns())
	{
		cerr << "compute_domega_for_path: irregular path length for bundle of 4 rays used to compute domega" << endl

			<< "Slowness grid probably defines turning rays in thie model" << endl;
	}
	for(i=0;i<npath;++i)
	{
		double slow;
		double depth;
		depth = raydx1.depth(i);
		slow = 1.0/vmod.getv(depth);
		dscal(3,slow,gradSdx1->get_address(0,i),1);
		dscal(3,slow,gradSdx2->get_address(0,i),1);
		dscal(3,slow,gradSdy1->get_address(0,i),1);
		dscal(3,slow,gradSdy2->get_address(0,i),1);
	}
/*
mp.load(gradP,string("gp"));
mp.load(*gradSdx1,string("gsdx1"));
mp.load(*gradSdx2,string("gsdx2"));
mp.load(*gradSdy1,string("gsdy1"));
mp.load(*gradSdy2,string("gsdy2"));
mp.process(string("figure; unitvectorplot(gsdx1);"));
mp.process(string("figure; unitvectorplot(gsdx2);"));
mp.process(string("figure; unitvectorplot(gsdy1);"));
mp.process(string("figure; unitvectorplot(gsdy2);"));
*/
	//
	// Now we interpolate the gradS vector arrays onto the gradP set
	// of depths. This produces a domega congruent with gradS and gradP
	// in the sense that the domega values will correspond to the same
	// node depths as those computed for gradS and gradP.  These can
	// be copied on return into the outputs, without any more interpolation
	//  albeit in the reverse order.
	//
	int npath_P;
	double z0=raydx1.depth(0);  // assume all four rays have same z0
	npath_P = gradP.columns();
	if(npath_P!=zP.size())
		throw SeisppError(string("compute_domega_for_path:  ")
		  + string("vector grid point depths and gradP matrix do not match"));
	dmatrix temp(3,npath_P); // Holds interpolated gradS values before replacement
	linear_vector_regular_to_irregular(z0,dz,*gradSdx1,&(zP[0]),temp);
	dmatrix nudx1(gradP+temp);  // creates vector gradP + gradS
	delete gradSdx1;  // Dont' need this any longer
	// Same for dx2, dy1, and dy2
	linear_vector_regular_to_irregular(z0,dz,*gradSdx2,&(zP[0]),temp);
	dmatrix nudx2(gradP+temp);  
	delete gradSdx2;  
	linear_vector_regular_to_irregular(z0,dz,*gradSdy1,&(zP[0]),temp);
	dmatrix nudy1(gradP+temp);  
	delete gradSdy1;  
	linear_vector_regular_to_irregular(z0,dz,*gradSdy2,&(zP[0]),temp);
	dmatrix nudy2(gradP+temp);  
	delete gradSdy2; 
	// normalize using antelope function d3norm to make these unit vectors
	for(i=0;i<npath_P;++i)
	{
		dr3norm(nudx1.get_address(0,i));
		dr3norm(nudx2.get_address(0,i));
		dr3norm(nudy1.get_address(0,i));
		dr3norm(nudy2.get_address(0,i));
	}
/*
mp.load(nudx1,string("nudx1"));
mp.load(nudx2,string("nudx2"));
mp.load(nudy1,string("nudy1"));
mp.load(nudy2,string("nudy2"));
mp.process(string("figure; unitvectorplot(nudx1);"));
mp.process(string("figure; unitvectorplot(nudx2);"));
mp.process(string("figure; unitvectorplot(nudy1);"));
mp.process(string("figure; unitvectorplot(nudy2);"));
mp.run_interactive();
*/
	//
	// Now get domega using cross products between pairs of unit vectors and 
	// small angle approximation (sin theta approx theta when theta is small)
	//
	vector<double>domega;
	for(i=0;i<npath_P;++i)
	{
		double cross[3];
		double dtheta_x, dtheta_y,domega_i;
		dr3cros(nudx1.get_address(0,i),nudx2.get_address(0,i),cross);
		dtheta_x = dr3mag(cross);
		dr3cros(nudy1.get_address(0,i),nudy2.get_address(0,i),cross);
		dtheta_y = dr3mag(cross);
		// abs shouldn't be necessary really, but better
		// safe than sorry given 0 cost
		domega_i=fabs(dtheta_x*dtheta_y);
		domega.push_back(domega_i);
	}
	return(domega);  
}
/* Much simpler routine to compute weight of this each member of a ray path
 * in generalized radon transform formula.  gradTp and gradTs are gradients
 * for P and S ray directions a defined in the Poppeliers and Pavlis (2003)
 * and Bostock et al. (2001).  Note this function drops the A term that
 * could be computed from geometric spreading because the assumption is
 * the this term is negligible for plane waves
 */
vector<double> compute_weight_for_path(dmatrix& gradTp,dmatrix& gradTs)
{
	const double eightpisq=78.95683521;
	double sum[3],nrmsum;
	int np=gradTp.columns();
	int i,j;
	vector<double> weight(np);

	for(j=0;j<np;++j)
	{
		for(i=0;i<3;++i)
		{
			sum[i]=gradTp(i,j)+gradTs(i,j);
		}
		nrmsum=dnrm2(3,sum,1);
		weight[j]=nrmsum*nrmsum*nrmsum/eightpisq;
	}
	return(weight);
}
/* Computing a running average of a specific length for a 
series of numbers stored in x.  npts is the number of points
in the smoother.  npts/2 on the left and right are padded as the 
constants for the first and last npts that can be averaged.
i.e. there is not endpoint tapering.  Return vector is x
smoothed as requested. 
*/
vector<double> running_average(vector<double>& x, int ns)
{
	int nx=x.size();
	int i,ii,j;
	double avg;
	vector<double> result(nx);

	if(nx<=ns)
	{
		for(i=0,avg=0.0;i<nx;++i)
			avg+=x[i];
		avg=avg/static_cast<double>(nx);
		for(i=0;i<nx;++i) result[i]=avg;
	}
	else
	{
		int npo2=(ns+1)/2;
		double avglen=static_cast<double>(ns);
		for(i=npo2-1;i<nx-(ns-npo2);++i)
		{
			for(j=i,ii=0,avg=0.0;ii<ns;++ii,++j)
				avg+=x[j-npo2+1];
			avg/=avglen;
			result[i]=avg;
		}
		// pad front and back
		for(i=npo2-2;i>=0;--i) result[i]=result[i+1];
		for(i=nx-(ns-npo2);i<nx;++i) result[i]=result[i-1];
	}
	return(result);
}
/* Overloaded version of same procedure as above for a matrix.  Smoothing
here is done only along rows.  Used here for coherence grid to handle 
three-component data */
dmatrix running_average(dmatrix x, int ns)
{
	int nr=x.rows();
	int nc=x.columns();
	int i,j;
	vector<double> row;
	row.reserve(nc);
	dmatrix result(nr,nc);
	for(i=0;i<nr;++i)
	{
		for(j=0;j<nc;++j) row.push_back(x(i,j));
		row=running_average(row,ns);
		for(j=0;j<nc;++j) result(i,j)=row[j];
		row.clear();
	}
	return(result);
}
		
// builds dfile names for migration output
string MakeDfileName(string base, int evid)
{
	string dfile;
	ostringstream sbuf(dfile);
	sbuf<<base<<"_"<<evid;
	return(sbuf.str());
}
// 
// We store velocity models externally in the database, but internally
// slowness is more appropriate.  This converts a velocity field to a 
// slowness field in an efficient way by altering the val array in place.
//
void VelocityFieldToSlowness(GCLscalarfield3d& g)
{
	for(int i=0;i<g.n1;++i)
		for(int j=0;j<g.n2;++j)
			for(int k=0;k<g.n3;++k) g.val[i][j][k]=1.0/g.val[i][j][k];
}
/* Returns a 1d velocity model consistent with slowness field
stored in u.  Algorithm averages node values at each constant x3
level in u and sets 1d node values to be the same.  It then 
computes gradients to provide linear interpolator between each
node point in the 1d model.  This makes the result as close to
the 3d model as reasonably possible in terms of interpolation method. */
VelocityModel_1d DeriveVM1Dfrom3D(GCLscalarfield3d& u)
{
	VelocityModel_1d result(u.n3);
	double zavg,uavg;
	int i,j,k;
	int nperlayer=(u.n1)*(u.n2);
	/* We have to count backwards because the grid orientation is
	from the bottom to the top */
	for(k=u.n3-1;k>=0;k--)
	{
		zavg=0.0;
		uavg=0.0;
		for(i=0;i<u.n1;++i)
		{
			for(j=0;j<u.n2;++j)
			{
				zavg+=u.depth(i,j,k);
				uavg+=u.val[i][j][k];
			}
		}
		zavg /= nperlayer;
		uavg /= nperlayer;
		result.z.push_back(zavg);
		result.v.push_back(1.0/uavg);
	}
	/* Now set gradients */
	for(k=0;k<result.nlayers-1;++k)
	{
		double deriv;
		deriv=(result.v[k+1] - result.v[k])
			/ (result.z[k+1]-result.z[k]);
		result.grad.push_back(deriv);
	}
	/* Set the gradient for the final point to 0 */
	result.grad.push_back(0.0);
	return(result);
}
SlownessVector slowness_average(ThreeComponentEnsemble *d)
{
	double ux,uy;
	int n;
	n=d->member.size();
	if(n<=0) throw SeisppError(string("slowness_average:  ")
			+"procedure was passed an empty ensemble ");
	ux=0.0;
	uy=0.0;
	for(int i=0;i<n;++i)
	{
		if(d->member[i].live)
		{
			ux+=d->member[i].get_double("ux");
			uy+=d->member[i].get_double("uy");
		}
	}
	SlownessVector result;
	result.ux=ux/static_cast<double>(n);
	result.uy=uy/static_cast<double>(n);
	ux /= static_cast<double>(n);
	uy /= static_cast<double>(n);
	if(hypot(ux,uy)<FLT_EPSILON)
	{
		try {
			/* assume n!= 0 or we don't get here */
			ux=d->member[0].get_double("ux0");
			uy=d->member[0].get_double("uy0");
			SlownessVector u0(ux,uy);
			if(u0.mag()<FLT_EPSILON)
				return(SlownessVector(0.0,0.0,0.0));
			else
				return(SlownessVector(0.0,0.0,u0.azimuth()));
			
		} catch (MetadataGetError& mde)
		{
			cerr << "Warning: slowness_average. "
				<< "ux0 and uy0 not defined. using 0"
				<<endl;
			return(SlownessVector(0.0,0.0,0.0));
		}
	}
	else
	{
		return(SlownessVector(ux,uy));
	}
}
/* Extracts du for this enseble.  This procedure should never to extracted
from this program without major modification.  REason is it make two 
extreme assumptions appropriate here, but anything but general.  
(1)  assume slowness vector attributes extracted from ensemble member 
objects exist and we don't need an error handler for metdata gets
(2) all members have the same delta u so result can be extracted from
member 0.  
A less obvious assumption is that the ensemble has data or the request for
member 0 would throw an exception.
*/
SlownessVector EnsembleDeltaSlow(ThreeComponentEnsemble *d)
{
	double ux,uy;
	ux=d->member[0].get_double("ux");
	uy=d->member[0].get_double("uy");
	ux-=d->member[0].get_double("ux0");
	uy-=d->member[0].get_double("uy0");
	return(SlownessVector(ux,uy));
}

/* Trivial function to initialize coherence image to constant negative values */
void initialize_cohimage(GCLvectorfield3d& f)
{
	const double cohundefined(-1.0);
	for(int i=0;i<f.n1;++i)
	  for(int j=0;j<f.n2;++j)
	    for(int k=0;k<f.n3;++k)
	      for(int l=0;l<f.nv;++l) 
	      {
		f.val[i][j][k][l]=cohundefined;
	      }
}


/* This procedure has similarities to the += operator but instead of adding
values it updates cells with max values.  This allows accumulation of a 3d grid
with maximum coherence of stacks moved to proper position in space. */
void accumulate_cohdata(GCLvectorfield3d& pwcomp, GCLvectorfield3d& cohimage)
{
	cohimage.reset_index(); 
	pwcomp.reset_index();
	int i,j,k,l;
	int ierr;
	double *valnew;
	/* Here we assume these grids are congruent so we don't need to have
	to deal with any geographic points */
	Cartesian_point cx;
	for(i=0;i<cohimage.n1;++i)
	  for(j=0;j<cohimage.n2;++j)
	    for(k=0;k<cohimage.n3;++k)
	    {
		cx.x1=cohimage.x1[i][j][k];
		cx.x2=cohimage.x2[i][j][k];
		cx.x3=cohimage.x3[i][j][k];
		ierr=pwcomp.lookup(cx.x1,cx.x2,cx.x3);
		// In this case silently do nothing unless lookup succeeded.
		if(ierr==0)
		{
			valnew=pwcomp.interpolate(cx.x1,cx.x2,cx.x3);
			for(l=0;l<cohimage.nv;++l)
			{
			    /* We bypass negative interpolated values because they can
			    * only be interpolation artifacts */
			    if(valnew[l]>0.0) 
			    {
				if(valnew[l]>cohimage.val[i][j][k][l])
					cohimage.val[i][j][k][l]=valnew[l];
			    }
			}
			delete [] valnew;
		}
	    }
	
}
bool SEISPP::SEISPP_verbose(false);

int main(int argc, char **argv)
{
        //Dbptr db,dbv;
	string Pmodel1d_name;
	const string pvfnm("P"), svfnm("S");
	ThreeComponentEnsemble *pwdata;
	ThreeComponentEnsemble *coh3cens;
	TimeSeriesEnsemble *cohens;
	int is,i,j,k,l;
	int kk;
	string pfin("pwmig");
	dmatrix gradTs;
	int border_pad;
	/* This constant controls when premature cutoff of the travel
	time lag estimates is considered an error.  That is, time are computed
	by working up a ray path from the ray grid.  The points on that path 
	are fixed at n3 of the raygrid.  Model inconsistencies can cause a
	mismatch of the integration along that path compared to other ray
	segments.  When number of points is less than N3_FRACTION_ERROR*n3 
	a seismogram will be dropped from projection with an error posted. */
	const double N3_FRACTION_ERROR(0.9);

        ios::sync_with_stdio();
	/* Initialize these to invalid numbers as a error check against
	inconsistent use in parallel environment */
        int rank(-1), np(-1);


        if(argc < 3) usage();
	/* We will open this file immediately because if that fails we 
	should exit right away */
	FILE *flistfp=fopen(argv[1],"r");
	if(flistfp==NULL)
	{
		cerr << "Cannot open list of pwstack output files = "<<argv[2]<<endl;
		usage();
	}
        string outdir(argv[2]);
        if(makedir(argv[2]))
        {
            cerr << "makedir failed to create output directory = "
                << outdir<<endl;
            usage();
        }
	string dfile;  // used repeatedly below for data file names

        for(i=3;i<argc;++i)
        {
		string sarg(argv[i]);
		if(sarg=="-V")
                        usage();
		else if(sarg=="-v")
        	{
            		SEISPP::SEISPP_verbose=true;
		}
                else if(sarg=="-rank")
                {
                        ++i;
                        if(i>=argc) usage();
                        rank=atoi(argv[i]);
			if(rank<0) 
			{
				cerr << "Illegal value for -r argument ="
					<< rank<<endl
					<< "Must be nonnegative integer"<<endl;
				usage();
			}
                }
                else if(sarg=="-np")
                {
                        ++i;
                        if(i>=argc) usage();
                        np=atoi(argv[i]);
			if(np<=0) 
			{
				cerr << "Illegal value for -np argument ="
					<< np<<endl
					<< "Must be positive integer"<<endl;
				usage();
			}
                }
                else if(sarg=="-pf")
                {
                        ++i;
                        if(i>=argc) usage();
                        pfin = string(argv[i]);
                }
		else
			usage();
	}

#ifdef MPI_SET
/*
        Initialize the MPI parallel environment, MPI_Init
        must be called before any other MPI call.
        rank -- rank of a specific process
        mp -- the total number of processes. This whole
                process group is called MPI_COMM_WORLD
*/

        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &np);
#else
	/* sanity checks on manual parallel variables */
	if( (np>0) || (rank>=0) )
	{
		if( (np<0) || (rank<0) )
		{
			cerr << "Illegal argument combination.  "
				<< "-rank and -np must both be set if either is set"
				<< endl;
			usage();
		}
		else if(rank>(np-1))
		{
			cerr << "Illegal argument combination.  "
				<< "Value for -rank must be >= -np"<<endl;
			usage();
		}
	}
	else
	{
		/* single processor mode set if we end up here.*/
		np=1;
		rank=0;
	}
#endif
	try {
		PfStyleMetadata control=pfread(pfin);
		border_pad = control.get_int("border_padding");
		double zpad = control.get_double("depth_padding_multiplier");
		if( (zpad>1.5) || (zpad<=1.0) )
		{
			cerr << "Riduculous value for depth_padding_multiplier parameter="<<zpad
				<< endl
				<< "Must be a number between 1 and 1.5"<<endl;
			exit(-1);
		}
		string fielddir=control.get_string("output_field_directory");
		// Make sure this directory exists
		if(makedir(const_cast<char *>(fielddir.c_str())))
		{
			cerr << "Failure trying to create directory for output fields = "
				<< fielddir << endl;
			exit(-1);
		}
		/* This is used to make sure name used will not overflow attribute field
		in db tables.  15 is the current size of the string field in the db.
		We compute it based on methods used to create unique names here.
		The 10 comes from size of "_data" and allowing for 5 characters in
		"_evid".  We warn for +2, abort if dfilebase is larger than this number */
		const int dfbase_max_size(15-10);
		string dfilebase=control.get_string("output_filename_base");
		int dfbsize=dfilebase.length();
		if(dfbsize>dfbase_max_size)
		{
			cerr << "Warning:  output_filename_base name = "
                                        << dfilebase<<" is too long"<<endl
                                        << "Recommended maximum length = "
                                        << dfbase_max_size<<endl
                                        << "Longer strings can overflow key field in output db"
                                        <<endl;
			if(dfbsize>(dfbase_max_size+2))
			{
				cerr << "Fatal:  this name is far to long.  It must be changed"
					<<endl;
				exit(-1);
			}
			else
			{
				cerr << "Be warned that data for any evid with more than "
					<< dfbase_max_size + 5 - dfbsize <<" digits "
					<< "may generate an error and not be saved"
					<<endl;
			}
		}
		int evid;  // output file name is build from dfilebase+"_"+evid
		bool use_depth_variable_transformation
			=control.get_bool("use_depth_variable_transformation");
		double zmax=control.get_double("maximum_depth");
		double tmax=control.get_double("maximum_time_lag");
		double dux=control.get_double("slowness_grid_deltau");
		double dt=control.get_double("data_sample_interval");
		int zdecfac=control.get_int("Incident_TTgrid_zdecfac");
		double duy=dux;
		double dumax=control.get_double("delta_slowness_cutoff");
		double dz=control.get_double("ray_trace_depth_increment");
		//
		// added Jan 2007 after recognition dweight and domega
		// were nearly invariant for a given slowness and 
		// were prone to creating artifacts at velocity discontinuities
		//
		bool rcomp_wt;
		rcomp_wt=control.get_bool("recompute_weight_functions");
		int nwtsmooth=control.get_int("weighting_function_smoother_length");
		bool smooth_wt;
		if(nwtsmooth<=0)
			smooth_wt=false;
		else
			smooth_wt=true;
		/* Parameters for handling elevation statics. */
		bool ApplyElevationStatics=control.get_bool("apply_elevation_statics");
		double static_velocity
			=control.get_double("elevation_static_correction_velocity");
		bool use_grt_weights;
		use_grt_weights=control.get_bool("use_grt_weights");
		/* When set domega weights are dropped */
		bool stack_only;
		stack_only=control.get_bool("stack_only");  
		bool save_partial_sums;
		save_partial_sums=control.get_bool("save_partial_sums");
		string Pmodel3d_name=control.get_string("P_velocity_model3d_name");
		string Smodel3d_name=control.get_string("S_velocity_model3d_name");
		GCLscalarfield3d Up3d(Pmodel3d_name);
		GCLscalarfield3d Us3d(Smodel3d_name);
		if(SEISPP_verbose)
		{
			cout << "P velocity model"<<endl;
			cout << Up3d;
			cout << "S velocity model"<<endl;
			cout << Us3d;
		}
/*
		Hack fix for test model.  Units wrong
                Retained in case I ever need to run that data through
                this program again for testing.
cout << "WARNING:  Using special version for simulation data with velocity "
 << "scaling problem"<<endl;
		Up3d *= 0.001;
		Us3d *= 0.001;
*/
		// 
		// For this program we need to convert velocities to slowness.
		// This function called on P and S models does this in a procedural
		// way by altering the field variables in place.
		//
		VelocityFieldToSlowness(Up3d);
		VelocityFieldToSlowness(Us3d);
		// The travel time calculator using a path integral 
		// can fail from rounding errors if the velocity model
		// is not absolutely registered to be at or above the
		// reference ellipsoid.  Here we force the model to be
		// properly registered with a fudge factor read from
		// the parameter file.
		double dzshift=control.get_double("Velocity_model_surface_tolerance");
		double dz0;
		dz0=fudge_model_upward(Up3d,dzshift);
		if(dz0>0.0)
			cout << "P wave 3D model shifted upward by "
				<< dz0 
				<< " km to avoid travel time calculator errors"
				<<endl;
		dz0=fudge_model_upward(Us3d,dzshift);
		if(dz0>0.0)
			cout << "S wave 3D model shifted upward by "
				<< dz0 
				<< " km to avoid travel time calculator errors"
				<< endl;
//DEBUG
//MatlabProcessor mp(string("zemlya"));
//MatlabProcessor mp(stdout);
/*
cout << "Display P velocity model horizontal slices"<<endl;
HorizontalSlicer(mp,Up3d);
cout << "Display S velocity model horizontal slices"<<endl;
sleep(5);
HorizontalSlicer(mp,Us3d);
*/
		string Pmodel1d_name=control.get_string("P_velocity_model1d_name");
		string Smodel1d_name=control.get_string("S_velocity_model1d_name");
		VelocityModel_1d Vp1d,Vs1d;
		if(Pmodel1d_name=="derive_from_3d")
		    //Caution this procedure assumes Up3d is slowness
		    Vp1d=DeriveVM1Dfrom3D(Up3d);
		else
                    Vp1d=VelocityModel_1d(Pmodel1d_name,string("plain"),
                            string("P"));
		if(Smodel1d_name=="derive_from_3d")
		    Vs1d=DeriveVM1Dfrom3D(Us3d);
		else
                    Vs1d=VelocityModel_1d(Smodel1d_name,string("plain"),
                            string("S"));
		string parent_grid_name=control.get_string("Parent_GCLgrid_Name");
                GCLgrid parent(parent_grid_name);

		// This loads the image volume assuming this was precomputed with
		// makegclgrid

		string stack_grid_name=control.get_string("stack_grid_name");
                GCLgrid3d *imggrid=new GCLgrid3d(stack_grid_name);
                /* Create the vectorfield from this pattern with 5 components
                   per node point.   Then delete the grid */
		GCLvectorfield3d migrated_image(*imggrid,5);
                delete imggrid;
                migrated_image.zero();
		/* This group of parameters relate to storing coherence values
                in a separate 4-vector field.  This was derived from an earlier
                inappropriate approach to put coherence weighting inside this
                program.  Decided that was a bad idea, but I note that here to
                remind me or any future maintainers there could be relics of that
                misguided idea.  Now we require a new grid that is assumed to be
		heavily decimated relative to the image grid.  For this reason
		we include a smoothing parameter that reduces errors when the 
		result is mapped to depth and then decimated. */
		string cohgridname=control.get_string("coherence_grid_name");
                GCLgrid3d *cohgrdtmp=new GCLgrid3d(cohgridname);
		GCLvectorfield3d cohimage(*cohgrdtmp,4);
                delete cohgrdtmp;
		initialize_cohimage(cohimage);
		int cohsl=control.get_int("coherence_smoother_length");
		int cohdecfac=control.get_int("coherence_raygrid_decimation");
// DEBUG -- used ONLY for partial sum accumulation.  delete next line otherwise
/*
		GCLvectorfield3d psum(migrated_image);
		remap_grid(dynamic_cast<GCLgrid3d&>(psum),
			dynamic_cast<BasicGCLgrid&>(parent));
*/
		/* Silently remap all grids that are not congruent with
		the parent.  This speeds execution by avoiding excess
		call to geographical conversions. */
		if(dynamic_cast<BasicGCLgrid&>(migrated_image)
			!= dynamic_cast<BasicGCLgrid&>(parent))
		{
			remap_grid(dynamic_cast<GCLgrid3d&>(migrated_image),
				dynamic_cast<BasicGCLgrid&>(parent));
		}
		if(dynamic_cast<BasicGCLgrid&>(Up3d)
			!= dynamic_cast<BasicGCLgrid&>(parent))
		{
			remap_grid(dynamic_cast<GCLgrid3d&>(Up3d),
				dynamic_cast<BasicGCLgrid&>(parent));
		}
		if(dynamic_cast<BasicGCLgrid&>(Us3d)
			!= dynamic_cast<BasicGCLgrid&>(parent))
		{
			remap_grid(dynamic_cast<GCLgrid3d&>(Us3d),
				dynamic_cast<BasicGCLgrid&>(parent));
		}
		if(dynamic_cast<BasicGCLgrid&>(cohimage)
			!= dynamic_cast<BasicGCLgrid&>(parent))
		{
			remap_grid(dynamic_cast<GCLgrid3d&>(cohimage),
				dynamic_cast<BasicGCLgrid&>(parent));
		}
		// Let's make sure these are initialized
		migrated_image.zero();
		
		/* Previous version had a long string of db manipulations here
		This version uses a special file to access data from pwstack.
		The main loop is over file names.  */
		int filecount=0;  /* needed in parallel mode*/
		char fname_base[128];
		bool skip_this_event(false);	
		double totalruntime=now();
		while(fscanf(flistfp,"%s",fname_base)==1)
		{
		/* When using paralle mode this skips files that aren't assigned to this
		processor.  The mod operator with number of processors (np) is 
		a simple way to do this. In single processor mode this 
		does nothing because np=1 and rank=0 */
			if(filecount%np == rank)
			{
			string dfname=string(fname_base)+DataFileExtension;
			PwmigFileHandle datafh(dfname,true,false);
			cout << "Processing pwstack file base name="<<dfname<<endl;
			/* Note the use of an auto_ptr allows me to put these
			inside this conditional in a clean way.  Note the auto_ptr
			further allows automatic deletion when changed with 
			operator = */
			auto_ptr<PwmigFileHandle> cohfh,cohfh3c;
			string coh3cfname=string(fname_base)+Coh3CExtension;
			string cohfname=string(fname_base)+CohExtension;
			cohfh3c=auto_ptr<PwmigFileHandle>
						(new PwmigFileHandle(coh3cfname,true,false));
			cohfh=auto_ptr<PwmigFileHandle>
						(new PwmigFileHandle(cohfname,true,true));
		
			double rundtime;
			rundtime=now();
			cout << "Main loop processing begins at time "<<strtime(rundtime)<<endl;
			// First get the hypo for the current event
			evid=datafh.filehdr.evid;
			Hypocenter hypo(datafh.filehdr.slat,datafh.filehdr.slon,
				datafh.filehdr.sdepth,datafh.filehdr.stime,
				string("tttaup"),string("iasp91"));
			// We build the P wave travel time field once for each
			// event as it is constant for all.  The border_pad parameter
			// is needed because plane waves come from outside the range
			// of the incident rays.  
			auto_ptr<GCLscalarfield3d> TPptr=ComputeIncidentWaveRaygrid(parent,
					border_pad,Up3d,Vp1d,hypo,zmax*zpad,tmax,dt,zdecfac);
			cout << "Time to compute Incident P wave grid "
					<<now()-rundtime<<endl;
			/* Now loop over plane wave components.  The method in 
			the PwmigFileHandle used returns a new data ensemble for
			one plane wave component for each call.  NULL return is
			used in that routine to signal no more data available. */
			while((pwdata=datafh.load_next_3ce())!=NULL)
			{
			// The data are assumed grouped in the ensemble by a common
			// Values do vary and we compute an average
			/* Kind of odd to do this at the top, but useful because 
			ccp stack mode bypasses this loop many times.  There are two procedures
			here.  slowness_average us needed because with large arrays the slowness
			vector of each component is variable and we need a representative average.
			EnsembleDeltaSlow is called second because it the first does error checking
			that is not then needed by the second.  */
			SlownessVector ustack=slowness_average(pwdata);
			SlownessVector deltaslow=EnsembleDeltaSlow(pwdata);
			if(deltaslow.mag()>dumax) 
			{
				delete pwdata;
				continue;  
			}
			cout << "Working on plane wave component with average slowness vector "
				<< "ux="<<ustack.ux<<", uy="<<ustack.uy
				<< ", ur="<<ustack.mag()<<", azimuth="<<deg(ustack.azimuth())<<endl;
			/* This usage requires the coherence and data files contain
			the same number of entries.  This should always happen unless
			pwstack fails.  For now we trust this is so and don't test
			for that condition.  May want to change this. */
			coh3cens=cohfh3c->load_next_3ce();
			cohens=cohfh->load_next_tse();
			cout << "Elapsed time to finish reading data "
				<<now()-rundtime<<endl;
#ifdef MATLABDEBUG
/* To examine P travel time volume.  Copied from MatlabGCLgrid*/
const string volname("F");
const string slicename("f");
const string merge_command("F=cat(3,F,f);");
               dmatrix slice(TPptr->n1,TPptr->n2);
               for(k=0,kk=TPptr->n3-1;k<TPptr->n3;++k,--kk)
                {
                        for(i=0;i<TPptr->n1;++i)
                                for(j=0;j<TPptr->n2;++j)
                                {
                                                slice(i,j)=TPptr->val[i][j][kk];
                                }
                        if(k==0)
                        {
                                mp.load(slice,volname);
                        }
                        else
                        {
                                mp.load(slice,slicename);
                                mp.process(merge_command);
                        }
                }
		mp.run_interactive();

/*
string chans[3]={"x1","x2","x3"};
for(i=0;i<pwdata->member.size();++i)
{
int itest,jtest;
double smax=0;
for(jtest=0;jtest<3;++jtest) for(itest=0;itest<pwdata->member[i].ns;++itest) 
		smax=max(smax,pwdata->member[i].u(jtest,itest));
cerr << "smax="<<smax<<endl;
	mp.load(pwdata->member[i],"s");
	mp.process(string("subplot(3,1,1),plot(s(1,:));")
		+string("subplot(3,1,2),plot(s(2,:));")
		+string("subplot(3,1,3),plot(s(3,:));") );
	mp.process(string("pause(1);"));
}
mp.load(*pwdata,chans);
mp.process(string("figure;wigb(x1);figure;wigb(x2);figure;wigb(x3);"));
mp.run_interactive();
*/
#endif
			int gridid=pwdata->member[0].get_int("gridid");

			// A necessary bailout
			if(dt!=pwdata->member[0].dt)
			{
				cerr << "pwmig(WARNING):  evid="
					<< evid
					<< "sample rate mismatch. "<<endl
					<< "expected dt="<<dt
					<< "found dt="<<pwdata->member[0].dt
					<<endl
					<< "Skipping this event.  Change data_sample_interval parameter"
					<< " and rerun this event"<<endl;
				skip_this_event=true;
				delete pwdata;
				break;
			}
			Ray_Transformation_Operator *troptr;
			string gridname;
			GCLgrid3d *grdptr;
			//
			// This builds a 3d grid using ustack slowness rays with
			// 2d grid parent as the surface grid array
			// use first member of ensemble to get dt
			// VPVS is used to scale tmax to avoid ray truncation
			// dt scaling could be done differently, but using
			// VPVS approximately keeps ray sample interval near
			// data sample interval
			const double VPVS(1.9);  // higher than normal, but better large here
			grdptr =  Build_GCLraygrid(true,parent,ustack,hypo,
				Vs1d,zmax,VPVS*tmax,dt*VPVS);
			GCLgrid3d& raygrid=*grdptr;  // convenient shorthand because we keep changing this
			int n30;  // convenient since top surface is at n3-1
			n30 = raygrid.n3 - 1;
			int SPtime_SIZE_MIN=static_cast<int>(N3_FRACTION_ERROR*static_cast<double>(n30));
			// create work spaces for accumation of this component
			// This is a large memory model.  I'll use it until it proves
			// intractable
			// Below assumes the val arrays in these fields
			// have been initialized to zero.
			GCLvectorfield3d pwdgrid(raygrid,5);
			GCLgrid3d *gtmp=decimate(raygrid,1,1,cohdecfac);
			GCLvectorfield3d pwcohgrid(*gtmp,4);
			pwcohgrid.zero();
			delete gtmp;


			// loop through the ensemble associating each trace with a 
			// point in the raygrid and interpolating to get time to depth.

			// 
			// this boolean is needed as part of optional setting 
			// recomputing weighting functions for each ray path.
			// When these weighting functions are computed only
			// once for each plane wave component (rcomp_wf false)
			// this seems the only safe and maintainable way to 
			// make sure these functions are always initialized.
			// They are way down in the code with too many 
			// potential errors that could cause a break condition
			// 
			bool weight_functions_set=false;
			// Warning:  n3 must not change in the loop below
			int n3=raygrid.n3;
			vector<double> domega_ij(n3),dweight_ij(n3);
			for(is=0;is<pwdata->member.size();++is)
			{
				// convenient shorthand variables.  ns is data length
				// while n3 is the ray path onto which data are mapped
				int ns=pwdata->member[is].ns;
				double zmaxray,tmaxray;
				vector<double> Stime(n3), SPtime(n3);
				double t0;

				dt=pwdata->member[is].dt;
				// first decide in which grid cell to place this seismogram 
				i = pwdata->member[is].get_int("ix1");
				j = pwdata->member[is].get_int("ix2");
				if(ApplyElevationStatics)
				{
					/* We assume elevation has been posted as elev.  This is done
					by PwmigFileHandle methods on loading, but if data handling method
					changes this needs to be watched.  Be aware this elevation is produced
					in pwstack as a weighted average of elevations of summed traces */
					double pselev=pwdata->member[is].get_double("elev");
					t0=pwdata->member[is].t0;
					ApplyGeometricStatic(dynamic_cast<BasicTimeSeries *>(&(pwdata->member[is])),
						static_velocity,pselev);
					if(SEISPP_verbose)
						cout << "Applying static time shift = "
							<< pwdata->member[is].t0 - t0<<endl;
				}
				t0=pwdata->member[is].t0;
				if( (i<0) || (i>raygrid.n1) || (j<0) || (j>raygrid.n2) )
				{
					cerr << "grid index of data = ("
						<< i << ","<< j 
						<< "is outside range of grid." <<endl
						<< "Check grid definitions in database and pf"<<endl;
					exit(-1);
				}
				// We need a reference ray for the incident wavefield.
				// This ray is chopped up and transformed to GCLgrid coordinates
				// in the integration loop below.  We compute it here to 
				// avoid constantly recomputing it
				zmaxray = raygrid.depth(i,j,0);
				zmaxray *= zpad;  // Small upward adjustment to avoid rubber banding
						// problems in 3d media
				tmaxray=10000.0;  // arbitrary large number.  depend on zmax 

				// Now compute a series of vectorish quantities that 
				// are parallel with the ray path associated with the 
				// current seismogram (i,j).  First we compute the S wave
				// travel time down this ray path (i.e. ordered from surface down)
				dmatrix *pathptr=extract_gridline(raygrid,i,j,raygrid.n3-1,3,true);
				Stime = compute_gridttime(Us3d,i,j,Vs1d,raygrid,*pathptr);
				// Now we compute the gradient in the S ray travel time
				// for each point on the ray.  Could have been done with 
				// less memory use in the loop below, but the simplification
				// it provides seems useful to me
				gradTs = compute_gradS(raygrid,i,j,Vs1d);
//DEBUG
#ifdef MATLABDEBUG
/*
mp.load(gradTs,string("gradts"));
mp.load(&(Stime[0]),Stime.size(),string("stime"));
cout << "Loaded gradts and stime"<<endl;
cout << "plotting gradts"<<endl;
mp.process(string("plot3c(gradts);"));
mp.process(string("pause(1)"));
mp.load(*pathptr,string("spath"));
cout << "plotting S wave path components for (i,j)="<<i<<","<<j;
mp.process(string("plot3c(spath);"));
*/
#endif
				dmatrix gradTp(3,n3);
				dmatrix nup(3,n3);
				vector<double> zP(n3);
				// Now loop over each point on this ray computing the 
				// p wave path and using it to compute the P time and
				// fill the vector gradP matrix
				// Note the double index, one running forward (k) and
				// a second (kk) running backward.  This complication is
				// caused by raygrid having upward oriented curves while
				// the S ray path (pathptr) is oriented downward.  
				// Only need to compute P time to surface once for 
				// each ray so we compute it before the loop below
				double Tpr=compute_receiver_Ptime(raygrid,i,j,*TPptr,hypo);
				double tlag,Tpx;
				bool needs_padding;
				int padmark=raygrid.n3-1;
				bool tcompute_problem;   // set true if tt calculation fails away from edges 
				/* It is a good idea to initialize these before each raty path */
				TPptr->reset_index();
				SPtime.clear();
				tcompute_problem=false;
				needs_padding=false;
				for(k=0,kk=raygrid.n3-1;k<raygrid.n3;++k,--kk)
				{
					vector<double>nu;
					double vp;

					nu = compute_unit_normal_P(*TPptr,raygrid.x1[i][j][kk],
						raygrid.x2[i][j][kk], raygrid.x3[i][j][kk]);
					for(l=0;l<3;++l) nup(l,kk)=nu[l];
					// zP and gradTp are computed in reverse order
					zP[k]=raygrid.depth(i,j,kk);
					vp=Vp1d.getv(zP[k]);
					for(l=0;l<3;++l) gradTp(l,k)=nu[l]/vp;
					/* This section used to be a procedure.  Inlined for 
					speed during a cleanup June 2012 */
					int error_lookup;
					error_lookup=TPptr->lookup(raygrid.x1[i][j][kk],
						raygrid.x2[i][j][kk],raygrid.x3[i][j][kk]);
					switch(error_lookup)
					{
					case 0:
						Tpx=TPptr->interpolate(raygrid.x1[i][j][kk],
                                                   raygrid.x2[i][j][kk],raygrid.x3[i][j][kk]);
						break;
					case 1:
					/* This means the point falls outside the P ray grid.
					Normally that means we should just break the loop.
					Issue a warning only if the kk index is small indicating
					the ray path is very short  */
						needs_padding=true;
						padmark=k;
						break;
					case -1:
					default:
					/* This is a convergence error in lookup and needs to be
					flagged as an error */
						tcompute_problem=true;
						needs_padding=true;
						padmark=k;
					}
					if(tcompute_problem || needs_padding) break;
					tlag=Tpx+Stime[k]-Tpr;
					SPtime.push_back(tlag);
				}

				// skip this ray if there were travel time computation problems
				if(tcompute_problem)
				{
					cerr << "Warning:  slowness gridid "<< gridid 
						<< ", grid position index ("
						<< i << ","
						<< j << ").  GCLscalarfield3d lookup failed at ray index="
						<< k <<endl
						<< "Data from failed ray index downward will be zeroed"<<endl
						<< "This may leave ugly holes in the output image."
						<<endl;

				}
				if (needs_padding)
				{
					if(padmark==0)
					{
						cerr << "Warming:   gridid "<< gridid
                                                << ", grid position index ("
                                                << i << ","
                                                << j << ")." <<endl
                                                << "P time interpolation failed at earth's surface.  "<<endl
                                                << "This should not happen and may cause holes in the output."<<endl
                                                << "This trace will be projected as zeros"<<endl;
					}
					else
					{
					/* Arbitrarily increment the times by dt intervals.  Will 
					zero data beyond pad mark below.  Not the most efficient way
					to do this, but don't expect this to be that common of an issue*/
						double tpadding0=Stime[padmark-1];
						for(k=padmark;k<raygrid.n3;++k)
							Stime.push_back(tpadding0+dt*static_cast<double>(k-padmark+1));
					}
				}
				// We now interpolate the data with tlag values to map
				// the data from time to an absolute location in space
				// Note carefully SPtime and work sense is inverted from
				// the raygrid.  i.e. they are oriented for a ray from the surface
				// to the bottom.  Below we have to reverse this
				dmatrix work(3,raygrid.n3);
				linear_vector_regular_to_irregular(t0,dt,pwdata->member[is].u,
					&(SPtime[0]),work);
				/* Zero all data that was padded */
				if(needs_padding)
				{
					for(k=padmark;k<raygrid.n3;++k)
						for(l=0;l<3;++l)  work(l,k)=0.0;
					//DEBUG
/*
					dmatrix wtr=tr(work);
					cout << "Padded data vector after interpolation"<<endl
						<< wtr <<endl;
*/
				}

#ifdef MATLABDEBUG
//DEBUG
cout << "Input data number of samples="<<pwdata->member[is].ns<<endl;
cout << "DAta before interpolation" << endl;
for(int ifoo=0;ifoo<pwdata->member[is].ns;++ifoo)
	cout << pwdata->member[is].time(ifoo) << " "
		<< pwdata->member[is].u(0,ifoo) << " "
		<< pwdata->member[is].u(1,ifoo) << " "
		<< pwdata->member[is].u(2,ifoo) << endl;
mp.load(pwdata->member[is],string("de"));
mp.load(&(SPtime[0]),SPtime.size(),string("sptime"));
mp.load(work,string("di"));
cerr << "Loaded sptime and interpolated data (work matrix)"<<endl;
mp.process(string("plot6c(de,sptime,di);pause(1.0);"));
#endif
				dmatrix raycoh(4,coh3cens->member[is].ns);
				int ncohcopy=coh3cens->member[i].ns;
				/* excessively paranoid, but murphy's law */
				if(cohens->member[is].ns != ncohcopy)
				{
					cerr << "Warning:  inconsistent coherence data size"<<endl
						<< "Three-C data size="<<ncohcopy
						<< " while all component sum size="
						<< cohens->member[is].ns<<endl;
					ncohcopy=min(cohens->member[is].ns,ncohcopy);
					cerr << "Set to minimum="<<ncohcopy<<endl;
				}
				/* This component coherence into first 3 rows and sum in 4 */
				dcopy(ncohcopy,coh3cens->member[is].u.get_address(0,0),3,
					raycoh.get_address(0,0),4);
				dcopy(ncohcopy,coh3cens->member[is].u.get_address(1,0),3,
					raycoh.get_address(1,0),4);
				dcopy(ncohcopy,coh3cens->member[is].u.get_address(2,0),3,
					raycoh.get_address(2,0),4);
				dcopy(ncohcopy,&(cohens->member[is].s[0]),1,
					raycoh.get_address(3,0),4);
				int nout=SPtime.size();
				dmatrix raycohout(4,nout);
				/* we assume t0 and dt are consistent or scalar and vector
				coherence data */
				linear_vector_regular_to_irregular(cohens->member[is].t0,
					cohens->member[is].dt, raycoh,&(SPtime[0]),raycohout);
				/* This nontrivial effort may not be required, but best to 
				have it available until proven to be a big issue. */
				raycohout=running_average(raycohout,cohsl);
				/* Now copy these results to the coherence raygrid.
				Note carefully the reverse order going from raycohout order
				to rayrid order and decimation by cohdecfac */
				for(k=0,kk=pwcohgrid.n3-1;(k<nout) && (kk>=0); 
							k+=cohdecfac,--kk)
				{
				    for(l=0;l<4;++l)
					pwcohgrid.val[i][j][kk][l]=raycohout(l,k);
				}

				//
				// Compute the transformation vector for each 
				// ray point in space.  This comes in two flavors.
				// A fast, inexact version and a exact slow version
				//
				if(use_depth_variable_transformation)
				{
					// the slow, exact method
					troptr=new Ray_Transformation_Operator(
						parent,
						*pathptr,
						ustack.azimuth(),
						nup);
				}
				else
				{
					// the fast approximation
					troptr=new Ray_Transformation_Operator(
						parent,
						*pathptr,ustack.azimuth());
				}
				Ray_Transformation_Operator& trans_operator=*troptr;
				work=trans_operator.apply(work);
				// done with these now
				delete pathptr;
				delete troptr;

				// This computes domega for a ray path in constant dz mode
				// We would have to interpolate anyway to mesh with 
				// time data choose the faster, coarser dz method here
				//
				if(rcomp_wt || !weight_functions_set)
				{
				  /* This is a gross inefficiency in stack_only but since I only expect it to be
				  used for CCP stacking equivalent, this should not be a big deal.  Probably should
				  do it right some day */
				  if(stack_only)
				  {
				    domega_ij.clear();
				    for(k=0;k<n3;++k) domega_ij.push_back(1.0);
				  }
				  else
				  {
				    /* This is the normal block for computing solid angles */
				    domega_ij=compute_domega_for_path(ustack,dux,duy,
					Vs1d, zmaxray,dz,
					raygrid, i, j, gradTp,zP);
				  }
				  if(use_grt_weights && (!stack_only) )
					dweight_ij=compute_weight_for_path(gradTp,gradTs);
				  else
					for(k=0;k<n3;++k)dweight_ij[k]=1.0;
				  if(smooth_wt && (!stack_only))
				  {
					domega_ij=running_average(domega_ij,nwtsmooth);
					// Unnecessary when not using grt weighting
					if(use_grt_weights)
					  dweight_ij=running_average(dweight_ij,nwtsmooth);
				  }
				  weight_functions_set=true;
				}
					
				//
				// copy transformed data to vector field
				// copy weights and domega at same time
				//
				for(k=0,kk=raygrid.n3-1;k<raygrid.n3;++k,--kk)
				{
					for(l=0;l<3;++l)
					{
						pwdgrid.val[i][j][k][l]=work(l,kk)
							*dweight_ij[kk]*domega_ij[kk];
					}
					pwdgrid.val[i][j][k][3]=domega_ij[kk];
					pwdgrid.val[i][j][k][4]=dweight_ij[kk];
				}
#ifdef MATLABDEBUG
//DEBUG
dmatrix dproj(3,n3);
for(k=0;k<n3;++k)
	for(l=0;l<3;++l) dproj(l,k)=pwdgrid.val[i][j][k][l];
mp.load(dproj,string("d"));
mp.process(string("plot3c(d);pause(0.1);"));
#endif
			}
			delete pwdata;
/*
cout << "plane wave data component" << endl;
bool debugexit;
debugexit=true;
//cout << pwdgrid;
GCLscalarfield3d *sfptr;
sfptr=extract_component(pwdgrid,0);
//output_gcl3d_to_vtksg(*sfptr,"component0.vtk");
SliceX1(mp,*sfptr);
delete sfptr;
sfptr=extract_component(pwdgrid,1);
//output_gcl3d_to_vtksg(*sfptr,"component1.vtk");
SliceX1(mp,*sfptr);
delete sfptr;
sfptr=extract_component(pwdgrid,2);
//output_gcl3d_to_vtksg(*sfptr,"component2.vtk");
SliceX1(mp,*sfptr);
delete sfptr;
*/
			// last but not least, add this component to the stack
			//
			cout << "Elapsed time to compute pwdgrid="<<now()-rundtime<<endl;
			migrated_image += pwdgrid;
//cout << pwcohgrid<<endl;
			accumulate_cohdata(pwcohgrid,cohimage);
//cout << "image grid after accumulate:"<<endl;
//cout << cohimage;
			if(save_partial_sums)
			{
                                // Oddity note:
				// This will write final result twice 
				// with different names.  
				dfile=MakeDfileName(dfilebase
					+string("_psum"),gridid+1000);
                                migrated_image.save(dfile,outdir);
			}
			cout << "Total time for this plane wave component="<<now()-rundtime<<endl;
//DEBUG  save partial sums
/*
                dfile=MakeDfileName(dfilebase+string("_psum"),gridid+1000);
		psum.zero();
*/
/*  DEBUG Scaffold to retain for now.  Loads ramp to test grid summation
		// test setting pwdgrid values all to zero
		int id,jd,kd,ld;
		for(id=0;id<pwdgrid.n1;++id)
		 for(jd=0;jd<pwdgrid.n2;++jd)
		  for(kd=0;kd<pwdgrid.n3;++kd)
		   for(ld=0;ld<3;++ld) pwdgrid.val[id][jd][kd][ld]
						=static_cast<double>(kd%100);
*/
		//psum += pwdgrid;
#ifdef MATLABDEBUG
/*
const string volname("F");
const string slicename("f");
const string merge_command("F=cat(3,F,f);");
               dmatrix slice(psum.n1,psum.n2);
               for(k=0,kk=psum.n3-1;k<psum.n3;++k,--kk)
                {
                        for(i=0;i<psum.n1;++i)
// Select x1 for display
                                for(j=0;j<psum.n2;++j)
                                {
                                                slice(i,j)=psum.val[i][j][kk][0];
                                }
                        if(k==0)
                        {
                                mp.load(slice,volname);
                        }
                        else
                        {
                                mp.load(slice,slicename);
                                mp.process(merge_command);
                        }
                }
		mp.run_interactive();
*/
#endif
                //psum.dbsave(db,"",fielddir,dfile,dfile);


			delete grdptr;
			rundtime=now();

		} // Bottom of plane wave component loop
		//
		// Save results
		// Second arg as empty string causes parent grid 
		// to not be saved and only the migrated data are saved
		// In all cases we force the field name in the output to
		// be the same as the output data file name, dfile
		// dfile is generated as base+tag+evid
		//

		if(!skip_this_event)
		{
			dfile=MakeDfileName(dfilebase+string("_data"),evid);
			migrated_image.save(dfile,fielddir);
			//
			// zero field values so when we loop back they can
			// be reused
			//
			migrated_image.zero();
			dfile=MakeDfileName(dfilebase+string("_coh"),evid);
			cohimage.save(dfile,fielddir);
		}
		} // Bottom of np%rank conditional 
		++filecount;
		skip_this_event=false;
		} // bottom of event loop
		totalruntime = now() - totalruntime;
		cout << "Finished normally after processing "<<filecount
			<< "events."<<endl
			<< "Elapsed time (sec)="<<totalruntime<<endl;
	}
	catch (exception& ge)
	{
		cerr << "Child of generic exception was thrown"
                    << endl << "Message thrown:  "
                    << ge.what()<<endl;
	}
	catch (SeisppError& seer)
	{
		seer.log_error();
	}
	catch (int ierr)
	{
                cerr << "Something threw a simple int exception of "
                    << ierr<<endl;
	}
	catch (...)
	{
            cerr << "Something threw an undefined exception type"
                <<endl;
	}
}


