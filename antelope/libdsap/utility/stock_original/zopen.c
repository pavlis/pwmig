
/* This routine should run compress on a file in the background */ 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "stock.h"

void compress ( path ) 
char *path ;
{
    char cmd[STRSZ] ; 
    sprintf ( cmd, "compress %s &\n", path ) ; 
    system ( cmd ) ; 
}

FILE *zopen ( filename, permissions ) 
char *filename ; 
char *permissions ; 
{ 
    char *cp ; 
    FILE *file ;
    char cmd[STRSZ] ; 

    cp = filename + strlen(filename) ; 
    if ( strcmp ( cp-2, ".Z" ) == 0 
	|| strcmp ( cp-3, ".gz" ) == 0 ) 
	{
	char *tmp ; 
	tmp = tempnam ( "/usr/tmp", "jstmp" ) ; 
	sprintf ( cmd, "zcat %s > %s", filename, tmp ) ; 
	system ( cmd ) ; 
	file = fopen ( tmp, permissions ) ; 
	unlink ( tmp ) ; 
	free (tmp) ; 
	}
    else
	file = fopen ( filename, permissions ) ; 
    return file ; 
}

/* $Id: zopen.c,v 1.1.1.1 1997/04/12 04:19:03 danq Exp $ */
