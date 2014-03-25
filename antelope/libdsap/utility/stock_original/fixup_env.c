
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stock.h"

int 
fixup_env (envsetup, nenv, force)
char *envsetup[] ;
int nenv ;
int force ;
{
    int i ; 
    char *dsap ; 
    static char newval[1024] ; 
    int retcode = 0 ; 

    for ( i=0 ; i<nenv ; i++ ) {
	if ( force || getenv(envsetup[2*i]) == 0 ) {
	    strcpy ( newval, envsetup[2*i]) ; 
	    strcat ( newval, "=" ) ; 
	    strcat ( newval, getdsap())  ;
	    strcat ( newval, "/" ) ; 
	    strcat (newval, envsetup[2*i+1]) ; 
	    retcode += putenv (strdup(newval)) ;
	} 
    }
    return retcode ; 
}

int
envfile ( char *filename ) 
{
    char *path ; 
    FILE *file ; 
    char *name, *value ; 
    char aline[STRSZ], env[STRSZ] ; 

    path = datapath ( 0, "env", filename, "env" ) ;

    if ( path != 0 ) { 
	file = fopen ( path, "r" ) ;  
	if ( file != 0 ) { 
	    while ( fgets ( aline, STRSZ, file ) ) {
		name = strtok ( aline, " \t" ) ; 
		value = strtok ( 0, " \t#" ) ; 
		value[strlen(value)-1] = 0 ; /* remove trailing newline */
		if ( name != 0 && name[0] != '#' ) { 
		    strcpy ( env, name ) ; 
		    strcat ( env, "=" ) ; 
		    if ( value != 0 ) {
			value = expand_env ( value ) ; 
			strcat ( env, value ) ; 
			free(value) ; 
		    }
		    putenv(strdup(env)) ;
		}
	    }
	}
	free(path) ;
    }
    return 0 ;
}

/* $Id: fixup_env.c,v 1.2 1998/02/16 21:26:14 danq Exp $ */
