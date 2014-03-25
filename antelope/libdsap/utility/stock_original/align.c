
#include <sys/types.h>

#define LOW_INT_BITS (sizeof(int)-1)
#define HIGH_INT_BITS (~ LOW_INT_BITS)
#define LOW_DOUBLE_BITS (sizeof(double)-1)
#define HIGH_DOUBLE_BITS (~ LOW_DOUBLE_BITS)


int align_int(i)
int i ; 
{
    unsigned int high, low ; 

    high = i & HIGH_INT_BITS ; 
    low = i & LOW_INT_BITS ; 
    return high + (low ? sizeof(int) : 0) ; 

}


int align_double(i)
int i ; 
{
    unsigned int high, low ; 
    high = i & HIGH_DOUBLE_BITS ; 
    low = i & LOW_DOUBLE_BITS ; 
    return high + (low ? sizeof(double) : 0) ; 
}

/* $Id: align.c,v 1.1.1.1 1997/04/12 04:18:59 danq Exp $ */
