
#include "rb.h"

void rb_walk ( root, x, p, private ) 
Arr *root ; 
Rb_node *x ; 
void (*p)() ; 
void *private ;
{
    if ( x->left != root->z ) 
	rb_walk ( root, x->left, p, private ) ; 
    p(x->key, x->value, private ) ; 
    if ( x->right != root->z ) 
	rb_walk ( root, x->right, p, private ) ; 
}

void rb_traverse ( root, p, private ) 
Arr *root ; 
void (*p)() ; 
void *private ;
{
    if ( root->head != root->z ) 
	rb_walk ( root, root->head, p, private ) ; 
}

/* $Id: rb_traverse.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
