
#include <errno.h>
#include <stdio.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include "stock.h" 

#ifdef __STDC__

char *ask(char *fmt, ...)
{

#else

char *
ask(va_alist)
va_dcl
{
    char           *fmt;

#endif
    va_list         args;
    FILE *file ; 
    char aline[STRSZ], *response ;

    file = fopen ( "/dev/tty", "w" ) ; 
    if ( file == 0 ) 
	die ( 1, "Can't open /dev/tty.\n" ) ; 
 
    fflush(file) ; 
    fprintf(file, "%s: ", Program_Name);
 
 
#ifdef __STDC__
    va_start(args, fmt);
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif

    if (fmt != NULL && *fmt != NULL)
        vfprintf(file, fmt, args);
    else
        fputs("\n", file);
    va_end(args) ;
    fclose (file); 

    file = fopen ( "/dev/tty", "r" ) ; 
    fgets ( aline, STRSZ, file ) ; 
    fclose ( file ) ; 
    aline[strlen(aline)-1] = 0 ;
    response = strdup ( aline ) ; 
    return response ; 
}
 
static char *yes[]  = { "y", "yes", "ok" }  ;
static int nyes = sizeof ( yes ) / sizeof ( char * ); 

static char *no[]  = { "n", "no" }  ;
static int nno = sizeof ( no ) / sizeof ( char * ); 

#ifdef __STDC__

int askyn(char *fmt, ...)
{
#else

int
askyn(va_alist)
va_dcl
{
    char           *fmt;
#endif

    va_list         args;
    FILE *file ; 
    char aline[STRSZ] ;
    int i ; 
 
    while ( 1 ) 
	{

	file = fopen ( "/dev/tty", "w" ) ; 
	if ( file == 0 ) 
	    die ( 1, "Can't open /dev/tty.\n" ) ; 
     
	fflush(file) ; 
	fprintf(file, "%s: ", Program_Name);
     
#ifdef __STDC__
	va_start(args,fmt) ; 
#else
	va_start(args);
	fmt = va_arg(args, char *);
#endif
	if (fmt != NULL && *fmt != NULL)
	    vfprintf(file, fmt, args);
	else
	    fputs("\n", file);
	va_end(args) ;
	fclose (file); 

	file = fopen ( "/dev/tty", "r" ) ; 
	fgets ( aline, STRSZ, file ) ; 
	fclose ( file ) ; 
	aline[strlen(aline)-1] = 0 ;
	for (i=0 ; i<nyes ; i++) 
	    if ( strcmp ( aline, yes[i] ) == 0 ) return 1 ; 

	for (i=0 ; i<nno ; i++) 
	    if ( strcmp ( aline, no[i] ) == 0 ) return 0 ; 

	file = fopen ( "/dev/tty", "w" ) ; 
	fflush(file) ; 
	fprintf(file, "Please answer 'yes' or 'no'\n" );
	fclose (file) ;
	}
}
 

/* $Id: ask.c,v 1.1.1.1 1997/04/12 04:18:59 danq Exp $ */
