#include <iostream>
#include "SlownessVectorMatrix.h"
using namespace SEISPP;
using namespace PWMIG;
using namespace std;
int main(int argc, char **argv)
{
    cout << "Testing 5x5 constructor"<<endl;
    SlownessVectorMatrix svm(5,5);
    cout << "filling matrix with meaningless vectors"<<endl;
    int i,j;
    for(i=0;i<5;++i)
        for(j=0;j<5;++j)
        {
            SlownessVector u((double)i,(double)j);
            svm(i,j)=u;
        }
    cout << "Filled, extracting vectors defined as ux=row, uy=col"<<endl;
    cout << "Success  - Output using operator <<"<<endl;
    cout << svm<<endl;
    cout << "Testing copy constructor"<<endl;
    SlownessVectorMatrix cpy(svm);
    cout << "Copy contents"<<endl;
    cout << cpy <<endl;
    cout << "Testing operator="<<endl;
    cpy=svm;
    cout << "Copy contents"<<endl;
    cout << cpy <<endl;
    cout << "Finished"<<endl;
    try {
        SlownessVector u;
        u=svm(10,5);
    }catch(SeisppError& serr)
    {
        cerr << "Test of exception handler"<<endl
            << "Will be out of order as this is to stderr"<<endl;;
        serr.log_error();
    }
}


