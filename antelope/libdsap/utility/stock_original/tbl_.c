
/* This set of routines is to implement the Fortran interface to the tbl
 * routines.
 */
int cs2fs ( char *dst, char *src, int nmax );
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include <string.h>
#include "stock.h"

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __STDC__
void
strtbl_(Tbl **ftbl, ...)
{
    char           *s;
    va_list         args;
    Tbl            *tbl;
    int 	    i, n ;

    va_start(args, ftbl);
#else
void
strtbl_(va_alist)
va_dcl
{
    char           *s;
    Tbl            *tbl, **ftbl;
    va_list         args;
    int 	    i, n ;

    va_start(args);
    ftbl = va_arg(args, Tbl **);

#endif

    tbl = newtbl(0);

    s = va_arg(args, char *);
    while (*s != 0 ) { 
	pushtbl(tbl, s);
	s = va_arg(args, char *) ;
    }

    n = maxtbl(tbl) ; 
    for ( i=0 ; i<n ; i++ ) 
	settbl(tbl, i, newcs(gettbl(tbl,i), va_arg(args, int )) ) ; 

    *ftbl = tbl;
}

void strfreetbl_ ( tbl ) 
Tbl **tbl ; 
{
    freetbl(*tbl, free) ; 
}

void strgettbl_ ( tbl, i, result, n_result ) 
Tbl **tbl ; 
int *i ; 
char *result ; 
int n_result ; 
{
    char *cp ; 

    cp = gettbl (*tbl, *i) ; 
    cs2fs ( result, cp, n_result ) ; 
}

int strsettbl_ ( tbl, i, string, n_string ) 
Tbl **tbl ; 
int *i ; 
char *string ; 
int n_string ; 
{
    char *s ; 
    s = newcs ( string, n_string ) ; 
    return settbl ( *tbl, *i, s ) ; 
}

void
inittbl_(ftbl, f_left_grow, f_right_grow, f_direct, null, f_entry_size)
Tbl	       **ftbl ;
int            *f_left_grow;
int            *f_right_grow;
int            *f_direct;
char           *null;
int            *f_entry_size;
{
    *ftbl = inittbl(*f_left_grow, *f_right_grow, *f_direct, null, *f_entry_size);
}

void
newtbl_(ftbl, f_size)
Tbl	       **ftbl ;
int            *f_size;
{
    *ftbl = inittbl(0, *f_size, 1, 0, sizeof (char *));
}

int 
settbl_(tbl, f_index, value)
Tbl           **tbl;
int            *f_index;
char           *value;
{
    return settbl(*tbl, *f_index, value);
}

void 
gettbl_(tbl, f_index, result)
Tbl           **tbl;
int            *f_index;
char           *result;
{
    char           *retcode;

    retcode = gettbl(*tbl, *f_index);
    if ((*tbl)->direct)
	memmove(result, retcode, (*tbl)->entry_size);
    else
	memmove(result, &retcode, (*tbl)->entry_size);
}

int 
maxtbl_(tbl)
Tbl           **tbl;
{
    return maxtbl(*tbl);
}

void 
freetbl_(tbl, free_tblval)
Tbl           **tbl;
void            (*free_tblval) ();
{
    freetbl(*tbl, free_tblval);
}

void 
clrtbl_(tbl, free_tblval)
Tbl           **tbl;
void            (*free_tblval) ();
{
    clrtbl(*tbl, free_tblval);
}

int 
applytbl_(tbl, function, private)
Tbl           **tbl;
int             (*function) ();
void           *private;
{
    return applytbl(*tbl, function, private);
}

int 
pushtbl_(tbl, value)
Tbl           **tbl;
char           *value;
{
    return pushtbl(*tbl, value);
}

void 
poptbl_(tbl, Result)
Tbl           **tbl;
char           *Result;
{
    char           *retcode;

    retcode = poptbl(*tbl);
    if ((*tbl)->direct)
	memmove(Result, retcode, (*tbl)->entry_size);
    else
	memmove(Result, &retcode, (*tbl)->entry_size);
}

int 
unshifttbl_(tbl, value)
Tbl           **tbl;
char           *value;
{
    return unshifttbl(*tbl, value);
}

void 
shifttbl_(tbl, Result)
Tbl           **tbl;
char           *Result;
{
    char           *retcode;

    retcode = shifttbl(*tbl);
    if ((*tbl)->direct)
	memmove(Result, retcode, (*tbl)->entry_size);
    else
	memmove(Result, &retcode, (*tbl)->entry_size);
}

int 
instbl_(tbl, f_index, value)
Tbl           **tbl;
int            *f_index;
char           *value;
{
    return instbl(*tbl, *f_index, value);
}

void 
deltbl_(tbl, f_index, Result)
Tbl           **tbl;
int            *f_index;
char           *Result;
{
    char           *retcode;

    retcode = deltbl(*tbl, *f_index);
    if ((*tbl)->direct)
	memmove(Result, retcode, (*tbl)->entry_size);
    else
	memmove(Result, &retcode, (*tbl)->entry_size);
}

int 
cattbl_(tbldest, tblsrc)
Tbl           **tbldest;
Tbl           **tblsrc;
{
    return cattbl(*tbldest, *tblsrc);
}

int 
savetbl_(tbl, f_filename, f_s, n_filename, n_s)
Tbl           **tbl;
char           *f_filename;
char           *f_s;
int             n_filename;
int             n_s;
{
    int             retcode;
    char           *filename,
                   *s;

    filename = newcs(f_filename, n_filename);
    s = newcs(f_s, n_s);
    retcode = savetbl(*tbl, filename, s);
    free(filename);
    free(s);
    return retcode;
}

void
readtbl_(ftbl, f_filename, f_s, f_rw, n_filename, n_s)
Tbl	       **ftbl ;
char           *f_filename;
char           *f_s;
int            *f_rw;
int             n_filename;
int             n_s;
{
    char           *filename,
                   *s;

    filename = newcs(f_filename, n_filename);
    *ftbl = readtbl(filename, &s, *f_rw);
    free(filename);
    strncpy ( f_s, s, n_s ) ; 
}

int 
cmptbl_(tbl1, tbl2)
Tbl           **tbl1;
Tbl           **tbl2;
{
    return cmptbl(*tbl1, *tbl2);
}

int 
searchtbl_(key, tbl, compare, private, ns, ne)
char           **key;
Tbl           **tbl;
int             (*compare) ();
void           *private;
int            *ns;
int            *ne;
{
    return searchtbl(key, *tbl, compare, private, ns, ne);
}

void 
sorttbl_(tbl, compare, private)
Tbl           **tbl;
int             (*compare) ();
void           *private;
{
    sorttbl(*tbl, compare, private);
}

void
duptbl_(ftbl, old, dup_tblval)
Tbl	      **ftbl;
Tbl           **old;
void           *(*dup_tblval) ();
{
    *ftbl = duptbl(*old, dup_tblval);
}

/* $Id: tbl_.c,v 1.1.1.1 1997/04/12 04:19:03 danq Exp $ */
