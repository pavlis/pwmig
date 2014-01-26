#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "seispp.h"
#include "dbpp.h"
#include "gclgrid.h"
#include "dmatrix.h"
#include "VectorStatistics.h"
using namespace std;
using namespace SEISPP;
/* This routine extracts a depth window from the grid using interpolation
   at interval dz over a range windowlen*dz.  This is not a very 
   efficient way to do this since this will be called many times
   and most of the calculations done here will be the same for 
   all points when using a constant depth slice.  However, I 
   did this because I plan to recycle this code for dipping 
   interfaces later and then this is the necessary way to do this. */
dmatrix ExtractDataWindow(GCLvectorfield3d& g, double lon, double lat, 
        double depth, int windowlen,double dz)
{
    try {
        dmatrix result(3,windowlen);
        result.zero();
        Cartesian_point cp;
        /* compute depth range */
        double zmin,zmax,zhalf; 
        zhalf=static_cast<double>(windowlen-1)*dz/2.0;
        zmin=depth-zhalf;
        /* Silently shift if window hits ceiling */
        if(zmin<0.0) zmin=0.0;
        zmax=zmin+2.0*zhalf;
        /* Assume g is a regular grid with base at constant radius */
        double zfloor=g.depth(0,0,0);
        if(zmax>zfloor)
        {
            if( (zfloor-(2.0*zhalf))<0.0)
            {
                cerr << "Illegal windowlen for dz of this grid"<<endl
                    <<"Window length "<<2.0*zhalf<<" exceed depth range of grid"
                    <<endl;
                exit(-1);
            }
            else
            {
                zmax=zfloor;
                zmin=zfloor-2.0*zhalf;
            }
        }
        int index[3];
        /* Now fill the result matrix filling the matrix from the bottom
           up.  Be warned if you try to plot this it will be a bit 
           upside down compared to standard convention.  Done for speed
           because of a peculiarity of the lookup method. */
        double z;
        Geographic_point gp;
        gp.lon=lon;
        gp.lat=lat;
        gp.r=r0_ellipse(lat)-zmin;
        for(int i=0;i<windowlen;++i)
        {
            cp=g.gtoc(gp);
            int ierr=g.lookup(cp.x1,cp.x2,cp.x3);
            if(ierr!=0) 
            {
                /* Note this returns the count where lookup failed */
                throw i;
            }
            double *vec;
            vec=g.interpolate(cp.x1,cp.x2,cp.x3);
            /*gridstacker sets component 4 to stack count.  If zero 
              it means there is no data for that cell.  Since this 
              is a common situation it cannot be treated as an 
              exception.  Thus we instead return a 1x1 matrix to 
              signal this condition. */
            if(vec[4]<=0.1) 
            {
                delete [] vec;
                return (dmatrix(1,1));
            }
            for(int k=0;k<3;++k) result(k,i)=vec[k];
            delete [] vec;
            gp.r -= dz;
        }
        return(result);
    } catch(...){throw;};
}
double rms(vector<double> d)
{
    vector<double>::iterator dptr;
    double result;
    for(result=0.0,dptr=d.begin();dptr!=d.end();++dptr)
        result+=(*dptr)*(*dptr);
    result=sqrt(result)/static_cast<double>(d.size());
    return(result);
}
double mad(vector<double> d)
{
    vector<double>::iterator dptr;
    vector<double> amp;
    amp.reserve(d.size());
    for(dptr=d.begin();dptr!=d.end();++dptr)
        amp.push_back(fabs(*dptr));
    double result=median<double>(amp);
    return(result);
}
/* These do computations.  All intentionally do not 
   test comp for validity or include an error handler.
   Done purely for efficiency.
   Change if these are transported outside this code.  
   They are also painfully inefficient and repetitious.
   For efficiency it would be preferable to have 
   one routine to extract a component and another to
   compute 3c amplitudes then return all the stats in
   a single data structure.  For this application this
   was considered unnecessary as performance is not
   an issue for current plans.*/
double compute_rms(dmatrix& d,int comp)
{
    vector<double> dv;
    int n=d.columns();
    for(int i=0;i<n;++i) dv.push_back(d(comp,i));
    return(rms(dv));
}
double compute_mad(dmatrix& d,int comp)
{
    vector<double> dv;
    int n=d.columns();
    for(int i=0;i<n;++i) dv.push_back(d(comp,i));
    return(mad(dv));
}
/* Return pair with first = lag and second is amplitude at peak.
Note late calculation assumes 0 is center of range passed */
pair<int,double> compute_max(dmatrix& d,int comp)
{
    vector<double> dv;
    vector<double>::iterator maxvalptr;
    int n=d.columns();
    for(int i=0;i<n;++i) dv.push_back(d(comp,i));
    maxvalptr=max_element(dv.begin(),dv.end());
    int i0=distance(dv.begin(),maxvalptr);
    /* convert to lag relative to center from vector count.
    Note counted from top */
    i0-=(n/2);
    pair<int,double> result(i0,*maxvalptr);
    return(result);
}
double compute_rms3c(dmatrix& d)
{
    vector<double> dv;
    int n=d.columns();
    for(int i=0;i<n;++i) 
    {
        double amp;
        amp=0.0;
        for(int k=0;k<3;++k) 
            amp+=d(k,i)*d(k,i);
        amp=sqrt(amp);
        dv.push_back(amp);
    }
    return(rms(dv));
}
double compute_mad3c(dmatrix& d)
{
    vector<double> dv;
    int n=d.columns();
    for(int i=0;i<n;++i) 
    {
        double amp;
        amp=0.0;
        for(int k=0;k<3;++k) 
            amp+=d(k,i)*d(k,i);
        amp=sqrt(amp);
        dv.push_back(amp);
    }
    return(mad(dv));
}
/* Return lag and amplitude using hypot for 3d vector.
   Note lag is relative to centered range.  (-n/2 to n/2)*/
pair<int,double> compute_max3c(dmatrix& d)
{
    vector<double> dv;
    vector<double>::iterator maxvalptr;
    int n=d.columns();
    for(int i=0;i<n;++i) 
    {
        double amp;
        amp=0.0;
        for(int k=0;k<3;++k) 
            amp+=d(k,i)*d(k,i);
        amp=sqrt(amp);
        dv.push_back(amp);
        //DEBUG
        //cerr << i << " "<<amp<<endl;
    }
    maxvalptr=max_element(dv.begin(),dv.end());
    int i0=distance(dv.begin(),maxvalptr);
    //DEBUG
    //cerr << "i0="<<i0;
    /* convert to lag relative to center from vector count. 
    Note order top to bottom*/
    i0-=(n/2);
    //cerr << " i0 relative to center="<<i0<<endl;
    pair<int,double> result(i0,*maxvalptr);
    return(result);
}
void usage()
{
    cerr << "horizonstats db gridname fieldname windowsize(km) [-raw -writenull] < inlist"<<endl
        << "gridname:fieldname expected in db.gclfield table"<<endl
        << "Compute with window length windowlen in depth sampled at grid dz/2"
        <<endl
        << "-raw output raw amplitude (default removes median)" 
        <<endl
        << "-writenull puts -1 in all fields when interp fails"<<endl;
    exit(-1);
}
bool SEISPP::SEISPP_verbose(true);
int main(int argc, char **argv)
{
    if(argc<5) usage();
    ios::sync_with_stdio();
    string dbname(argv[1]);
    string gridname(argv[2]);
    string fieldname(argv[3]);
    int windowsize=atof(argv[4]);
    if(windowsize<0)
    {
        cerr << "Illegal window size = "<<windowsize<<endl;
        usage();
    }
    bool rawoutput(false);
    bool writenull(false);
    for(int iarg=5;iarg<argc;++iarg)
    {
        string sarg(argv[iarg]);
        if(sarg=="-raw")
            rawoutput=true;
        else if(sarg=="-writenull")
            writenull=true;
        else
            usage();
    }
    if(writenull)
    {
        if(!rawoutput)
        {
            cerr << "Warning setting raw output (no median scaling) because -writenull enabled"<<endl;
            rawoutput=true;
        }
    }
    try{
        DatascopeHandle dbh(dbname,true);
        /* pwmig output is a 5component vector field.  Read it from
           the database */
        GCLvectorfield3d image(dbh,gridname,fieldname,5);
        const double oversampling(2.0);  // oversampling relative to dz
        double dz=image.dx3_nom/oversampling;
        int windowlen=SEISPP::nint(windowsize/dz)+1;
        double plon, plat, pdepth;
        char line[256];
        vector<double> lon,lat,depth;
        vector<double> rmsT,madT,maxT,rmsR,madR,maxR,rms3d,mad3d,max3d;
        vector<double> depthR,depthT,depth3d;
        double peakdepth;
        pair<int,double> maxreturn;
        while(cin.getline(line,256))
        {
            sscanf(line,"%lf%lf%lf",&plon,&plat,&pdepth);
            plon=rad(plon);
            plat=rad(plat);
            try {
                dmatrix d=ExtractDataWindow(image,plon,plat,pdepth,
                    windowlen,dz);
                /* A window with null data in the window has d set
                   as a 1x1 matrix.  Thus this is a test for null
                   data.  We only write results without valid data*/
                if(d.rows()>1)
                {
                    lon.push_back(deg(plon));
                    lat.push_back(deg(plat));
                    depth.push_back(pdepth);
                    rmsT.push_back(compute_rms(d,0));
                    madT.push_back(compute_mad(d,0));
                    maxreturn=compute_max(d,0);
                    maxT.push_back(maxreturn.second);
                    peakdepth=pdepth+static_cast<double>(maxreturn.first)*dz;
                    depthT.push_back(peakdepth);
                    rmsR.push_back(compute_rms(d,1));
                    madR.push_back(compute_mad(d,1));
                    maxreturn=compute_max(d,1);
                    maxR.push_back(maxreturn.second);
                    peakdepth=pdepth+static_cast<double>(maxreturn.first)*dz;
                    depthR.push_back(peakdepth);
                    rms3d.push_back(compute_rms3c(d));
                    mad3d.push_back(compute_mad3c(d));
                    maxreturn=compute_max3c(d);
                    max3d.push_back(maxreturn.second);
                    peakdepth=pdepth+static_cast<double>(maxreturn.first)*dz;
                    depth3d.push_back(peakdepth);
                }
                else if(rawoutput)
                {
                    lon.push_back(deg(plon));
                    lat.push_back(deg(plat));
                    depth.push_back(pdepth);
                    rmsT.push_back(-1.0);
                    madT.push_back(-1.0);
                    maxT.push_back(-1.0);
                    depthT.push_back(-1.0);
                    rmsR.push_back(-1.0);
                    madR.push_back(-1.0);
                    maxR.push_back(-1.0);
                    depthR.push_back(-1.0);
                    rms3d.push_back(-1.0);
                    mad3d.push_back(-1.0);
                    max3d.push_back(-1.0);
                    depth3d.push_back(-1.0);
                }
            } catch(int ierr)
            {
                cerr << "GCLgrid3d lookup method failed for point "
                    <<line<<endl
                    <<"Lookup failed to find point "<<ierr
                    <<" in window of length "<<windowlen<<endl
                    <<"Point skipped - trying next input point"<<endl;
            };
        }
        // Not best form with inverted logic here 
        // This removes median by default
        int npoints=lon.size();
        if(!rawoutput)
        {
            double medrmsR,medmadR,medmaxR,medrmsT,medmadT,medmaxT;
            double medrms3d,medmad3d,medmax3d;
            medrmsR=median(rmsR);
            cerr << "median rms radial "<<medrmsR<<endl;
            medmadR=median(madR);
            cerr << "median mad radial "<<medmadR<<endl;
            medmaxR=median(maxR);
            cerr << "median max radial "<<medmaxR<<endl;
            medrmsT=median(rmsT);
            cerr << "median rms transverse "<<medrmsT<<endl;
            medmadT=median(madT);
            cerr << "median mad transverse "<<medmadT<<endl;
            medmaxT=median(maxT);
            cerr << "median max transverse "<<medmaxT<<endl;
            medrms3d=median(rms3d);
            cerr << "median rms 3-component "<<medrms3d<<endl;
            medmad3d=median(mad3d);
            cerr << "median mad 3-component "<<medmad3d<<endl;
            medmax3d=median(max3d);
            cerr << "median max 3-component "<<medmax3d<<endl;
            for(int i=0;i<npoints;++i)
            {
                rmsR[i]=rmsR[i]/medrmsR;
                madR[i]=madR[i]/medmadR;
                maxR[i]=maxR[i]/medmaxR;
                rmsT[i]=rmsT[i]/medrmsT;
                madT[i]=madT[i]/medmadT;
                maxT[i]=maxT[i]/medmaxT;
                rms3d[i]=rms3d[i]/medrms3d;
                mad3d[i]=mad3d[i]/medmad3d;
                max3d[i]=max3d[i]/medmax3d;
            }
        }
        for(int i=0;i<npoints;++i)
        {
            cout << lon[i]<<" "
                <<lat[i]<<" "
                <<depth[i]<<" "
                <<rmsR[i]<<" "
                <<madR[i]<<" "
                <<maxR[i]<<" "
                <<rmsT[i]<<" "
                <<madT[i]<<" "
                <<maxT[i]<<" "
                <<rms3d[i]<<" "
                <<mad3d[i]<<" "
                <<max3d[i]<<" "
                <<depthR[i]<<" "
                <<depthT[i]<<" "
                <<depth3d[i]<<endl;
        }
    } catch (SeisppError& serr)
    {
        serr.log_error();
    }
    catch (GCLgridError& gclerr)
    {
        cerr << gclerr.what()<<endl;
    }
}
