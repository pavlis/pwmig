
#include "stock.h"

void rb_level_traverse ( root, p, private ) 
Arr *root ; 
void (*p)() ; 
void *private ;
{
    Tbl *tbl ; 
    Rb_node *node ; 
    tbl = newtbl ( 20 ) ; 
    if ( root->head != root->z ) 
	pushtbl ( tbl, root->head ) ;
    while ( maxtbl(tbl) ) { 
	node = (Rb_node *) shifttbl(tbl) ; 
	p(node->key, node->value, private) ; 
	if ( node->left != root->z ) pushtbl ( tbl, node->left ) ; 
	if ( node->right != root->z ) pushtbl ( tbl, node->right ) ; 
	}
    freetbl ( tbl, 0 ) ; 
}

/* $Id: rb_level_traverse.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
