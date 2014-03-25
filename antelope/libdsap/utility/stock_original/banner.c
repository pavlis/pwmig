/* This routine can be used to print out a standard DSAP banner line, 
 * along with a program name and version.
 * $Name $Revision: 1.4 $ $Date: 1998/01/05 05:39:37 $
 */

#include <stdio.h>

void banner ( program, version ) 
char *program, *version ; 
{
    fprintf ( stderr, "\n%s %s ** Datascope Application Package 3.4 (PASSCAL Release) **\n", 
	    program, version ) ;
    fprintf ( stderr, 
	"\n\t  Please report problems to the PASSCAL Instrument Center\n" ) ; 
}

#ifdef DEBUG

main ()
{
    banner( "test program", "Version 1.5" ) ; 
}

#endif

/* $Id: banner.c,v 1.4 1998/01/05 05:39:37 danq Exp $ */
