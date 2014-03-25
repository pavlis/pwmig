
#ifndef _arrays_
#define _arrays_
#include "rb.h"

/*-
   Lists are implemented as arrays which may grow from the right or left.
   The interesting indexes are shown in the diagram below:

            base (pointer)
            |
   |________|___________________|_________________|
   left  active_left        active_right        right

   There is just one pointer, the base pointer.  The other
   indexes are integers.

   Basically, the routines manipulate active_left and
   active_right, and ensure that they remain bounded by
   left and right.

   As shown, the indexes are over-specified; base may be chosen to
   coincide with either left or active_left.  These routines choose to
   make base and active_left coincide; ie, active_left is defined to be
   zero.  This makes the grow routine more complex, but the set and get
   routines less complex.

   Each entry of the list has a particular size, which is often
   sizeof (char *).  Correspondingly, while the list is often
   an array of pointers, it may be an array of structures or
   doubles, integers, shorts, or chars.
*/

#define TBL_MIN_INCREMENT 10
/* 64bit conversion Jan 13.  long conversions to match include in antelope 5.2-64 */

typedef struct Tbl
{
    long             left,
		    active_right,
                    right;
    int             entry_size;
    int		    direct ;
    char	    *mapped ; 
    long	    mapped_size ;
    int		    fd; 
    int		    prot ;
    long	    left_grow, right_grow ;
    char           *null;
    char	   *deleted;
    union     { 
	char *cp ; 
	char **pp ; 
	} base ;
}               Tbl;



typedef struct Stbl
{
    Tbl            *tbl;
    Arr		   *arr;
    int             changed;
}               Stbl;

#define maxtbl(TBL) ((TBL)->active_right)

#ifdef __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */

extern Arr *newarr PL_(( int (*cmp)(char *, char *) ));
extern void *delarr PL_(( Arr *arr, char *key ));
extern void *setarr PL_(( Arr *arr, char *key, void *value ));
extern void *getarr PL_(( Arr *arr, char *key ));
extern Tbl *keysarr PL_(( Arr *arr ));
extern Tbl *valsarr PL_(( Arr *arr ));
extern int freearr PL_(( Arr *root, void (*free_value) (void *) ));
extern int applyarr PL_(( Arr *arr, void (*function) (char *, void *, void*), void *private ));
extern long cntarr PL_(( Arr *arr ));
extern void left_growtbl PL_(( Tbl *tbl ));
extern void right_growtbl PL_(( Tbl *tbl, int incr ));
extern Tbl *inittbl PL_(( long left_grow, long right_grow, int direct, void *null, int entry_size ));
extern Tbl *newtbl PL_(( long size ));
extern int settbl PL_(( Tbl *tbl, long index, void *value ));
extern void *gettbl PL_(( Tbl *tbl, long index ));
extern void freetbl PL_(( Tbl *tbl, void (*free_tblval) (void *) ));
extern void trunctbl PL_(( Tbl *tbl, long last, void (*free_tblval) () ));
extern void clrtbl PL_(( Tbl *tbl, void (*free_tblval) (void *) ));
extern int applytbl PL_(( Tbl *tbl, int (*function) (void *, void *), void *private ));
extern long pushtbl PL_(( Tbl *tbl, void *value ));
extern void *poptbl PL_(( Tbl *tbl ));
extern long unshifttbl PL_(( Tbl *tbl, void *value ));
extern void *shifttbl PL_(( Tbl *tbl ));
extern long instbl PL_(( Tbl *tbl, int index, void *value ));
extern void *deltbl PL_(( Tbl *tbl, int index ));
extern long cattbl PL_(( Tbl *tbldest, Tbl *tblsrc ));
extern int writetbl PL_(( Tbl *tbl, FILE *file, char *s ));
extern int savetbl PL_(( Tbl *tbl, char *filename, char *s ));
extern Tbl *readtbl PL_(( char *filename, char **s, int rw ));
extern int cmptbl PL_(( Tbl *tbl1, Tbl *tbl2 ));
extern long searchtbl PL_(( char **key, Tbl *tbl, int (*compare) (void *, void *, void *), void *private, long *ns, long *ne ));
extern void sorttbl PL_(( Tbl *tbl, int (*compare) (void *, void *, void *), void *private ));
extern Tbl *duptbl PL_(( Tbl *old, void * (*dup_tblval)(void *) ));
extern char *jointbl PL_(( Tbl *tbl, char *s)) ;
extern Stbl *newstbl PL_(( int (*cmp)(void *, void *) ));
extern void *addstbl PL_(( Stbl *stbl, void *key ));
extern void *delstbl PL_(( Stbl *stbl, void *key ));
extern void *tststbl PL_(( Stbl *stbl, void *key ));
extern long maxstbl PL_(( Stbl *stbl ));
extern void *getstbl PL_(( Stbl *stbl, long index ));
extern int freestbl PL_(( Stbl *stbl, void (*free_stblval) (void *) ));
extern int applystbl PL_(( Stbl *stbl, int (*function) (void *, void *), void *private ));
extern Tbl *strtbl PL_(( char *s, ... ));

#undef PL_

#endif

/* $Id: arrays.h,v 1.3 1997/08/27 20:39:21 danq Exp $ */
