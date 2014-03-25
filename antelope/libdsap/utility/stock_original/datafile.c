/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "stock.h"

static char Dsappath[FILENAME_MAX] ; 
static char *Default_dsappath = "/opt/dsap" ;

char *
getdsap ()
{
    char *me, *dsappath ;
    char *slash ; 
    char **argv ;
    int n ;

    if ( (dsappath = getenv ( "DSAP" )) != 0 ) {
	return dsappath ; 
    }

    elog_query ( ELOG_ARGV, 0, (void *) &argv ) ; 
    if ( argv == 0 ) { /* elog not initialized, give up */
	return Default_dsappath ; 
    } else { 
	me = argv[0] ;
    }

    if ( strchr(me, '/') == 0 ) { 
	/* bare name -- have to find along PATH */
	char *path ; 
	if ( (path = datapath ( "PATH", 0, me, "" )) == 0 ) { 
	    return Default_dsappath ; /* can't figure out path to program */
	} else { 
	    strncpy ( Dsappath, path, FILENAME_MAX ) ; 
	    free ( path ) ; 
	}
    } else {
      strcpy ( Dsappath, me ) ;
      slash = strrchr(Dsappath, '/' ) ; 
      *slash = 0 ; 
      if ( *me != '/' ) {
	  char *old, *pwd ; 
	  /* relative path! */
	  old = getcwd(0, FILENAME_MAX) ; 
	  if ( chdir(Dsappath) != 0 ) {
	    return Default_dsappath ; /* die ( 1, "Can't cd to bin directory '%s'\n", Dsappath ) ; */
	  }
	  pwd = getcwd(0, FILENAME_MAX);
	  strcpy ( Dsappath, pwd ) ; 
	  free ( pwd ) ; 
	  if ( chdir(old) != 0 ) {
	    die ( 1, "getdsap can't return to original directory '%s'\n", old ) ; 
	  }
	  free ( old ) ; 
      }
    }

    n = strlen(Dsappath) ; 
    if (   Dsappath[n-4] != '/'
	|| Dsappath[n-3] != 'b'
	|| Dsappath[n-2] != 'i'
	|| Dsappath[n-1] != 'n' ) {
	return Default_dsappath ; /* 
	    die ( 0, "bad configuration: '%s' has no final bin component.\n",
	    Dsappath ) ; */
	}
    Dsappath[n-4] = 0 ; 

    return Dsappath;
}

char
               *
datapath (envname, dirname, filename, suffix)
char           *envname;
char           *dirname;
char           *filename;
char           *suffix;
{
    char            local[FILENAME_MAX];

    local[0] = 0;
    if (dirname != 0 && *dirname != 0) {
	strcpy (local, dirname);
	strcat (local, "/");
    }
    strcat (local, filename);
    if (suffix != 0 && *suffix != 0) {
	strcat (local, ".");
	strcat (local, suffix);
    }
    return datafile (envname, local);
}


char
               *
datafile (envname, filename)
char           *envname;
char           *filename;
{
    char           *path,
                   *localpath;
    char            local[FILENAME_MAX];
    Tbl            *pathtbl;
    int             i,
                    n;
    struct stat     statbuf;
    char           *basename,
                   *save;

    basename = strrchr (filename, '/');
    if (basename)
	basename++;

    if (envname != 0
	    && *envname != 0
	    && (path = getenv (envname)) != 0) {
	if (strchr (path, ':') == 0) {
	    if (stat (path, &statbuf) == 0) {
		if (S_ISDIR (statbuf.st_mode)) {
		    strcpy (local, path);
		    strcat (local, "/");
		    save = local + strlen (local);
		    strcat (local, filename);
		    if (access (local, R_OK) == 0)
			return strdup (local);
		    if (basename) {
			*save = 0;
			strcat (local, basename);
			if (access (local, R_OK) == 0)
			    return strdup (local);
		    }
		} else if (access (path, R_OK) == 0)
		    return strdup (path);
	    }
	} else {
	    localpath = strdup (path);
	    pathtbl = split (localpath, ':');
	    n = maxtbl (pathtbl);
	    for (i = 0; i < n; i++) {
		strcpy (local, gettbl (pathtbl, i));
		if (stat (local, &statbuf) == 0) {
		    if (S_ISDIR (statbuf.st_mode)) {
			strcat (local, "/");
			save = local + strlen (local);
			strcat (local, filename);
			if (access (local, R_OK) == 0) {
			    freetbl (pathtbl, 0);
			    free(localpath) ;
			    return strdup (local);
			}
			if (basename) {
			    *save = 0;
			    strcat (local, basename);
			    if (access (local, R_OK) == 0) {
				freetbl (pathtbl, 0);
				free(localpath) ;
				return strdup (local);
			    }
			}
		    } else {
			if (access (local, R_OK) == 0) {
			    freetbl (pathtbl, 0);
			    free(localpath) ;
			    return strdup (path);
			}
		    }
		}
	    }
	    freetbl (pathtbl, 0);
	    free (localpath);
	}
    }

    path = getdsap() ;

    strcpy (local, path);
    strcat (local, "/data/");
    strcat (local, filename);
    if (access (local, R_OK) == 0)
	return strdup (local);

    return 0;
}

/* $Id: datafile.c,v 1.4 1998/02/16 21:26:13 danq Exp $ */
