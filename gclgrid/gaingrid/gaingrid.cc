/* applies power law gain function */
#include <sstream>
#include "seispp.h"
#include "dbpp.h"
#include "gclgrid.h"
using namespace std;
using namespace SEISPP;
void usage()
{
    cerr << "gaingrid db grid field outfield scale power [-clip level]"<<endl
        << "   scales data by scale*depth^power"<<endl
        << "   -clip will set an amplitudes larger than level to level"
        <<endl;;
    exit(-1);
}
bool SEISPP::SEISPP_verbose(true);
int main(int argc, char **argv)
{
    /* This may need to be a parameter but for use with pwmig
       and current scaling this should be workable to test for a zero value*/
    if(argc<7) usage();
    string dbname(argv[1]);
    string gridname(argv[2]);
    string fieldname(argv[3]);
    string outfield(argv[4]);
    /* frozen for now */
    const string fielddir("gainedgrids");
    double scale,power;
    scale=atof(argv[5]);
    power=atof(argv[6]);
    bool clip_on(false);
    double cliplevel(9.9E99);;
    int i,j,k,l;
    for(i=7;i<argc;++i)
    {
        string sarg(argv[i]);
        if(sarg=="-clip")
        {
            ++i;
            if(i>=argc) usage();
            clip_on=true;
            cliplevel=atof(argv[i]);
        }
        else
            usage();
    }
    try {
        DatascopeHandle dbh(dbname,false);
        GCLvectorfield3d f(dbh,gridname,fieldname);
        double z,gain;
        int clipcount(0);
        for(k=0;k<f.n3-1;++k)
        {
            for(i=0;i<f.n1;++i)
            {
                for(j=0;j<f.n2;++j)
                {
                    z=f.depth(i,j,k);
                    gain=scale*pow(z,power);
                    /* This stuff is all needed to enable clipping */
                    double amp;
                    for(l=0,amp=0.0;l<3;++l) 
                        amp+=f.val[i][j][k][l]*f.val[i][j][k][l];
                    amp=sqrt(amp);
                    amp *= gain;
                    if(clip_on && (amp>cliplevel)) 
                    {
                        gain *= cliplevel/amp;
                        ++clipcount;
                    }
                    for(l=0;l<3;++l) 
                        f.val[i][j][k][l]*=gain;
                }
            }
        }
        stringstream ss;
        ss << outfield<<"_pow_"<<power;
        string dfile=ss.str();
        f.save(dbh,string(""),fielddir,outfield,dfile);
        cout << "Output saved in db with field name="<<outfield<<endl;
        if(clipcount>0)
        {
            cout << clipcount << " samples were clipped in output at level="
                <<cliplevel<<endl;
        }
    }
    catch(SeisppError& serr)
    {
        serr.log_error();
    }
    catch(std::exception& err)
    {
        cerr << err.what();
    }
}





