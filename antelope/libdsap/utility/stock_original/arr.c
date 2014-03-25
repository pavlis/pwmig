
/* This set of routines is to implement associative arrays
 */

#include <string.h>
#include "stock.h"

typedef struct Node { 
    char *key, *value ;
    } Node ; 

Arr *newarr (cmp) 
int (*cmp)() ; 
{
    Arr *arr ; 
    int unique = 1 ;
    if ( cmp == 0 ) 
	cmp = strcmp ; 
    arr = rb_init ( cmp, unique ) ; 
    return arr ; 
}

void *delarr ( arr, key ) 
Arr *arr ; 
char *key ; 
{
    Rb_node *old ; 
    char *oldvalue = 0 ;

    old = rb_delete ( arr, key ) ; 
    if ( old ) 
	{
	oldvalue = old->value ; 
	free ( old->key ) ; 
	}
    return oldvalue ; 
}

void *setarr ( arr, key, value ) 
Arr *arr ; 
char *key ; 
void *value ; 
{
    Rb_node *old ; 
    void *oldvalue = 0 ;

    old = rb_insert ( arr, strdup(key), value ) ; 
    if ( old->red < 0 ) 
	{
	oldvalue = old->value ; 
	free ( old->key ) ; 
	}
    return oldvalue ; 
}

void *getarr ( arr, key ) 
Arr *arr ; 
char *key ; 
{
    return rb_search ( arr, key ) ; 
}

static void addkey ( key, value, private ) 
Rb_key *key ; 
Rb_value *value ;
Tbl *private ;
{
    pushtbl ( private, key ) ; 
}
	

Tbl *keysarr ( arr ) 
Arr *arr ; 
{
    Tbl *keys ;

    keys = newtbl (0) ; 
    rb_traverse ( arr, addkey, keys ) ; 
    return keys ; 
}


static void addvalue ( key, value, private ) 
Rb_key *key ; 
Rb_value *value ;
Tbl *private ;
{
    pushtbl ( private, value ) ; 
}

Tbl *valsarr ( arr ) 
Arr *arr ; 
{
    Tbl *values ;

    values = newtbl (0) ; 
    rb_traverse ( arr, addvalue, values ) ; 
    return values ; 
}

static void arr_free_node ( root, x, freenode ) 
Arr *root ;
Rb_node *x ; 
void (*freenode)() ; 
{
    if ( x->left != root->z ) 
	arr_free_node ( root, x->left, freenode ) ;
    if ( x->right != root->z ) 
	arr_free_node ( root, x->right, freenode ) ;
    if ( freenode ) 
	freenode ( x->value ) ; 
    free(x->key) ; 
    free ( x ) ; 
}

int freearr ( root, free_value ) 
Arr *root ; 
void (*free_value) () ; 
{
    if ( root->head != root->z ) 
	arr_free_node ( root, root->head, free_value ) ;
    free ( root->z ) ; 
    free ( root ) ; 
    return 0 ; 
}
	
static void arr_walk ( root, x, p, private ) 
Arr *root ;
Rb_node *x ; 
void (*p)() ; 
char *private ;
{
    if ( x->left != root->z ) 
	arr_walk ( root, x->left, p, private) ; 
    p(x->key, x->value, private ) ; 
    if ( x->right != root->z ) 
	arr_walk ( root, x->right, p, private ) ; 
}

static void
cntnode ( char *key, char *value, void *private ) 
{
    int *cnt ; 
    cnt = private ; 
    (*cnt)++ ;
}

long cntarr ( Arr *arr ) 
{
    long cnt = 0 ; 
    if (arr->head != arr->z ) 
	arr_walk ( arr, arr->head, cntnode, (void *) &cnt ) ; 
    return cnt ; 
}

int applyarr ( arr, function, private ) 
Arr *arr ; 
void (*function) () ; 
void *private ;
{
    if (arr->head != arr->z ) 
	arr_walk ( arr, arr->head, function, private ) ; 
    return 0 ; 
}


/* $Id: arr.c,v 1.3 1997/08/14 21:48:53 danq Exp $ */
