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
    int32_t so;  //so is short for swapped output
    int i;
    for(i=0;i<n;++i)
    {
        so=__builtin_bswap32(static_cast<int32_t>(in[i]));
        out[i]=static_cast<int>(so);
    }
}
void md2hd(double *in, double *out,int n)
{
    int64_t so;
    int i;
    for(i=0;i<n;++i)
    {
        so=__builtin_bswap64(static_cast<int64_t>(in[i]));
        out[i]=static_cast<double>(so);
    }
}
/*Added this one for efficiency.  The taup f2c junk uses a define to
for a SWAP4 function with a single argument to swap one 32 bit integer.
With C++ we can use overloading to avoid the define.
*/
void swap4(int *d)
{
    int32_t so;
    so=__builtin_bswap32(static_cast<int32_t>(*d));
    *d=static_cast<int>(so);
}


