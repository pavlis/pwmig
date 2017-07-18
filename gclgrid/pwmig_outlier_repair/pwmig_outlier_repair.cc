#include <vector>
#include "VectorStatistics.h"
#include "gclgrid.h"
using namespace std;
using namespace SEISPP;
void usage()
{
    cerr << "pwmig_outlier_repair infile outfile [-iqc iqmult -sc x]"<<endl
        << "Will zero all data + or - iqmult*interquartiles (default 10.0)" <<endl
        << "-sc option scales data so interquartile  = x (default 1.0)"<<endl
        <<endl;
    exit(-1);
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
    if(argc<3) usage();
    string infile(argv[1]);
    string outfile(argv[2]);
    double iqmult(10.0);
    double sc(1.0);
    int iarg;
    for(iarg=3;iarg<argc;++iarg)
    {
        string sarg(argv[iarg]);
        if(sarg=="-iqc")
        {
            ++iarg;
            if(iarg>=argc) usage();
            iqmult=atof(argv[iarg]);
        }
        else if(sarg=="-sc")
        {
            ++iarg;
            if(iarg>=argc) usage();
            sc=atof(argv[iarg]);
        }
        else
            usage();
    }
    try {
        GCLvectorfield3d f(infile);
        vector <double> d;
        /* This is frozen for pwmig which has nv=5.  This is a
           sanity check */
        if(f.nv!=5) 
        {
            cerr << "pwmig_outlier_repair:   illegal nv="
                << " in grid to process found in file"<<infile<<endl
                << " This code is for pwmig grids only where nv is 5"
                <<" and components 0 and 1 are data to be repaired"
                <<endl;
            usage();
        }
        /* load all the samples from components 0 and 1 */
        d.reserve(f.n1*f.n2*f.n3*2);
        int i,j,k,l;
        for(i=0;i<f.n1;++i)
            for(j=0;j<f.n2;++j)
                for(k=0;k<f.n3;++k)
                    for(l=0;l<2;++l)
                        d.push_back(f.val[i][j][k][l]);

        VectorStatistics<double> stats(d);
        double trimline=stats.interquartile();
        cout << "File "<<infile<<" interquartile distance is "<<trimline<<endl;
        double scale=sc/trimline;
        trimline *= iqmult;
        cout << "Trimming data outside a range of "<<trimline<<endl;
        int zerocount(0);
        for(i=0;i<f.n1;++i)
            for(j=0;j<f.n2;++j)
                for(k=0;k<f.n3;++k)
                    for(l=0;l<2;++l)
                    {
                        if(fabs(f.val[i][j][k][l])>trimline)
                        {
                            f.val[i][j][k][l]=0.0;
                            ++zerocount;
                        }
                        else
                            f.val[i][j][k][l]*=scale;
                    }
        cout << "Zeroed "<<zerocount<<" samples found outside this range"
            <<endl;
        cout << "Ok to continue with save?"<<endl
            << "Enter y to save.  Anything else and I will exit without saving"
            <<endl;
        string ques;
        cin >> ques;
        if(ques!="y")exit(-2);
        f.save(outfile,"editfields");
    }catch(std::exception& e)
    {
        cerr << e.what()<<endl;
    }
    catch(SeisppError& serr)
    {
        serr.log_error();
    }
}

