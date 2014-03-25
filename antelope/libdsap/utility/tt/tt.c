#include <stdio.h>
#include <dlfcn.h>

#include "tt.h"
#include "stock.h"

typedef struct TT_method {
    void *library ; 
    int  (*_ttcalc)( char *model,  
		    char *phase_code,
		    int mode,
		    TTGeometry *geometry,
		    Tbl **times,
		    Hook **hook) ;
    Hook *hook ;
} TT_method ;

typedef struct TT_hook {
    Arr *open_methods ; 
} TT_hook ;


static void
TT_free_method ( void *tt_methodp )
{
    TT_method *tt_method  = (TT_method *) tt_methodp ;
    free_hook ( &(tt_method->hook) ) ; 
    free(tt_method) ; 
}

static void
TT_free_hook ( TT_hook *tt_hook ) 
{
    freearr ( tt_hook->open_methods, TT_free_method ) ;
}

int ttcalc (
    char *method,
    char *model, 
    char *phase_code, 
    int mode, 
    TTGeometry *geometry,
    Tbl **timesp,
    Hook **hook
)
{
    void *library ;
    int retcode ;
    TT_hook *tt_hook ;
    TT_method *tt_method ; 
    int  (*_ttcalc)( char *model,  
		    char *phase_code,
		    int mode,
		    TTGeometry *geometry,
		    Tbl **times,
		    Hook **hook) ;

    if ( *hook == NULL ) {
	*hook = new_hook ( TT_free_hook ) ; 
	allot ( TT_hook *, tt_hook, 1 ) ;
	tt_hook->open_methods = newarr(0) ; 
	(*hook)->p = tt_hook ;
    } else {
	tt_hook = (*hook)->p ; 
    }

    if ( *timesp == NULL ) {
        *timesp = inittbl ( 0, 0, 1, 0L, sizeof(TTTime)) ;
    } else {
        clrtbl(*timesp,NULL) ;
    }

    if ( (tt_method = getarr(tt_hook->open_methods, method)) == NULL ) {
	char library_name[STRSZ] ;

	strcpy ( library_name, "lib" ) ; 
	strcat ( library_name, method ) ; 
	strcat ( library_name, ".so" ) ; 
	library = dlopen ( library_name, RTLD_LAZY ) ;
	if ( library == NULL ) {
	    register_error ( 0, "Can't open travel time library '%s' (for method '%s')\n", 
		library_name, method ) ; 
	    return -3 ; 
	}

	_ttcalc = (int (*)(char *, char *, int, TTGeometry *, Tbl **, Hook **))dlsym(library, method);
	if ( _ttcalc == 0 ) {
	    register_error ( 0, "Can't find entry point '%s' in library '%s'\n",
		method, library_name ) ; 
	    return -3 ; 
	}

	allot ( TT_method *, tt_method, 1 ) ;
	tt_method->library = library ;
	tt_method->_ttcalc = _ttcalc ;
	tt_method->hook = 0 ;
	setarr(tt_hook->open_methods, method, tt_method) ;
    } else {
	_ttcalc = tt_method->_ttcalc ;
    }

    retcode = (*_ttcalc)(model, phase_code, mode, geometry, 
	timesp, &(tt_method->hook))  ;
    return retcode ; 
}

/* $Id: tt.c,v 1.1.1.1 1997/04/12 04:19:08 danq Exp $ */
