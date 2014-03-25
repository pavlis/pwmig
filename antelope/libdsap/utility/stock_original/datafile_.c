
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "stock.h"

int cs2fs ( char *dst, char *src, int nmax );

void datafile_ ( Result, f_envname, f_filename, n_Result, n_envname, n_filename )
char *Result ;
char *f_envname ;
char *f_filename ;
int n_Result;
int n_envname ;
int n_filename ;
{

	char *filename, *envname ;
	char *retcode;

	envname = newcs(f_envname, n_envname);
	filename = newcs(f_filename, n_filename);

	retcode = datafile(envname, filename );

	free(envname);
	free(filename);
	if ( retcode == 0 ) 
	    cs2fs ( Result, "", n_Result ) ; 
	else
	    {
	    cs2fs(Result, retcode, n_Result);
	    free ( retcode ) ; 
	    }
}

void datapath_ ( Result, f_envname, f_dirname, f_filename, f_suffix, 
		n_Result, n_envname, n_dirname, n_filename, n_suffix )
char *Result ;
char *f_envname ;
char *f_dirname ; 
char *f_filename ;
char *f_suffix ;
int n_Result;
int n_envname ;
int n_dirname ; 
int n_filename ;
int n_suffix ;
{

	char *envname, *dirname, *filename, *suffix ;
	char *retcode;

	envname = newcs(f_envname, n_envname);
	dirname = newcs(f_dirname, n_dirname);
	filename = newcs(f_filename, n_filename);
	suffix = newcs(f_suffix, n_suffix);

	retcode = datapath(envname, dirname, filename, suffix );

	free(envname);
	free(dirname);
	free(filename);
	free(suffix);
	if ( retcode == 0 ) 
	    cs2fs ( Result, "", n_Result ) ; 
	else
	    {
	    cs2fs(Result, retcode, n_Result);
	    free ( retcode ) ; 
	    }
}


int newdata_ ( Result, f_envname, f_dirname, f_filename, f_suffix, 
		n_Result, n_envname, n_dirname, n_filename, n_suffix )
char *Result ;
char *f_envname ;
char *f_dirname ; 
char *f_filename ;
char *f_suffix ;
int n_Result;
int n_envname ;
int n_dirname ; 
int n_filename ;
int n_suffix ;
{

	char *envname, *dirname, *filename, *suffix ;
	int retcode;
	char path[FILENAME_MAX] ;

	envname = newcs(f_envname, n_envname);
	dirname = newcs(f_dirname, n_dirname);
	filename = newcs(f_filename, n_filename);
	suffix = newcs(f_suffix, n_suffix);

	retcode = newdata(path, envname, dirname, filename, suffix );

	free(envname);
	free(dirname);
	free(filename);
	free(suffix);
	cs2fs(Result, path, n_Result);
	return retcode ;
}

/* $Id: datafile_.c,v 1.1.1.1 1997/04/12 04:18:59 danq Exp $ */
