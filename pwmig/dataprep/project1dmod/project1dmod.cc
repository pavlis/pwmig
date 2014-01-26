#include <vector>
#include "seispp.h"
#include "dbpp.h"
#include "gclgrid.h"
#include "VelocityModel_1d.h"
using namespace std;
using namespace SEISPP;

void usage()
{
	cerr << "project1dmod db gridname vmodname "
	<< "[-field fieldname -mt modtype -p rayparameter -V]"<<endl
	<< "Default fieldname=vmodname, modtype=P, rayparameter=0(s/km)"<<endl;
	exit(-1);
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
	if(argc<4) usage();
	string dbname(argv[1]);
	string gridname(argv[2]);
	string vmodname(argv[3]);
	string modtype("P");
	string fieldname=vmodname;
	double rayp(0.0);
	int i;
	for(i=4;i<argc;++i)
	{
		string argstr(argv[i]);
		if(argstr=="-field")
		{
			++i;
			fieldname=string(argv[i]);
		}
		else if(argstr=="-mt")
		{
			++i;
			modtype=string(argv[i]);
		}
		else if(argstr=="-p")
		{
			++i;
			rayp=atof(argv[i]);
		}
		else if(argstr=="-V")
		{
			SEISPP_verbose=true;
		}
		else
		{
			cerr << "Illegal argument="<<argstr<<endl;
			usage();
		}
	}
	try {
		cout << "Building database handles"<<endl;
		DatascopeHandle dbh(dbname,false);
		DatascopeHandle dbhmod(dbh);
		dbhmod.lookup("mod1d");
		DatascopeHandle dbhgrid(dbh);
		dbhgrid.lookup("gclgdisk");
		cout << "Loading gridname="<<gridname<<endl;
		GCLgrid3d grid(dbhgrid,gridname);
		GCLscalarfield3d mod(grid);
		cout << "Loading 1d velocity model with name="<<vmodname<<endl;
		VelocityModel_1d Vmod(dbhmod.db,vmodname,modtype);
		/* the following works only when the grid has constant
		depth slices for index 3 constant */
		vector<double> zi,vi;
		cout << "1D velocities (z,v,vapparent)"<<endl;
		for(i=mod.n3-1;i>=0;--i)
		{
			double z;
			z=mod.depth(0,0,i);
			zi.push_back(z);
			double vel,slow;
			vel=Vmod.getv(z);
			cout << vel <<"  ";
			if(rayp>0.0)
			{
				slow=1/vel;
				if(slow<rayp)
				{
					cerr << "Ray parameter mismatch"<<endl
					<< "slowness="<<slow<<" at z="<<z
					<< " but rayp="<<rayp<<endl
					<< "ray parameter must be less than slowness"<<endl;
					exit(-1);
				}
				slow=sqrt(slow*slow-rayp*rayp);
				vel=1.0/slow;
			}
			cout <<vel<<endl;
			vi.push_back(vel);
		}
		initialize_1Dscalar(mod,vi,zi);
		if(SEISPP_verbose)
		{
			cout << "Model written to db"<<endl;
			cout << mod;
		}
		/* for now this is a frozen directory name */
		string fielddir("vmodels");
		string gclgdir("");  /* null as save is not needed*/
		/* Use the fieldname as the file name.  Restrictive,
		but this code may never leave the neighborhood. */
		mod.save(dbhgrid,gclgdir,fielddir,fieldname,fieldname);
	} catch (SeisppError serr)
	{
		serr.log_error();
	}
	catch (int ierr)
	{
		cerr << "Some GCLgrid routine threw error number="<<ierr<<endl;
	}

	catch (GCLgrid_error gclerr)
	{
		gclerr.log_error();
	}
}
