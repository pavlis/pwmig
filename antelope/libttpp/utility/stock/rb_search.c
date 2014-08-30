
#include "rb.h"

Rb_value * rb_search ( root, key ) 
Arr *root ; 
Rb_key *key ; 
{
    Rb_node * x = root->head ; 
    int cmp ;

    root->z->key = key ; 
    while ( (cmp = root->cmp ( key, x->key) ) != 0 )  
	{
	if ( cmp < 0 ) 
	    x = x->left ; 
	else 
	    x = x->right ; 
	}
    if ( x == root->z ) 
	return 0 ; 
    else
	return x->value ; 
}


/* $Id: rb_search.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
