
#include <memory.h>
#include "stock.h"

void shellsort_ ( a, f_n, f_size, compare, private )
char *a ;
int *f_n ;
int *f_size ;
int (*compare)() ;
void *private ;
{
	int n ; 
	int size ; 

	n = *f_n ; 
	size = *f_size ; 

	shellsort(a, n, size, compare, private );
}


/* $Id: shellsort_.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
