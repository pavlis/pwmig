#include <iostream>
#include <fstream>
#include "VelocityModel_1d.h"
#include "gclgrid.h"
using namespace std;
using namespace SEISPP;
/* this is a special program to convert the Berkeley Dynamic North America model
   aka 2010 to a gcl grid that can be converted to a vtk file for inclusion in the TA scene.
   It has lots of frozen constants built in to the 
   program.  Done because this is not expected to ever be released.
   */
 
 /*This procedure loads the iasp91 matrix in the Sigloch et al supplement
   saved with matlab save -ascii   This is probably wrong for Berkeley model - CHECK BEFORE USING*/
/*   Retained for later use if needed.   
VelocityModel_1d load_iasp91()
{
    vector<double> din,vin;
    try {
        ifstream modins;
        modins.open("iasp91.txt",ios::in);
        if(modins.fail())
        {
            cerr << "Cannot open model file iasp91.txt"<<endl;
            exit(-1);
        }
        do {
            double d,v;
            modins >> d;
            modins >> v;
            if(!modins.eof())
            {
                din.push_back(d);
                vin.push_back(v);
            }
        } while(!modins.eof());
        int np=din.size();
        if(np<=1) 
        {
            cerr << "Read error in iasp91.txt.  points read="<<np<<endl;
            exit(-1);
        }
        VelocityModel_1d result(np);
        int i;
        // Top is special and is always saved
        result.v.push_back(vin[0]);
        result.z.push_back(din[0]);
        for(i=1;i<np;++i)
        {
            int iend=result.v.size() - 1;
            if(fabs(din[i]-din[i-1])<0.001)
            {
                result.v[iend]=vin[i];
            }
            else
            {
                result.v.push_back(vin[i]);
                result.z.push_back(din[i]);
                // Note grad will be implicitly placed at i-1
                result.grad.push_back((vin[i]-vin[i-1])/(din[i]-din[i-1]));
            }
        }
        result.grad.push_back(0.0);  //insert 0 at end
        result.nlayers=result.v.size();
        cout << "depth   VP    grad (vel/km)"<<endl;
        for(i=0;i<result.v.size();++i)
        {
            cout << result.z[i] << " "
                << result.v[i] << " "
                << result.grad[i] << endl;
        }
        return(result);
    }catch(...)
    {
        cerr << "Something threw and unknown exception"<<endl;
        exit(-1);
    }
}
*/

void usage()
{
    cout << "convertucb db"<<endl;
    exit(-1);
}
int main(int argc, char **argv)
{
    try {
    if(argc!=2) usage();
    string dbname(argv[1]);
    cout << "Will write results to Antelope db = "<< dbname<<endl;
    Dbptr db;
    if(dbopen(const_cast<char *>(dbname.c_str()),"r+",&db))
    {
        cerr << "dbopen failed on this database.  Cannot continue!"<<endl;
        usage();
    }
    /* This is procedure defined above that loads the iasp91 file from
       Sigloch et al.s matlab file.  Returns a model */
    //VelocityModel_1d vmod=load_iasp91();
    /* Will make origin first point in ascii file */
    double lat0(32.0);
    double lon0(-125.0);
    /* gclgrid requires these in radians */
    lat0=rad(lat0);
    lon0=rad(lon0);
    double r0=r0_ellipse(lat0);
    /* These are frozen constants for DNA10 */
    const int n1vp(141),n2vp(101),n3vp(41);
    const double dx1nom(25.0),dx2nom(25.0),dx3nom(25.0);
    const int i0(0),j0(0);

    /* Build a template for the grid that will define this model */
    GCLgrid3d *grid=new GCLgrid3d(n1vp,n2vp,n3vp,string("UCBtomo10"),
            lat0,lon0,r0,0.0,
            dx1nom,dx2nom,dx3nom,i0,j0);

    /* This clones the grid but does not set the field variables*/
    GCLscalarfield3d dVS(*grid);
    delete grid;
    int i,j,k,kk;
    double lat,lon,r;
    double dep,dvsin,dummy;
    Geographic_point gp;
    Cartesian_point cp;
    char linebuffer[256];
    /* Skip the header line */
    cin.getline(linebuffer,256);
    /* This model scans in order fairly conguenty with GCLgrid geometry but
       a different scan order thatn natural.  Order is longitude fastest, 
       latitude second, and depth last.  All are arranged right handed, at least,
       so there are no inverted indices.  Most unusual is depth moves from bottom up,
       which is more rational in my view.*/
    for(k=0;k<n3vp;++k)
        for(j=0;j<n2vp;++j)
            for(i=0;i<n1vp;++i)
    /*  REMOVE ME.  Retained for initial hacking.
    for(k=n3vp-1,kk=0;kk<n3vp;--k,++kk)
      for(i=0;i<n1vp;++i)
        for(j=n2vp-1;j>=0;--j)
        */
        {
            cin >> r;
            cin >> lat;
            cin >> lon;
            cin >> dvsin;
            cin >> dummy;  // needed because format has a column of zeros here 
            lat=rad(lat);
            lon=rad(lon);
            double rsurface=r0_ellipse(lat);
            /*This model uses a spherical approximation but everything
              we've done uses reference ellipsoid.  Have to convert or this 
              will look funny.*/
            dep=6370.0-r;
            r=rsurface-dep;
            cp=dVS.gtoc(lat,lon,r);
            dVS.x1[i][j][k]=cp.x1;
            dVS.x2[i][j][k]=cp.x2;
            dVS.x3[i][j][k]=cp.x3;
            dVS.val[i][j][k]=dvsin;
        }
        string griddir("grids"),modeldir("vmodels");
        /* save the dvs data. arg2 is null string to prevent save error for duplicate
        grid */
        dVS.dbsave(db,griddir,modeldir,string("dVS10"),string("dVSDNA10"));
        /* Now we convert all values to absolute velocities using the ak135
           velocity model sorted in the vmod object */
//        double v1d,depth;
//        for(j=0;j<n2vp;++j)
//        {
//            for(i=0;i<n1vp;++i)
//            {
//                for(k=0;k<n3vp;++k)
//                {
//                    depth=dVP.depth(i,j,k);
//                    v1d=vmod.getv(depth);
//                /* model is percent perturbation of velocity */
//                dVP.val[i][j][k]=v1d+(0.01*v1d*dVP.val[i][j][k]);
//                }
//            }
//        }
//        dVP.dbsave(db,string(""),modeldir,string("VPsig08"),string("VPsig08"));
    } catch (int ierr)
    {
        cerr << "dbsave threw error code "<< ierr<<endl
            << "Probably no output"<<endl;
    }
    catch (SeisppError& serr)
    {
        serr.log_error();
    }
}
