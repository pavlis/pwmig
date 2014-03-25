void isort ( char *a, int n , int size ,int (*compare)() , void *private) ; 

void isort_ ( a, f_n, f_size, compare, private )
char *a ;
int *f_n ;
int *f_size ;
int (*compare)() ;
void *private ;
{
    isort(a, *f_n, *f_size, compare, private );
}


/* $Id: isort_.c,v 1.1.1.1 1997/04/12 04:19:01 danq Exp $ */
