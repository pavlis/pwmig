
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

#ifdef  __cplusplus
extern "C" {
#endif

extern Arr *newarr ( int (*cmp)(char *, char *) );
extern void *delarr ( Arr *arr, char *key );
extern void *setarr ( Arr *arr, char *key, void *value );
extern void *getarr ( Arr *arr, char *key );
extern Tbl *keysarr ( Arr *arr );
extern Tbl *valsarr ( Arr *arr );
extern int freearr ( Arr *root, void (*free_value) (void *) );
extern int applyarr ( Arr *arr, void (*function) (char *, void *, void*), void *private );
extern long cntarr ( Arr *arr );
extern void left_growtbl ( Tbl *tbl );
extern void right_growtbl ( Tbl *tbl, long incr );
extern Tbl *inittbl ( long left_grow, long right_grow, int direct, void *null, int entry_size );
extern Tbl *newtbl ( long size );
extern int settbl ( Tbl *tbl, long index, void *value );
extern void *gettbl ( Tbl *tbl, long index );
extern void freetbl ( Tbl *tbl, void (*free_tblval) (void *) );
extern void trunctbl ( Tbl *tbl, long last, void (*free_tblval) () );
extern void clrtbl ( Tbl *tbl, void (*free_tblval) (void *) );
extern int applytbl ( Tbl *tbl, int (*function) (void *, void *), void *private );
extern long pushtbl ( Tbl *tbl, void *value );
extern void *poptbl ( Tbl *tbl );
extern long unshifttbl ( Tbl *tbl, void *value );
extern void *shifttbl ( Tbl *tbl );
extern long instbl ( Tbl *tbl, long index, void *value );
extern void *deltbl ( Tbl *tbl, long index );
extern long cattbl ( Tbl *tbldest, Tbl *tblsrc );
extern int writetbl ( Tbl *tbl, FILE *file, char *s );
extern int savetbl ( Tbl *tbl, char *filename, char *s );
extern Tbl *readtbl ( char *filename, char **s, int rw );
extern int cmptbl ( Tbl *tbl1, Tbl *tbl2 );
extern long searchtbl ( char *key, Tbl *tbl, int (*compare) (void *, void *, void *), void *private, long *ns, long *ne );
extern void sorttbl ( Tbl *tbl, int (*compare) (void *, void *, void *), void *private );
extern Tbl *duptbl ( Tbl *old, void * (*dup_tblval)(void *) );
extern char *jointbl ( Tbl *tbl, char *s) ;
extern Stbl *newstbl ( int (*cmp)(void *, void *) );
extern void *addstbl ( Stbl *stbl, void *key );
extern void *delstbl ( Stbl *stbl, void *key );
extern void *tststbl ( Stbl *stbl, void *key );
extern long maxstbl ( Stbl *stbl );
extern void *getstbl ( Stbl *stbl, long index );
extern int freestbl ( Stbl *stbl, void (*free_stblval) (void *) );
extern int applystbl ( Stbl *stbl, int (*function) (void *, void *), void *private );
extern Tbl *strtbl ( char *s, ... );
long ssearch(char *key,
                 int keysize,
                 char *index,
                 long nkeys,
                 int unique,
                 int  (*compare)(char*, char*, void*),
                 void *private,
                 long *ns, long *ne);
#ifdef  __cplusplus
}
#endif


#endif

/* $Id: arrays.h,v 1.3 1997/08/27 20:39:21 danq Exp $ */
