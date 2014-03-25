
#include "stock.h" 

int ssearch ( key, keysize, index, nkeys, unique, compare, private, ns, ne ) 
char *key, *index ; 
int keysize, nkeys, unique ; 
int (*compare)() ; 
void *private ; 
int *ns, *ne ; 
{
    int try ; 
    int n1 ;
    int n2 ;
    int result ; 
    int in, try2 ; 

    n1 = 0 ; 
    n2 = nkeys-1 ;
    while ( n2 >= n1 ) 
	{
	try = ( n1 + n2 ) / 2 ;
	result = (*compare) ( key, index+try*keysize, private ) ; 
	if      ( result < 0 ) n2 = try-1 ; 
	else if ( result > 0 ) n1 = try+1 ; 
	else   /* result == 0 */ break ;
	}

    if ( unique ) 
	{
	if ( n2 > n1 ) n1 = n2 = try ; 
	}
    else 
	/* {  This is a dumber version of finding the range, but guaranteed 
		to work 
	in = try-1 ; 
	while ( in >= n1 &&  (*compare)(key, index+in*keysize, private) == 0)
	    in -- ; 
	n1 = in+1 ; 

	in = try+1 ; 
	while ( in <= n2 &&  (*compare)(key, index+in*keysize, private) == 0)
	    in ++ ; 
	n2 = in-1 ; 
	}

*/

    
    if ( n2 > n1 ) 
	{
	in = try ;
	while ( in > n1 ) /* narrow range on left */
	    {
	    in-- ; 
	    if ( (*compare)(key, index+in*keysize, private) != 0) 
		{
		n1 = ++in  ;
		break ;
		}
	    try2 = ( n1 + in ) / 2 ;
	    result = (*compare) ( key, index+try2*keysize, private ) ; 
	    if ( result == 0 ) 
		in = try2 ; 
	    else
		n1 = try2+1 ; 
	    }

	in = try ; 
	while ( n2 > in ) /* narrow range on left */
	    {
	    in++ ; 
	    if ( (*compare)(key, index+in*keysize, private) != 0)
		{
		n2 = --in ;
		break ;
		}
	    try2 = ( in + n2 ) / 2 ;
	    result = (*compare) ( key, index+try2*keysize, private ) ; 
	    if ( result == 0 ) 
		in = try2 ; 
	    else 
		n2 = try2-1 ; 
	    }
	}

    *ns = n1 ; 
    *ne = n2 ; 
    return n2-n1+1 ; 
}

/* $Id: ssearch.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
