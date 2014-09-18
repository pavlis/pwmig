#include <stdio.h>
#include <string>
#include "Hypocenter.h"
using namespace std;
using namespace SEISPP;
using namespace PWMIG;
int main(int argc, char **argv)
{
try{
    ios::sync_with_stdio();
    cout << "Hypocenter test routine"<<endl
        <<"Calling default constructor"<<endl;
    string method("tttaup"),model("iasp91");
    PWMIG::Hypocenter h(0.0,0.0,0.0,0.0,method,model);
    //Hypocenter h;
    double t;
    //t=h.ptime(rad(1.0),0.0,0.0);
    cout << t <<endl;

}catch(SeisppError& serr)
{
    serr.log_error();
}
}

