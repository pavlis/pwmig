
/* This set of routines is to implement sorted tables.
 */

#include <string.h>
#include "stock.h"

Stbl *newstbl ( cmp ) 
int (*cmp)() ; 
{
    Stbl *stbl ; 
    allot (Stbl *, stbl, 1 ) ; 
    stbl->tbl = 0 ; 
    stbl->changed = 1 ; 
    stbl->arr = newarr(cmp) ; 
    stbl->arr->unique = -1 ;
    return stbl ; 
}

void *addstbl ( stbl, key ) 
Stbl *stbl ;
void *key ; 
{
    Rb_node *node ;
    stbl->changed = 1 ;
    node = rb_insert ( stbl->arr, key, key ) ;
    return node->key  ; 
}

void *delstbl ( stbl, key ) 
Stbl *stbl ;
void *key ; 
{
    Rb_node *old ; 
    stbl->changed = 1 ;
    old = rb_delete ( stbl->arr, key ) ;
    return (old == 0) ? (Rb_key *) 0 : old->key ;
}

void *tststbl ( stbl, key )
Stbl *stbl ; 
void *key ; 
{
    return rb_search ( stbl->arr, key ) ; 
}

static void fixstbl ( stbl ) 
Stbl *stbl; 
{
    stbl->changed = 0 ; 
    if ( stbl->tbl != 0 ) 
	freetbl (stbl->tbl, 0) ;
    stbl->tbl = keysarr(stbl->arr) ;
}

int maxstbl ( stbl ) 
Stbl *stbl ;
{
    if ( stbl->changed ) fixstbl(stbl) ;
    return maxtbl(stbl->tbl) ; 
}

void *getstbl ( stbl, index ) 
Stbl *stbl ; 
int index ; 
{
    if ( stbl->changed ) fixstbl(stbl) ; 
    return gettbl(stbl->tbl, index) ; 
}

static void stbl_free_node ( root, x, freenode )
Arr *root ;
Rb_node *x ;
void (*freenode)() ;
{
    if ( x->left != root->z )
	stbl_free_node ( root, x->left, freenode ) ;
    if ( x->right != root->z )
	stbl_free_node ( root, x->right, freenode ) ;
    if ( freenode )
	freenode ( x->value ) ;
    free ( x ) ;
}   

static int freestblarr ( root, free_value )
Arr *root ;
void (*free_value) () ;
{
    if ( root->head != root->z )
	stbl_free_node ( root, root->head, free_value ) ;
    free ( root->z ) ;
    free ( root ) ;
    return 0 ;
}

int freestbl ( stbl, free_stblval )
Stbl *stbl ; 
void (*free_stblval) () ; 
{
    if ( stbl->tbl ) 
	freetbl ( stbl->tbl, 0 ) ; 
    freestblarr ( stbl->arr, free_stblval ) ; 
    free (stbl) ;
    return 0 ; 
}

int applystbl ( stbl, function, private )
Stbl *stbl ; 
int (*function) () ; 
void *private ; 
{
    if ( stbl->changed ) fixstbl(stbl) ; 
    applytbl ( stbl->tbl, function, private ) ; 
    return 0 ; 
}

/* $Id: stbl.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
