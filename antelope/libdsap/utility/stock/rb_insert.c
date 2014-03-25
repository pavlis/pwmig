

#include "rb.h"

static Rb_node *rb_rotate_root ( gc, c, y ) 
Arr *y ;
Rb_node *gc, *c ; 
{
    if ( gc == c->left ) 
	{
	c->left = gc->right ; gc->right = c ; 
	}
    else
	{
	c->right = gc->left ; gc->left = c ; 
	}
    y->head = gc ; 
    return gc ; 
}

static Rb_node *rb_rotate ( gc, c, y ) 
Rb_node *gc, *c, *y ; 
{
    if ( gc == c->left ) 
	{
	c->left = gc->right ; gc->right = c ; 
	}
    else
	{
	c->right = gc->left ; gc->left = c ; 
	}
    if ( c == y->left ) 
	y->left = gc  ;
    else
	y->right = gc ; 
    return gc ; 
}

Rb_node *rb_split ( root, x, p, g, gg ) 
Arr *root ;
Rb_node *x, *p, *g, *gg ; 
{
    x->red = TRUE ; 
    x->left->red = FALSE ; 
    x->right->red = FALSE ; 

    if ( p->red ) 
	{
	g->red = TRUE ; 
	/* if ( p->left != g->left && p->right != g->right ) */
	if ((x == p->left) != (p == g->left))
	    p = rb_rotate ( x, p, g ) ; 
	if ( g == gg ) 
	    x = rb_rotate_root ( p, g, root ) ;
	else
	    x = rb_rotate ( p, g, gg ) ; 
	x->red = FALSE ; 
	}
    return x ;
}


Rb_node *rb_insert ( root, key, value ) 
Arr *root ; 
Rb_key *key ; 
Rb_value *value ; 
{
    Rb_node *x, *p, *g, *gg ; 
    int result ;

    if ( root->head == root->z ) { 
	x = rb_newnode (root) ;
	x->key = key ;
	x->value = value ;
	root->head = x ;
	}
    else { 
	p = g = x = root->head ; 
	if ( x->left->red && x->right->red ) 
	    x->left->red = x->right->red = FALSE ;
	while ( x != root->z )
	    {
	    gg = g ; 
	    g = p ; 
	    p = x ; 
	    result = root->cmp ( key, x->key ) ;
	    if ( result < 0 ) 
		x = x->left ; 
	    else if ( result == 0 && root->unique ) 
		{
		if ( root->unique > 0 ) 
		    {
		    root->old.key = x->key ; 
		    root->old.value = x->value ;
		    x->key = key ; 
		    x->value = value ; 
		    root->old.red = -1 ;
		    return &root->old ;
		    }
		else
		    return x ;
		}
	    else 
		x = x->right ; 
	    if ( x->left->red && x->right->red ) 
		x = rb_split ( root, x, p, g, gg ) ;
	    }

	x = rb_newnode (root) ; 
	x->key = key ; 
	x->value = value ; 
	x->red = TRUE ; 

	result = root->cmp ( key, p->key ) ;
	if ( result < 0 ) 
	    p->left = x ; 
	else
	    p->right = x ; 

	rb_split ( root, x, p, g, gg ) ; 

	root->head->red = FALSE ; 
	}
    return x ; 
}

/* $Id: rb_insert.c,v 1.2 1997/06/06 16:17:34 danq Exp $ */
