
#include "stock.h"

Hook *
new_hook (hookfree)
void (*hookfree)() ;
{
    Hook *hook ;

    allot ( Hook *, hook, 1 ) ; 
    hook->magic = HOOK_MAGIC ; 
    hook->free = hookfree ;
    hook->p = 0 ;
    return hook ;
}

void 
free_hook (hookp)
Hook **hookp ; 
{
    Hook *hook ; 

    if ( hookp != 0 && (hook = (Hook *) *hookp) != 0 ) {
	if ( hook->magic == HOOK_MAGIC ) {
	    if ( hook->p != 0 )
		(hook->free)(hook->p) ; 
	    free (hook) ; 
	    *hookp = 0 ;
	} else 
	    die ( 0, "free_hook called with bad pointer.\n" ) ; 
    }
}


/* $Id: hook.c,v 1.1.1.1 1997/04/12 04:19:00 danq Exp $ */
