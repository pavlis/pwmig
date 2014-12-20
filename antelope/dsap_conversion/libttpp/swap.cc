#define _BSD_SOURCE
#include <typeinfo>
#include <sys/types.h>
//#include <endian.h>
#include "swapbytes_pwmig.h"
using namespace std;

/* This is the old antelope routine to swap bit endian 32 bit vectors  of
   length n*/
void vector_swap4(int *in,int *out,int n)
{
    u_int32_t so;  //so is short for swapped output
    int i;
    for(i=0;i<n;++i)
    {
        so=be32toh(static_cast<u_int32_t>(in[i]));
        out[i]=static_cast<int>(so);
    }
}
void md2hd(double *in, double *out,int n)
{
    u_int64_t so;
    int i;
    for(i=0;i<n;++i)
    {
        so=be64toh(static_cast<u_int64_t>(in[i]));
        out[i]=static_cast<double>(so);
    }
}
/*Added this one for efficiency.  The taup f2c junk uses a define to
for a SWAP4 function with a single argument to swap one 32 bit integer.
With C++ we can use overloading to avoid the define.
*/
void swap4(int *d)
{
    u_int32_t so;
    so=be32toh(static_cast<u_int32_t>(*d));
    *d=static_cast<int>(so);
}


