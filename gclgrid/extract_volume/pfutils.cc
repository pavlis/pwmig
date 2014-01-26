#include <string>
#include <list>
#include "stock.h"
#include "pf.h"
#include "seispp.h"
using namespace std;
using namespace SEISPP;
string pftbl2string(Pf *pf, const char* tag)
{
    Tbl *t;
    t=pfget_tbl(pf,const_cast<char *>(tag));
    if(t==NULL) throw SeisppError(string("pftbl2string:  ")
            + "pf file is missing Tbl with tag = "
            +tag);
    string result;
    for(int i=0;i<maxtbl(t);++i)
    {
        char *line=(char *)gettbl(t,i);
        result += line;
        result += "\n";
    }
    return result;
}
list<string> pftbl2list(Pf *pf, const char* tag)
{
    Tbl *t;
    t=pfget_tbl(pf,const_cast<char *>(tag));
    if(t==NULL) throw SeisppError(string("pftbl2list:  ")
            + "pf file is missing Tbl with tag = "
            +tag);
    list<string> result;
    for(int i=0;i<maxtbl(t);++i)
    {
        char *line=(char *)gettbl(t,i);
        result.push_back(string(line));
    }
    return result;
}
