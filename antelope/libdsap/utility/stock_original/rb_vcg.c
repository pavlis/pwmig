
#include "rb.h"
#include <stdio.h>

void rb_shownode ( root, level, x, p, private ) 
Arr *root ; 
int level ;
Rb_node *x ; 
void (*p)() ; 
void *private ;
{
    printf ( "\nLevel %2d Rb_node %8x\n", level, x ) ; 
    printf ( "\t%s\n", x->red ? "Red" : "Black" ) ; 
    printf ( "\tLeft: %8x\n", x->left ) ; 
    printf ( "\tRight: %8x\n", x->right ) ; 
    p(x->key, x->value, private ) ; 

    if ( x->left != root->z ) 
	rb_shownode ( root, level+1, x->left, p, private ) ; 

    if ( x->right != root->z ) 
	rb_shownode ( root, level+1, x->right, p, private ) ; 
}

void rb_show ( root, p, private ) 
Arr *root ; 
void (*p)() ; 
void *private ;
{
    if ( root->head != root->z ) 
	rb_shownode ( root, 0, root->head, p, private ) ; 
}

/* $Id: rb_vcg.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
