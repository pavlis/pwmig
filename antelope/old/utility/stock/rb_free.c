
#include "rb.h"
#include <stdlib.h>

void rb_free_node ( root, x, freenode ) 
Arr *root ;
Rb_node *x ; 
void (*freenode)() ; 
{
    if ( x->left != root->z ) 
	rb_free_node ( root, x->left, freenode ) ;
    if ( x->right != root->z ) 
	rb_free_node ( root, x->right, freenode ) ;
    if ( freenode ) 
	freenode ( x->key, x->value ) ; 
    free ( x ) ; 
}

void rb_free ( root, freenode ) 
Arr *root ; 
void (*freenode)() ; 
{
    if ( root->head != root->z ) 
	rb_free_node ( root, root->head, freenode ) ;
    free ( root->z ) ; 
    free ( root ) ; 
}

/* $Id: rb_free.c,v 1.1.1.1 1997/04/12 04:19:01 danq Exp $ */
