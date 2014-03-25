
#ifndef _rbtree_
#define _rbtree_

#define TRUE 1
#define FALSE 0
/** 
 * The following will implement associative arrays as red-black
 * balanced trees.  These trees may be represented either in memory
 * or as a file in the filesystem.
 */

typedef char Rb_key ; 
typedef char Rb_value ;

extern Rb_key * Rb_Key_Min ;
extern Rb_value * Rb_Value_Nil ;

typedef struct Rb_node { 
    short red ; 
    struct Rb_node *left, *right ; 
    Rb_key *key ; 
    Rb_value *value ; 
    } Rb_node ; 

typedef struct Rb_tree { 
    int flag ;
    int unique ; 
    int (*cmp)() ; 
    Rb_node *head ; 
    Rb_node *z ; 
    Rb_node old ;
    } Arr ;

extern Rb_value *Rb_Value_Nil ;

#ifdef __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */

extern Rb_node *rb_delete PL_(( Arr *root, Rb_key *key ));
extern void rb_free_node PL_(( Arr *root, Rb_node *x, void (*freenode)() ));
extern void rb_free PL_(( Arr *root, void (*freenode)() ));
extern Rb_node *rb_newnode PL_(( Arr *root ));
extern Arr *rb_init PL_(( int (*cmp) (), int unique ));
extern Rb_node *rb_split PL_(( Arr *root, Rb_node *x, Rb_node *p, Rb_node *g, Rb_node *gg ));
extern Rb_node *rb_insert PL_(( Arr *root, Rb_key *key, Rb_value *value ));
extern void rb_level_traverse PL_(( Arr *root, void (*p)(), void *private ));
extern void pnode PL_(( Arr *root, int level, Rb_node *node, void (*p)(), void *private ));
extern void rb_lshow PL_(( Arr *root, void (*p)(), void *private ));
extern Rb_value * rb_search PL_(( Arr *root, Rb_key *key ));
extern void rb_shownode PL_(( Arr *root, int level, Rb_node *x, void (*p)(), void *private ));
extern void rb_show PL_(( Arr *root, void (*p)(), void *private ));
extern void rb_walk PL_(( Arr *root, Rb_node *x, void (*p)(), void *private ));
extern void rb_traverse PL_(( Arr *root, void (*p)(), void *private ));

#undef PL_

#endif

/* $Id: rb.h,v 1.2 1997/06/06 16:17:32 danq Exp $ */
