
/* This sort is good for small, or almost ordered arrays ; 
   the routine is coded directly from Sedgewick, Algorithms in Modula 3, pp 100*/

#include "stock.h"

void isort ( a, n, size, compare, private )  
 char *a ;
 int n ;
 int size ;
 int (*compare)() ;
 void *private ; 
{
    int i, j ; 
    char *s ; 

    allot ( char *, s, size ) ; 
    for ( i=1 ; i<n ; i++ ) {
	j = i ; 
	memcpy ( s, a+j*size, size ) ; 
	while ( j>0 && (*compare) ( a+(j-1)*size, s, private ) > 0 ) { 
	    memcpy ( a+j*size, a+(j-1)*size, size ) ; 
	    j-- ; 
	}
	memcpy ( a+j*size, s, size ) ; 
    }

    free(s);
}

#ifdef DEBUG

#include <stdio.h>


usage()
{
    banner ( Program_Name, "alpha" ) ; 
    fprintf(stderr, "Usage: %s\n", Program_Name);
    exit(1);
}

int ccmp ( a, b, private ) 
char *a, *b ; 
void *private ; 
{ 
    return *a-*b ; 
}

int
main(argc, argv)
int             argc;
char          **argv;
{

    extern char    *optarg;
    extern int      optind;
    int             c, errflg = 0 ;
    char	    aline[STRSZ] ;

    Program_Name = argv[0];

    while ((c = getopt(argc, argv, "V")) != -1)
	switch (c)
	  {
	  case 'V':
	    usage() ;
	    break;
	  case '?':
	    errflg++;
	  }
    if (errflg)
	usage();

    while ( gets(aline) ) { 
	    printf ( "input : %s\n", aline ) ; 
	    isort ( aline, strlen(aline), 1, ccmp, 0 ) ;
	    printf ( "sorted: %s\n", aline ) ; 
	  }
    return 0;
}
#endif

/* $Id: isort.c,v 1.1.1.1 1997/04/12 04:19:01 danq Exp $ */
