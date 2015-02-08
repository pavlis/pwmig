#include <string>
#include <list>
#include "gclgrid.h"
#include "seispp.h"
#include "dbpp.h"
using namespace std;
using namespace SEISPP;
void usage()
{
    cerr << "importpwmig file1 file2 ... filen [-db dbname]"<<endl
        <<  "  only pwmig output fields can be imported"<<endl
        <<  "  dbname default is imports"<<endl;
    exit(-1);
}
bool SEISPP::SEISPP_verbose(true);
int main(int argc,char **argv)
{
    if(argc<2) usage();
    string dbname("imports");
    string outdir("pwmigdata");
    string nulldir("");
    list<string> fieldlist;
    /* First parse the arg list to build the list of field names to import*/
    int i;
    for(i=1;i<argc;++i)
    {
        string sarg(argv[i]);
        if(sarg=="-db")
        {
            ++i;
            if(i>=argc) usage();
            dbname=string(argv[i]);
        }
        else
            fieldlist.push_back(sarg);
    }
    try {
        DatascopeHandle dbh(dbname,false);
        list<string>::iterator fptr;
        for(fptr=fieldlist.begin(),i=0;fptr!=fieldlist.end();++fptr,++i)
        {
            try {
                GCLvectorfield3d f(*fptr);
                if(i==0)
                    f.save(dbh,outdir,outdir,*fptr,*fptr);
                else
                    f.save(dbh,nulldir,outdir,*fptr,*fptr);
            }catch(GCLgridError& gerr)
            {
                cerr << "Problems with input file "<<*fptr<<endl
                    << "Error message:  "<<gerr.what()<<endl
                    << "Data from that file not imported"<<endl;
            }
        }
    }catch(SeisppError& serr)
    {
        serr.log_error();
    }
    catch(GCLgridError& gerr)
    {
        cerr << "Fatal Error:  "<<gerr.what()<<endl;
    }
}
