
#include "stock.h"

typedef struct Node {
    int level ; 
    Rb_node *node ; 
    } Node ;

void pnode ( root, level, node, p, private ) 
Arr *root ;
int level ;
Rb_node *node ;
void (*p)() ; 
void *private ;
{
    p(node->key, node->value, private ) ;
    if ( node->red ) 
	printf ( "+" ) ; 
}


void rb_lshow ( root, p, private ) 
Arr *root ; 
void (*p)() ; 
void *private ;
{
    Tbl *tbl ; 
    Rb_node *node, *left, *right ; 
    Node anode ;
    int lastlevel = -1 ;
    int flag=1 ;

    tbl = inittbl ( 0, 20, 1, 0, sizeof(Node) ) ; 
    printf ( "\n*** Red-Black Tree ***" ) ; 
    if ( root->head != root->z ) 
	{
	anode.level = 0 ; 
	anode.node = root->head ; 
	pushtbl ( tbl, &anode ) ;
	}
    while ( maxtbl(tbl) ) { 
	memmove ( &anode,  shifttbl(tbl), sizeof(Node)) ; 

	if ( anode.level > lastlevel ) { 
	    if ( flag == 0 ) 
		break ;
	    flag = 0 ;
	    printf ( "\nLevel #%3d ", anode.level ) ; 
	    lastlevel = anode.level ; 
	    }

	if ( anode.node == root->z ) 
	    printf ( " Z" ) ; 
	else
	    {
	    pnode(root, anode.level, anode.node, p, private ) ; 
	    flag = 1 ; 
	    }

	anode.level++ ;
	left = anode.node->left ; 
	right = anode.node->right ;
	    
	anode.node = left ;
	pushtbl ( tbl, &anode ) ;
	
	anode.node = right ;
	pushtbl ( tbl, &anode ) ;
	}
    printf ( "\n**********************\n" ) ; 
    freetbl ( tbl, 0 ) ; 
}

/* $Id: rb_lshow.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
