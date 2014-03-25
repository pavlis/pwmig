
#include "stock.h"

void clear_register_ ( flag ) 
int flag ; 
{
    clear_register ( flag ) ; 
}

void die_ ( sysflag, fs, n ) 
int *sysflag ; 
char *fs ; 
int n ;
{
    char *s = newcs ( fs, n ) ; 
    die ( *sysflag, "%s\n", s ) ; 
    free ( s ) ; 
}

void register_error_ ( sysflag, fs, n ) 
int *sysflag ; 
char *fs ; 
int n ;
{
    char *s = newcs ( fs, n ) ; 
    register_error ( *sysflag, "%s\n", s ) ; 
    free ( s ) ; 
}

void complain_ ( sysflag, fs, n ) 
int *sysflag ; 
char *fs ; 
int n ;
{
    char *s = newcs ( fs, n ) ; 
    complain ( *sysflag, "%s\n", s ) ; 
    free ( s ) ; 
}

void clear_register__ ( flag ) 
{ clear_register (flag) ; }

void die__ ( int *sysflag, char *fs, int n ) 
{ die_ ( sysflag, fs, n ) ; } 

void register_error__ ( int *sysflag, char *fs, int n ) 
{ register_error_ ( sysflag, fs, n ) ; } 

void complain__ ( int *sysflag, char *fs, int n ) 
{ complain_ ( sysflag, fs, n ) ; } 

/* $Id: error_.c,v 1.3 1998/02/16 21:26:13 danq Exp $ */
