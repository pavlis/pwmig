#include <math.h>
#include <float.h>
#include <fstream>
#include "stock.h"
#include "dbpp.h"
#include "TimeSeries.h"
#include "Metadata.h"
#include "seispp.h"
#include "gclgrid.h"
#include "SEGY2002FileHandle.h"
#include "GenericFileHandle.h"
#include "LatLong-UTMconversion.h"
/* These are functions in pfutils.cc.  Eventually they will probably land
   in Metadata.h */
//string pftbl2string(Pf *pf, const char* tag);
//list<string> pftbl2list(Pf *pf, const char* tag);
using namespace std;
using namespace SEISPP;
/* This is a less general version of a similar program called extract_section.
   This program blindly dumps a GCLvectorfield writing the output as seismic
   unix data made to look like 3D seismic data.  It does this by blindly
   extracting data as original grid point data.  Output is in x1, x2 order with
   each seismogram having 0 at the surface (reflection convention).

   Header data to write is frozen and will evolve - writing this as a design
   document
   */

void usage()
{
	cerr << "petrel_export gclfield_filename "
	  << "Read a gclfield file with base name defined by arg1"<<endl
		<< "[ -o outfile -scalar -pf pffile -v]"<<endl
	      << "Default outfile is called extract_volume.su"<<endl
              << "Use -scalar flag if input is scalar data (default is pwmig output)"<<endl
              << "Examples are tomography models and coherence grids"<<endl;
	exit(-1);
}
/* Workhorse for this program.  Always interpolates data in field f working from
the top down with top defined as gp.   dz is depth sample interval.  Last point
will be at gp.r - n*dz */
TimeSeries ExtractFromGrid(GCLscalarfield3d f,Geographic_point gp0,int nz, double dz)
{
	try{
		/* We gp is a point at the surface.  For the output this becomes the
		first sample of the output seismogram.   That is, we work from the top
		down using gp as the anchor and working downward dz at each step. */
		double r;  // working radius in loop below
		TimeSeries d(nz);  // empty time series - result
		/* This is actually not necessary, but useful since the algorithm below
		has to assume this is true */
		d.live=false;
		Geographic_point gp;
		Cartesian_point cp;
		gp=gp0;
		int k,nset;
		double fval;
		double dzkm=dz/1000.0;   // dz is m convert to km
		for(k=0,nset=0;k<nz;++k)
		{
			gp.r=gp0.r-dzkm*((double)k);
			cp=f.gtoc(gp);
			int iret=f.lookup(cp.x1,cp.x2,cp.x3);
			if(iret)
			{
				/* land here if the lookup failed */
				d.s.push_back(0.0);
			}
			else
			{
				fval=f.interpolate(cp.x1,cp.x2,cp.x3);
				d.s.push_back(fval);
				++nset;
			}
		}
		if(nset>0)
		{
			d.live=true;
			d.put("nsamp",nz);
			d.put("dt",dz);  // note this is in m
			d.put("samprate",1.0/dz);
			d.put("time",0.0);
			d.put("fldr",1);  // not sure this is needed
			d.put("duse",1);
		}
		return d;
	}catch(...){throw;};
}

GCLgrid BuildUTMgrid(GCLscalarfield3d *g, double dx1, double dx2,
	  int RefEllipse, string utmzone)
{
	/* To get the extent of the input grid we compute the great circle path
	distance between the lower and right hand boundaries of the grid.  This
	makes a tacit assumption the input grid is approximately a box or a
	rotated box mapped to a sphere */
	double delta,az;
	Geographic_point gp1,gp2;
	/* lower left corner - the n3-1 oddity is because the top of a standard
	GCLgrid is n3-1 and 0 is at the base */
	gp1=g->geo_coordinates(0,0,g->n3-1);
	gp2=g->geo_coordinates(g->n1-1,0,g->n3-1);
	dist(gp1.lat,gp1.lon,gp2.lat,gp2.lon,&delta,&az);
	/* compute the grid size assumig dx1 and dx2 are in meters */
	double dkm;
	dkm=delta*EQUATORIAL_EARTH_RADIUS;
	int nx1out,nx2out;
	nx1out=(dkm*1000.0)/dx1;
	gp2=g->geo_coordinates(0,g->n2-1,g->n3-1);
	dist(gp1.lat,gp1.lon,gp2.lat,gp2.lon,&delta,&az);
	dkm=delta*EQUATORIAL_EARTH_RADIUS;
	nx2out=(dkm*1000.0)/dx2;
	if(SEISPP_verbose)
	{
		cerr << "petrel_export:   creating a surface grid of size "
		  << nx1out << " X "<<nx2out<<endl;
	}
	/* If the original grid was not rotated force the lower baseline to be
	oriented EW.   Otherwise use the rotation angle.   The later will work
	badly for large areas, but I've never seen a continent scale grid made
	with a rotated baseline - common only at regional scales where the issue
	should not br as extreme */
	double x1_azimuth,x2_azimuth;
	if(fabs(g->azimuth_y)>FLT_EPSILON)
	{
		x1_azimuth = g->azimuth_y + M_PI_2;
		x2_azimuth = g->azimuth_y;
	}
	else
	{
		x1_azimuth = M_PI_2;
		x2_azimuth = 0.0;
	}
	/* These are unit cell vectors for x1 and x2 directions of uniform utm
	coordiante output grid.   Stock trig formulas. */
	dvector utmdx1(2), utmdx2(2);
	utmdx1(0)=dx1*sin(x1_azimuth);
	utmdx1(1)=dx1*cos(x1_azimuth);
	utmdx2(0)=dx2*sin(x2_azimuth);
	utmdx2(1)=dx2*cos(x2_azimuth);
	dvector origin(2);
	origin(0)=0.0;  origin(1)=0.0;
	/* Build the GCLgrid with origin at the 0,0 point of the utm grid = lower
	left top of 3d grid.   number of points is defined by utmgrid size.  The
	extents will not be exactly right but I don't think that will be an issue.*/
	try{
		/*protect this section with a try block mainly to avoid a memory trap
		for absurd grid sizes - strong possibility*/
	  GCLgrid gout(nx1out,nx2out,string("temp"),gp1.lat,gp1.lon,gp1.r,x2_azimuth,
	     dx1*1000.0,dx2*1000.0,0,0);
		int i,j,k;
		dvector x(2);
		Geographic_point gpout;
		for(i=0;i<gout.n1;++i)
		{
			for(j=0;j<gout.n2;++j)
			{
				double latdeg,londeg;
				for(k=0;k<2;++i)
				{
					x(k)=((double)i)*utmdx1(k) + ((double)j)*utmdx2(k);
				}
				/* Note we use the full utmzone specification here but the
				equatorial form later - I don't think this will matter.   */
				UTMtoLL(RefEllipse,x(1),x(0),utmzone.c_str(),latdeg,londeg);
				/* GCLgrid needs coordinates in radians, but this utm converter
				returns coordinates in degrees */
				gpout.lat=rad(latdeg);
				gpout.lon=rad(londeg);
				/* for surface registration */
				gpout.r=r0_ellipse(gpout.lat);
				Cartesian_point cp;
				cp=gout.gtoc(gpout);
				gout.x1[i][j]=cp.x1;
				gout.x2[i][j]=cp.x2;
				gout.x3[i][j]=cp.x3;
			}
		}
		return gout;
	}catch(...){throw;};
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
	int i,j,k;
	ios::sync_with_stdio();
	if(argc<2) usage();
	string infilebase(argv[1]);
  string pffile("extract_volume");
	ofstream outstrm;
	bool out_to_other(false);
	string outfile("petrel_export.sgy");
  bool input_is_scalar(false);
	for(i=4;i<argc;++i)
	{
		string argstr=string(argv[i]);
		if(argstr=="-o")
		{
		    ++i;
                    if(i>=argc) usage();
		    out_to_other=true;
		    outfile=string(argv[i]);
		}
    else if(argstr=="-pf")
    {
      ++i;
      if(i>=argc) usage();
      pffile=string(argv[i]);
    }
		else if(argstr=="-v")
			SEISPP_verbose=true;
    else if(argstr=="-scalar")
      input_is_scalar=true;
		else
		{
			cerr << "Unknown argument = "<<argstr<<endl;
			usage();
		}
	}

  Pf *pf;
  if(pfread(const_cast<char *>(pffile.c_str()),&pf))
  {
		cerr << "pfread failed for pf file="<<pffile<<endl;
    exit(-1);
  }
	try {
      Metadata control(pf);
      double dx1=control.get_double("easting_sample_interval");
      double dx2=control.get_double("northing_sample_interval");
			double dz=control.get_double("depth_sample_interval");
			double nz=control.get_int("number_depth_points_in_output");
      int RefEllipse=control.get_int("UTMReferenceEllipsoidNumber");
      string utmzone=control.get_string("UTMzone");
      bool comp_to_save[3];
      comp_to_save[0]=control.get_bool("save_transverse");
      comp_to_save[1]=control.get_bool("save_radial");
      comp_to_save[2]=control.get_bool("save_vertical");
      if(!(comp_to_save[0] || comp_to_save[1] || comp_to_save[2]))
      {
          cerr << "Fatal error:  Must set one of save parameters in pf true"
                    <<endl;
          exit(-1);
      }
			int icount,ic2save;
			for(icount=0,i=0;i<3;++i)
			{
			  if(comp_to_save[i])
				{
					ic2save=i;
					++icount;
				}
			}
			if(icount>1)
			{
				cerr << "Fatal error:   "<<icount<<" save_component parameters were set true"<<endl
				   << "Set only one of save_radial, save_transverse, and save_vertical true"<<endl;
				exit(-1);
			}
            /* override the above if scalar - hack adaption of this program
               puts scalar value in comp[0] and zeros other components. */
      GenericFileHandle *outhandle;
      GCLscalarfield3d *g;
      if(input_is_scalar)
      {
				g=new GCLscalarfield3d(infilebase);
			}
			else
			{
				/* landing here means vector (pwmig) data */
				GCLvectorfield3d f(infilebase);
				/* the dynamic_cast here is probably not necessary, but shows
				clearly this clones the grid but not the data to build the
				working grid. */
				g=new GCLscalarfield3d(dynamic_cast<GCLgrid3d&>(f));
				g->zero();
				for(i=0;i<g->n1;++i)
						for(j=0;j<g->n2;++j)
								for(k=0;k<g->n3;++k)
										g->val[i][j][k]=f.val[i][j][k][ic2save];
			}
      string xrefstr=pftbl2string(pf,"metadata_cross_reference");
      AttributeCrossReference outxref(xrefstr);
      list<string> tmdlist=pftbl2list(pf,"output_metadata_list");
			/* This is the seispp abstract handle to write to a segy file */
      SEGY2002FileHandle *sgyh;
      pfput_int(pf,(char *)"number_samples",nz);
			/* This is an oddity of segy not defined in the standard.   When reading
			data in depth units petrel assumes units are mm so scaling is roughly
			the same as segy time in microsections.*/
      pfput_int(pf,(char *)"sample_interval",dz*1000.0);
      sgyh=new SEGY2002FileHandle(outfile,tmdlist,pf);
      outhandle=dynamic_cast<GenericFileHandle *>(sgyh);
			GCLgrid gout0(BuildUTMgrid(g,dx1,dx2,RefEllipse,utmzone));
			const int warngridsize(10000);
			if((gout0.n1>warngridsize) || (gout0.n2>warngridsize))
			{
				cerr << "Warning:   grid size is very large"<<endl
				  << "Number of points in x1 direction = "<<gout0.n1<<endl
					<< "Number of points in x2 direction = "<<gout0.n2<<endl
					<< "Are you sure this is what you want? (y to continue):";
				char ques;
				cin >> ques;
				if(ques!='y')exit(-2);
			}
			TimeSeries d;
		  cout << "Starting main loop over surface grid"<<endl;
			/* This makes i (easting) direction inline and northing crossline */
			for(j=0;j<gout0.n2;++j)
			{
				for(i=0;i<gout0.n2;++j)
				{
					Geographic_point gp=gout0.geo_coordinates(i,j);
					d=ExtractFromGrid(*g,gp,nz,dz);
					double easting,northing;
					double lon=d.get_double("rx");
					double lat=d.get_double("ry");
					std::pair<double,double> utmtmp;
					utmtmp=LLtoUTMFixedZone(RefEllipse,lat,lon,
									utmzone.c_str());
					easting=utmtmp.first;
					northing=utmtmp.second;
					/* We define the output to be like zero offset CMP stacked
					reflection data where source and receiver coordinates are equal */
					d.put("rx",easting);
					d.put("ry",northing);
					d.put("sx",easting);
					d.put("sy",northing);
					/* petrel wants numbers greater than 0 for inine and crossline.
					These are written in segy2002 slots, but are ignored at this time
					by petrel. */
					d.put("inline",i+1);
					d.put("crossline",j+1);
					/* These are the default positions for inline and crossline in petrel.
					This is not specified in the standard to my knowledge. */
					d.put("cdp",i+1);  // treated as inline index by petrel
					d.put("tracr",j+1);   // treated as crossline index by petrel
					/*Not sure it is necessary to set these.  2002 standard specifies these
					should be set for three component data */
					switch (ic2save)
					{
							case (0):
									d.put("trid",16);
									break;
							case (1):
									d.put("trid",17);
									break;
							case (2):
							default:
									d.put("trid",15);
					}
					/* This algorithm assumes if d is marked dead the outhandle will
					wrie a null seismogram with the header set to be marked dead.
					The SEGY2002FileHandle version currently in this directory
					does that, but beware if that object is ever placed in a library */
					outhandle->put(d);
				}
			}
		}
		catch (SeisppError& serr)
		{
			serr.log_error();
		}
  	catch(std::exception& sterr)
  	{
    	cerr << sterr.what()<<endl;
  	}
}
