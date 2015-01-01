/* This is a local include file to define glp implementation of 
   swapbyte routines in antelope.   Needed only for prototypes.*/
#ifdef  __cplusplus
extern "C" {
#endif
void vectorswap4(int *in,int *out,int n);
void swap4(int *d);
void swap4_float(float *d);
void md2hd(double *in, double *out,int n);
#ifdef  __cplusplus
}
#endif
