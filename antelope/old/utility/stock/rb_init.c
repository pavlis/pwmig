
#include "stock.h"

Rb_key * Rb_Key_Min = (char *) -1 ;
Rb_value * Rb_Value_Nil = (char *) -1 ;

Rb_node *rb_newnode (root) 
Arr *root ;
{
    Rb_node *node ; 
    allot ( Rb_node *, node, 1 ) ; 
    node->left = root->z ;
    node->right = root->z ; 
    node->red = FALSE ; 
    return node ; 
}
    

Arr *rb_init ( cmp, unique ) 
int (*cmp) () ; 
int unique ;
{
    Arr *root ; 
    Rb_node *z ; 

    allot ( Arr *, root, 1 ) ; 
    root->z = 0 ; 
    z = rb_newnode (root) ; 
    z->left = z ; 
    z->right = z ; 

    root->z = z ;
    root->head = z ;
    root->cmp = cmp ;  
    root->unique = unique ;
    return root ; 
}


/* $Id: rb_init.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
