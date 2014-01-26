#include <fstream>
#include "stock.h"
#include "dbpp.h"
#include "TimeSeries.h"
#include "Metadata.h"
#include "seispp.h"
#include "gclgrid.h"
#include "SEGY2002FileHandle.h"
#include "GenericFileHandle.h"
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
	cerr << "extract_volume db gridname fieldname "
		<< "[-o outfile -pf pffile -v]"<<endl
	      << "Default outfile is called extract_volume.su"<<endl;

	exit(-1);
}
ThreeComponentSeismogram ExtractAligned(GCLvectorfield3d& g, int i, int j)
{
    ThreeComponentSeismogram result(g.n3);
    int k,kk,l;
    for(k=0,kk=g.n3-1;k<g.n3;++k,--kk)
        for(l=0;l<3;++l) result.u(l,k)=g.val[i][j][kk][l];
    /* This is a huge potential confusion but necessary to extract these 
       here.  To make matters worse main can optionally rescale them. */
    double lat,lon;
    lat=g.lat(i,j,g.n3-1);
    lon=g.lon(i,j,g.n3-1);
    lat=deg(lat);   lon=deg(lon);
    result.put("ry",lat);
    result.put("rx",lon);
    result.put("sy",lat);
    result.put("sx",lon);
    return(result);
}
/* This version used when position is not on a grid point.   Much 
   more complex algorithm using linear interpolation of grid 
   coordinates.  xi and xj play a role similar to i and j for
   aligned version above. That is these are not coordinates but
   i and j with a fractional component.  

   this algorithm might be simpler if I called the bilinear interpolator
   directly, but since this program is seen as an end use application 
   applied only once per image volume I do not consider efficiency 
   that huge of an issue.  That is, there is an overhead here in using the
   lookup method that could be avoided.
   */
ThreeComponentSeismogram ExtractInterp(GCLvectorfield3d& g, double xi,
        double xj)
{
    ThreeComponentSeismogram result(g.n3);
    int i,j,k,kk,l;
    i=static_cast<int>(xi);
    j=static_cast<int>(xj);
    /* position this to start at top of grid */
    kk=g.n3-1;
    double dxi,dxj;
    /* Because dxi and dxj are in grid spacing units we can compute
       these with this mod arithmetic formula.  In addition, the 
       remainders serve as weights in the interpolation */
    dxi=xi-static_cast<double>(i);
    dxj=xj-static_cast<double>(j);
    /* This safety valve could be removed after debugging */
    if(i>=(g.n1-1)) 
    {
        cerr << "Coding error - xi points outside grid boundary"<<endl;
        exit(-1);
    }
    if(j>=(g.n2-1)) 
    {
        cerr << "Coding error - xj points outside grid boundary"<<endl;
        exit(-1);
    }
    double x1_0,x2_0,x3_0;

    
        double dx;
        /* First interpolate along i axis */
        dx=g.x1[i+1][j][kk]-g.x1[i][j][kk];
        x1_0=g.x1[i][j][kk] + dxi*dx;
        dx=g.x2[i+1][j][kk]-g.x2[i][j][kk];
        x2_0=g.x2[i][j][kk]+ dxi*dx;
        dx=g.x3[i+1][j][kk]-g.x3[i][j][kk];
        x3_0=g.x3[i][j][kk]+ dxi*dx;
        /* Elegantly simple formula for j axis */
        dx=g.x1[i][j+1][kk]-g.x1[i][j][kk];
        x1_0+= dxj*dx;
        dx=g.x2[i][j+1][kk]-g.x2[i][j][kk];
        x2_0+= dxj*dx;
        dx=g.x3[i][j+1][kk]-g.x3[i][j][kk];
        x3_0+= dxj*dx;
    
    /* Now we need to convert this to a geographic point.  We
       then define points desired as radially below the 
       surface point.   We could not use cartesian system
       to do this */
    Geographic_point gp0=g.ctog(x1_0,x2_0,x3_0);
    Geographic_point gp;
    for(k=0,kk=g.n3-1;k<g.n3;++k,--kk)
    {
        /* use the reference point to get depth */
        gp=g.geo_coordinates(i,j,kk);
        /* not sure why this is necessary, but was generating
           a compilation error in Macos with gcc */
        BasicGCLgrid *gptr=dynamic_cast<BasicGCLgrid*>(&g);
        double depth=gptr->depth(gp);
        gp=gp0;
        gp.r-=depth;
        double *vdata;
        Cartesian_point cp;
        try {
            cp=g.gtoc(gp);
            g.lookup(cp.x1,cp.x2,cp.x3);
            vdata=g.interpolate(cp.x1,cp.x2,cp.x3);
            for(l=0;l<3;++l) result.u(l,k)=vdata[l];
            delete [] vdata;
        }catch(std::exception& err)
        {
            cerr << "Interpolation error:  fatal for this program"<<endl
                << "Posted message follows"<<endl
                << err.what();
        }
    }
    /* Se need to set these here to avoid having to compute them 
       again */
    double lat,lon;
    lat=deg(gp0.lat);
    lon=deg(gp0.lon);
    result.put("ry",lat);
    result.put("rx",lon);
    result.put("sy",lat);
    result.put("sx",lon);
    return(result);
}

ThreeComponentSeismogram ExtractFromGrid(GCLvectorfield3d& g, 
        double x1i, double x2j)
{
    ThreeComponentSeismogram result;
    const double align_tolerance(0.01);
    bool grid_aligned(false);
    int i,j;
    i=static_cast<int>(x1i);
    j=static_cast<int>(x2j);
    double atest;
    atest=fabs(x1i-static_cast<double>(i));
    if(atest<align_tolerance)
    {
        atest=fabs(x2j-static_cast<double>(j));
        if(atest<align_tolerance) grid_aligned=true;
    }
    if(grid_aligned)
        result=ExtractAligned(g,i,j);
    else
        result=ExtractInterp(g,x1i,x2j);
    /* Required metadata */
    result.put("nsamp",g.n3);
    /* make dt units of meters */
    result.put("dt",g.dx3_nom*1000.0);
    result.ns=g.n3;
    /* This pair doesn't include 1000 factor because segy oddity of
       microsection dt */
    result.dt=g.dx3_nom;
    result.live=true;
    result.put("samprate",1.0/g.dx3_nom);
    result.tref=relative;
    result.t0=0.0;
    result.put("time",0.0);
    /* SU specific */
    /* use tracl to define lines - i fast becomes inline */
    //result.put("tracl",i);
   // result.put("tracr",1);
    result.put("fldr",1);
    /* This makes the data look like cdp stacked data */
    result.put("cdpt",1);
    result.put("cdp",j*g.n1+i);
    /*segy code for live seismic data. SU allows variable codes
    set later */
    result.put("trid",1);
    result.put("duse",1);
    return(result);
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
	int i,j;
	ios::sync_with_stdio();
	if(argc<4) usage();
	string dbname(argv[1]);
	string gridname(argv[2]);
	string fieldname(argv[3]);
        string pffile("extract_volume");
	ofstream outstrm;
	bool out_to_other(false);
	string outfile("extract_volume.su");
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
            double dx1=control.get_double("nondimensional_x1_sample_interval");
            double dx2=control.get_double("nondimensional_x2_sample_interval");
            /* This is a safety valve appropriate here, but not essential*/
            if( (dx1>1.0) || (dx2>1.0) )
            {
                cerr << "Fatal error:  can only resample"<<endl
                    << "nondimensional dx1 and dx2 values = "
                    << dx1 << " and " <<dx2<<endl
                    << "These must be less than 1 to define resampling "
                    << "in unit cell sizes"<<endl;
                exit(-1);
            }
            /* Additional scalar control parameters read next */
            // Lat-lon values are scaled by this factor before 
            // being written to output file headers
            double llscf=control.get_double("lat_lon_output_scale_factor");
            //Depth scale factor 
            double dpscf=control.get_double("depth_scale_factor");
            bool comp_to_save[3];
            comp_to_save[0]=control.get_bool("save_transverse");
            comp_to_save[1]=control.get_bool("save_radial");
            comp_to_save[2]=control.get_bool("save_vertical");
            if(!(comp_to_save[0] || comp_to_save[1] || comp_to_save[2]))
            {
                cerr << "Fatal error:  Must set one of save parameters true"
                    <<endl;
                exit(-1);
            }
            string outform=control.get_string("output_format");
            GenericFileHandle *outhandle;
            DatascopeHandle dbh(dbname,true);
            dbh.lookup(string("gclgdisk"));
	    DatascopeHandle dbhg(dbh);
	    dbhg.lookup(string("gclfield"));
	    Dbptr db=dbh.db;
	    Dbptr dbgrd=dbhg.db;
	    GCLvectorfield3d g(dbh,gridname,fieldname,5);
            string xrefstr=pftbl2string(pf,"metadata_cross_reference");
            AttributeCrossReference outxref(xrefstr);
            /* We share this list for both su and segy formats. This 
               will be a problem because segy2002 has some attributes 
               we set here not defined in SU (namely inline and crossline) */
            list<string> tmdlist=pftbl2list(pf,"output_metadata_list");
            /* We create empty lists for these for use in SU formaat */
            list<string> orderkeys,endslist;
            orderkeys.clear();   endslist.clear();
            if(outform=="SeismicUnix")
            {
                outhandle=new GenericFileHandle(outfile,string("SEGYfloat"),
                    outxref,orderkeys,endslist,tmdlist,false,
                    string("nsamp"),string("dt"),1000.0,true);
            }
            else if(outform=="SEGY")
            {
                SEGY2002FileHandle *sgyh;
                pfput_int(pf,"number_samples",g.n3);
                pfput_int(pf,"sample_interval",g.dx3_nom*1000);
                sgyh=new SEGY2002FileHandle(outfile,tmdlist,pf);
                outhandle=dynamic_cast<GenericFileHandle *>(sgyh);
            }
            else
            {
                cerr << "Illegal value specified for output_format parameter = "
                    << outform<<endl
                    << "Must be either SeismicUnix of SEGY"<<endl;
                exit(-1);
            }
            /* Now loop over grid.  */
            int i,j,tracr,npts;
            double x1i,x2j;
            double x1max,x2max;
            x1max=static_cast<double>(g.n1)-1;
            x2max=static_cast<double>(g.n2)-1;
            ThreeComponentSeismogram d;
            for(j=0,x2j=0.0,tracr;x2j<x2max;x2j+=dx2,++j)
                for(i=0,x1i=0.0;x1i<x1max;x1i+=dx1,++i)
                {
                    //DEBUG
                    //cout << i<<" "<<x1i<<" "<<j<<" "<<x2j<<endl;
                    d=ExtractFromGrid(g,x1i,x2j);
                    ++npts;
                    /* Some hard coded header edits better done
                       here than in ExtractFromGrid */
//                    double dv=d.get_double("rx");
//                    dv *= llscf;
//                    d.put("rx",dv);
//                    dv=d.get_double("ry");
//                    dv *= llscf;
//                    d.put("ry",dv);
//                    dv=d.get_double("sx");
//                    dv *= llscf;
//                    d.put("sx",dv);
//                    dv=d.get_double("sy");
//                    dv *= llscf;
//                    d.put("sy",dv);
                    d.put("rx",(double)i);
                    d.put("ry",(double)j);
                    d.put("sx",(double)i);
                    d.put("sy",(double)j);
                    /* SEGY uses this scale factor explicitly */
                    d.put("scalco",llscf);
                    /* Note these do not mesh with SU but can be 
                       written in modern SEGY in slots that were
                       formerly undefined extension slots. */
                    d.put("inline",i);
                    d.put("crossline",j);
                    /* This needs to be made optional for SU.  
                       For testing these are thought to work with Kingdom*/
                    d.put("ep",i);
                    d.put("tracf",j);
                    d.put("cdp",npts);
                    /* SU, I think, uses this like crossline number */
                    d.put("tracl",j);
                    //DEBUG
                    //cout << "dt="<<d.dt<<" metadata dt="<<d.get_double("dt")<<endl;
                    /* We have to extract and write each component
                       as scalar becaue segy/su are not explicitly 
                       vector formats.  write will not allow this. */
                    for(int k=0;k<3;++k)
                    {
                      if(comp_to_save[k])
                      {
                        TimeSeries *comp=ExtractComponent(d,k);
                        ++tracr;
                        comp->put("tracr",tracr);
                        /* These are special codes for SU and segy.
                         16 is vertical, 17 transverse, and 15 radial*/
                        switch (k)
                        {
                            case (0):
                                comp->put("trid",16);
                                break;
                            case (1):
                                comp->put("trid",17);
                                break;
                            case (2):
                            default:
                                comp->put("trid",15);
                        }
                        //DEBUG
                        //cout << *(dynamic_cast<Metadata*>(comp))<<endl;
                        outhandle->put(*comp);
                        delete comp;
                      }
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
