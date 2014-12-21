#include <typeinfo>
#include <sys/types.h>
#include <iostream>
#include "swapbytes_pwmig.h"
/* Taken from:  
http://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
*/
template <typename T>
void SwapEnd(T& var)
{
    T varSwapped;
    for(long i = 0; i < static_cast<long>(sizeof(var)); i++)
         ((char*)(&varSwapped))[sizeof(var) - 1 - i] = ((char*)(&var))[i];
    for(long i = 0; i < static_cast<long>(sizeof(var)); i++)
                          ((char*)(&var))[i] = ((char*)(&varSwapped))[i];
}

using namespace std;

/* This is the old antelope routine to swap bit endian 32 bit vectors  of
   length n*/
void vectorswap4(int *in,int *out,int n)
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
    int i;
    //cout << "Byte swapped double result - length="<<n<<endl;
    for(i=0;i<n;++i)
    {
        //cout << in[i]<<" ";
        out[i]=in[i];
        SwapEnd(out[i]);
        //cout << out[i]<<endl;
    }
}
void swap4(int *d)
{
    int32_t so;
    so=__builtin_bswap32(static_cast<int32_t>(*d));
    *d=static_cast<int>(so);
}
void swap4_float(float *d)
{
    int32_t so;
    so=__builtin_bswap32(static_cast<int32_t>(*d));
    *d=static_cast<int>(so);
}

