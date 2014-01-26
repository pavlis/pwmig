#include <iostream>
#include <fstream>
#include "stock.h"
#include "pf.h"
#include "dbpp.h"
#include "seispp.h"
#include "gclgrid.h"
using namespace std;
using namespace SEISPP;
void usage()
{
	cerr << "grdtodb grdfile db patterngrid"<<endl
		<<"  Note:  grdfile.pf implied and required"<<endl;
	exit(-1);
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
	if(argc != 4) usage();
	string grdfile_base(argv[1]);
	string dbname(argv[2]);
	string patterngrid(argv[3]);
	string grdconverted=grdfile_base + string(".xyz");
	string pffile=grdfile_base;
	Pf *pf;
	if(pfread(const_cast<char *>(pffile.c_str()),&pf))
	{
		cerr << "pfread failed for file = "<<pffile<<".pf"<<endl;
		cerr << "Required for definition of file ="<<grdconverted<<endl;
	}
	ifstream in;
	try {
		in.open(grdconverted.c_str(),ios::in);
	}
	catch (ios::failure& var)
	{
		cerr << "Open failed on file = " << grdconverted << endl
			<< "ios error message:  "<<endl
			<< var.what()<<endl;
		exit(-1);
	}
	try {
		DatascopeHandle dbh(dbname,false);
		// For now we fetch these parameters from the pf.  
		// Eventually we should have this read grd files directly.
		Metadata control(pf);
		int n1,n2;
		n1=control.get_int("n1");
		n2=control.get_int("n2");
		string gridname=control.get_string("gridname");
		double lat0,lon0,r0,dx1n,dx2n;
		lat0=control.get_double("lat0");
		lon0=control.get_double("lon0");
		lat0=rad(lat0);
		lon0=rad(lon0);
		r0=control.get_double("r0");
		dx1n=control.get_double("dx1n");
		dx2n=control.get_double("dx2n");
		int iorigin,jorigin;
		iorigin=control.get_int("iorigin");
		jorigin=control.get_int("jorigin");
		double zscale=control.get_double("vertical_scale_factor");
		// desirable to make vtk, at least, behave properly.  Usually
		// we will want a surface grid to not intersect with a 3d grid
		// or their color maps get locked together.
		// Note this will be in kilometers
		// Watch for scale interaction
		double zoffset=control.get_double("vertical_offset");
		// This parameter is odd to be taken from control, but 
		// for now it is the path of least resistance.  This MUST
		// be changed if this is ever a released program.
		string griddir=control.get_string("grid_directory");
		// This creates a tentative grid.  Will be altered with 
		// remap procedure
		GCLgrid g(n1,n2,gridname,rad(lat0),rad(lon0),r0,0.0,dx1n,dx2n,iorigin,jorigin);
		// This needs to be more flexible and allow other grid types as patterns
		GCLgrid3d gpat(dbh,patterngrid);
		remap_grid(g,dynamic_cast<BasicGCLgrid&>(gpat));
		// This assumes grd2xyz writes from top down which is inverted
		// from required order for the gclgrid object.  
		int i,j;
		Geographic_point gp;
		Cartesian_point cp;
		double latd,lond;
		double elev;
		for(j=n2-1;j>=0;--j)
			for(i=0;i<n1;++i)
			{
				in >> lond;
				in >> latd;
				in >> elev;
				gp.lon=rad(lond);
				gp.lat=rad(latd);
				// DEMs tabulte elevation in meters 
				gp.r=r0_ellipse(gp.lat)+zoffset+(elev*zscale);
				cp=g.gtoc(gp);
				g.x1[i][j]=cp.x1;
				g.x2[i][j]=cp.x2;
				g.x3[i][j]=cp.x3;
			}
		// Necessary to reset extents variables because of remap_grid
		// in combination with explicitly resetting x1,x2, and x3 arrays
		g.compute_extents();
		g.save(dbh,griddir);
	} 
	catch (std::exception& stex)
	{
		cerr << "Fatal Error"<<endl
                    << stex.what()<<endl;
		usage();
	}
	catch (SeisppError& serr)
	{
		serr.log_error();
		usage();
	}
}
