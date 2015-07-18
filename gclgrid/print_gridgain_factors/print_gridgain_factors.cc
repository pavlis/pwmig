/* This small program takes an input grid (in file format not db) 
   and prints out suggested gain factors for a fixed range of 
   geometric spread factors.  Uses median absolute difference of vector
   amplitudes as a function of depth.   Assumes a standard pwmig grid
   geometry with fixed depth intervals on all grid points.   Discards 
   NULL points.  
   */
#include <vector>
#include <fstream>
#include "gclgrid.h"
#include "seispp.h"
using namespace SEISPP;
void usage()
{
    cerr << "print_gridgain_factors gclgridfile"<<endl;
    exit(-1);
}
bool SEISPP::SEISPP_verbose(true);
int main(int argc, char **argv)
{
    if(argc!=2) usage();
    vector<double> pfac;  //power law scaling factors
    for(double p=0.5;p<2.1;p+=0.1)
        pfac.push_back(p);
    try{
        string fname(argv[1]);
        GCLvectorfield3d d(fname);
        if(d.nv!=5)
        {
            cerr << "File "<<fname<<" does not appear to be a pwmig output file"
                <<endl;
            exit(-1);
        }
        int i,j,k,l;
        vector<double> avz,aij,z;
        aij.reserve(d.n1*d.n2);
        avz.reserve(d.n3);
        z.reserve(d.n3);
        int nfac=pfac.size();
        for(k=d.n3-1;k>=0;--k)
        {
            for(i=0;i<d.n1;++i)
                for(j=0;j<d.n2;++j)
                {
                    if(d.val[i][j][k][4]>0.0)
                    {
                        double amp;
                        for(amp=0.0,l=0;l<3;++l)
                            amp+=d.val[i][j][k][l]*d.val[i][j][k][l];
                        amp=sqrt(amp);
                        aij.push_back(amp);
                    }
                }
            if(aij.size()>0)
            {
                z.push_back(d.depth(0,0,k));
                avz.push_back(median<double>(aij));
            }
            else
            {
                cerr << "print_gaingrid_factor:  entire image volume "
                    << "at depth="<<d.depth(0,0,k)
                    <<" was null"<<endl
                    << "Output will not be regular"
                    <<endl;
            }
            aij.clear();
        }
        /* Load the dmatrix with power scaled amplitudes */
        int nz=avz.size();
        dmatrix curves(nz,nfac);
        for(j=0;j<nfac;++j)
        {
            /* Needed to avoid roundoff error at 0 which could
                yield imaginary factor  and a NaN from pow */
            if(z[i]<0.0) z[i]=fabs(z[i]);
            for(i=0;i<nz;++i)
            {
                /* This test is needed to avoid NaN when pow
                   is called with a negative x */
                if(z[i]<=0.0)
                    curves(i,j)=0.0;
                else
                    curves(i,j)=avz[i]*pow(z[i],pfac[j]);
            }
        }
        ofstream dcf;
        dcf.open("decaycurves.dat");
        cout << "Writing amplitude decay curves to file decaycurves.dat"<<endl
            << "First column of that file contains exponent for decay correction"<<endl;
        for(i=0;i<nfac;++i)
            dcf << pfac[i]<<" ";
        dcf <<endl;
        dcf << curves<<endl;
        dcf.close();
        cout << "Recommended scale factors for each decay curve"<<endl;
        /* Recycle avz and use for median calculations */
        avz.clear();
        for(i=0;i<nfac;++i)
        {
            for(k=0;k<nz;++k) avz.push_back(curves(k,i));
            cout << "For exponent="<< pfac[i]<< " use scale="<< median<double>(avz)<<endl;
            avz.clear();
        }
    }catch(std::exception& ex)
    {
        cerr << ex.what()<<endl;
    }
}


