
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "stock.h"

int
newdata ( returnpath, envname, dirname, filename, suffix )
char *returnpath ;
char *envname ; 
char *dirname ; 
char *filename ; 
char *suffix ;
{
    char flocal[FILENAME_MAX] ; 
    int retcode=0 ; 
    char *basename, *envpath, *colon ; 

    flocal[0] = 0 ; 
    if ( dirname != 0 && *dirname != 0 ) {
	strcpy ( flocal, dirname ) ; 
	strcat ( flocal, "/" ) ; 
    }
    strcat ( flocal, filename )  ; 
    if ( suffix != 0 && *suffix != 0 ) { 
	strcat ( flocal, "." ) ; 
	strcat ( flocal, suffix ) ; 
    }

    if ( envname != 0 
	&& *envname != 0 
	&& (envpath = getenv ( envname ) ) != 0 )  {
	strcpy ( returnpath, envpath ) ;
	if ( (colon = strchr(envpath, ':')) != 0 ) 
	    *colon = 0 ; 
	if ( returnpath[strlen(returnpath)-1] != '/' ) 
	    strcat ( returnpath, "/" ) ; 
    } else {
	envpath = getdsap() ;
	strcpy ( returnpath, envpath ) ;
	strcat ( returnpath, "/data/" ) ; 
    }
    strcat ( returnpath, flocal ) ; 

    basename = strrchr ( returnpath, '/' ) ; 
    if ( basename != 0  ) {
	*basename = 0 ; 
	retcode = makedir(returnpath) ; 
	*basename = '/' ;
    }

    if ( retcode == 0 ) 
	if ( access ( returnpath, R_OK ) == 0 ) {
	    if ( access (returnpath, W_OK) == 0 )
		retcode = 1 ;
	    else
		retcode = -1 ;
	}
    return retcode ;
}

/* $Id: newdata.c,v 1.2 1998/02/16 21:26:14 danq Exp $ */
