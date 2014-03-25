
#include "rb.h"
#include <stdlib.h>

Rb_node *rb_delete ( root, key ) 
Arr *root ; 
Rb_key *key ; 
{
    Rb_node *c, *t ;
    Rb_node *p, *x, *oldp ; 
    int cmp ;

    x = root->head ; 
    if ( x == root->z ) 
	return 0 ;
    cmp = root->cmp ( key, x->key ) ;
    if ( cmp == 0 ) { 
	if ( x->right == root->z ) 
	    root->head = x->left ; 
	else if ( x->left == root->z ) 
	    root->head = x->right ; 
	else if ( x->right->left == root->z ) 
	    {
	    root->head = x->right ; 
	    root->head->left = x->left ; 
	    }
	else if ( x->left->right == root->z ) 
	    {
	    root->head = x->left ; 
	    root->head->right = x->right ; 
	    }
	else
	    {
	    c = x->right ; 
	    while ( c->left->left != root->z ) 
		c = c->left ; 
	    t = c->left ; 
	    c->left = t->right ;
	    root->head = t ; 
	    t->right = x->right ;
	    t->left = x->left ;
	    }
	root->head->red = FALSE ; 
	root->old.key = x->key ; 
	root->old.value = x->value ; 
	if ( x != root->z ) 
	    free (x) ; 
	else {
	    x->key = 0 ; 
	    x->value = 0 ; 
	    }
	return &root->old ; 
	}


    root->z->key = key ; 

    while ( (cmp = root->cmp ( key, x->key )) != 0 ) 
	{
	p = x ; 
	if ( cmp < 0 ) 
	    x = x->left ; 
	else 
	    x = x->right ; 
	}

    if ( x == root->z ) 
	return 0 ;

    t = x ; 
    
    if ( t->right == root->z ) 
	x = x->left ; 
    else if ( t->right->left == root->z ) 
	{
	x = x->right ; 
	x->left = t->left ; 
	}
    else
	{
	c = x->right ; 
	while ( c->left->left != root->z ) 
	    c = c->left ; 
	x = c->left ; 
	c->left = x->right ; 
	x->left = t->left ; 
	x->right = t->right ; 
	}

    cmp = root->cmp (t->key, p->key) ; 
    if ( cmp < 0  ) 
	{
	oldp = p->left ;
	p->left = x ; 
	}
    else
	{
	oldp = p->right ;
	p->right = x ; 
	}
    root->old.key = oldp->key ;
    root->old.value = oldp->value ;
    if ( oldp != root->z )
	free (oldp) ;
    else {
	oldp->key = 0 ;
	oldp->value = 0 ;
	}
    return &root->old ;
}

/* $Id: rb_delete.c,v 1.2 1997/06/06 16:17:33 danq Exp $ */
