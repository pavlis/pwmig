#include <list>
#include "seispp.h"
#include "gclgrid.h"
using namespace SEISPP;
void usage()
{
  cerr << "gclfield_resample pattern infield outfield [-vectori -null val -dir dir]"<<endl
    << "Resample GCLfield object data in infield onto discrete points in pattern"<<endl
    << "pattern - gclgrid3d defining output geometry"<<endl
    << "(Best constructed with makegclgrid)"<<endl
    << "infield - field object base name to be resampled"<<endl
    << "outfile - output filed base name"<<endl
    << "(Note all 3 above are base names used for file storage of gclgrid objects)"<<endl
    << " -vector - If present assume the input field is a vector field"<<endl
    << "  (default is scalar data)"<<endl
    << " -null - specify alternative to null value used when interpolation fails"<<endl
    << "Default is 9999999.9"<<endl
    << " -dir - save data to directory dir (default is current directory)"<<endl;
}
const double nullvalue(9999999.9);
void setnulls(GCLscalarfield3d *f)
{
  int i,j,k;
  for(i=0;i<f->n1;++i)
    for(j=0;j<f->n2;++j)
      for(k=0;k<f->n3;++k)
        f->val[i][j][k]=nullvalue;
}
void setnulls(GCLvectorfield3d *f)
{
  int i,j,k,l;
  for(i=0;i<f->n1;++i)
    for(j=0;j<f->n2;++j)
      for(k=0;k<f->n3;++k)
        for(l=0;l<f->nv;++l)
          f->val[i][j][k][l]=nullvalue;
}
/* These 2 procedures are needed because of a flaw in the design
   of GLCgrid from my lack of experience in object design when 
   this library was originally build. */
void setpoint(GCLscalarfield3d *fin,Cartesian_point cp,
    GCLscalarfield3d *fout,int i, int j, int k)
{
  /* Interpolate never throws an error */
  double val;
  val=fin->interpolate(cp.x1,cp.x2,cp.x3);
  fout->val[i][j][k]=val;
}
void setpoint(GCLvectorfield3d *fin,Cartesian_point cp,
    GCLvectorfield3d *fout,int i, int j, int k)
{
  /* Interpolate never throws an error */
  double *val;
  val=fin->interpolate(cp.x1,cp.x2,cp.x3);
  int l;
  for(l=0;l<fout->nv;++l)
    fout->val[i][j][k][l]=val[l];
  delete [] val;
}
/* This does the main work of this program in a generic way to
   support both scalar and vector data.   returns the number of 
   points set in output */
template <typename T> int resamplegrid(GCLgrid3d& g, T *f,T *fout)
{
  try{
    int i,j,k;
    Geographic_point gp;
    Cartesian_point cp;
    int nset(0);
    for(i=0;i<g.n1;++i)
    {
      for(j=0;j<g.n2;++j)
      {
        for(k=0;k<g.n3;++k)
        {
          gp=g.geo_coordinates(i,j,k);
          /* Critical to look up cartesian coordinates in f no g */
          cp=f->gtoc(gp);
          int iret;
          /* returning 0 is success.   Only try to set the point if 
             lookup succeeds*/
          iret=f->lookup(cp.x1,cp.x2,cp.x3);
          if(iret==0)
          {
            setpoint(f,cp,fout,i,j,k);
            ++nset;
          }
        }
      }
    }
    return nset;
  }catch(...){throw;};
}
/* this routine does all the work of this program.   pattern is the
   grid geometry to be clones.   fin and fout are input and output
   fields. 

   Making this a template turned out to be baggage.   Retained
   during development.  Uncovered a design flaw in the GCLgrid library
   that makes template programming difficult.
*/
template <typename Tfield> int resamp_and_save(GCLgrid3d& pattern,
    Tfield *fin, Tfield *fout,string outfile,string outdir)
{
  int nset;
  setnulls(fout);
  nset=resamplegrid<Tfield>(pattern,fin,fout);
  fout->save(outfile,outdir);
  return nset;
}
bool SEISPP::SEISPP_verbose(true);
int main(int argc, char **argv)
{
  int i;
  if(argc<4) usage();
  string pattern(argv[1]);
  cout << "gclfield_resample:"<<endl
   << "  using gclgrid3d data stored with base name="<<pattern
   << " to define resampled geometry"<<endl;
  string infield(argv[2]);
  cout << "Will read field data with base name="<<infield<<endl;
  string outfield(argv[3]);
  string outdir(".");
  cout << "Will write resampled field data to output with base name="<<outfield
    <<endl;
  bool vector_data(false);
  for(i=4;i<argc;++i)
  {
    string sarg(argv[i]);
    if(sarg=="-vector")
      vector_data=true;
    else if(sarg=="-dir")
    {
      ++i;
      if(i>=argc) usage();
      outdir=string(argv[i]);
    }
    else
    {
      cerr << "Illegal command line argument="<<sarg<<endl;
      usage();
    }
  }
  try{
    GCLgrid3d g(pattern);
    int nset,number_grid_points;
    GCLscalarfield3d *fin,*fout;
    GCLvectorfield3d *fvin,*fvout;

    if(vector_data)
    {
      fvin=new GCLvectorfield3d(infield);
      fvout=new GCLvectorfield3d(g,fvin->nv);
      nset=resamp_and_save<GCLvectorfield3d>(g,fvin,fvout,outfield,outdir);
      delete fvin;
      delete fvout;
    }
    else
    {
      fin=new GCLscalarfield3d(infield);
      fout=new GCLscalarfield3d(g);
      nset=resamp_and_save<GCLscalarfield3d>(g,fin,fout,outfield,outdir);
      delete fin;
      delete fout;
    }
    number_grid_points=(g.n1)*(g.n2)*(g.n3);
    cout << "gclfield_resample:  set data in "<<nset
      <<" grid points in grid with total number of grid points="
      <<number_grid_points<<endl;
    cout << "Unset values were set to null value="<<nullvalue<<endl;
  }catch(std::exception& exc)
  {
    cerr << exc.what()<<endl;
  }
}

