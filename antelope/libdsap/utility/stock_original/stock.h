/* stock set of macros and declarations for DSAP programs danq 6/5/92 */
/* $Name $Revision: 1.8 $ $Date: 1998/02/16 21:26:16 $ */

#ifndef _stock_
#define _stock_

#include "config.h"

#ifndef __STDC__
/* Try to allow compiling on the old machines also. */
# include <sys/param.h>
# ifndef FILENAME_MAX
# define FILENAME_MAX MAXPATHLEN
# endif

/* in stdlib in SVR4 */
extern char *optarg;
extern int optind, opterr;

#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef FILENAME_MAX
#define FILENAME_MAX 1024
#endif

#if FILENAME_MAX < 1024
#define FILENAME_MAX 1024
#endif


#ifdef __STDC__
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif

#ifndef HAVE_MEMMOVE
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
#endif
 
#include "elog.h"
#include "arrays.h"
#include "pf.h"

typedef Tbl     Bitvector;
#define BITS_PER_INT   	(8*sizeof(int))

#define bitnew() inittbl(0,1,0,0,sizeof(char *))
#define bitfree(B) freetbl(B,0)

#define HOOK_MAGIC 814615491

typedef struct Hook {
    int magic ; 
    void (*free)() ;
    void *p ;
} Hook ; 

typedef struct Xlat {
        char *name ;
        int num ;
} Xlat ;


#ifdef __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */

#ifdef  __cplusplus
extern "C" {
#endif

extern Hook *new_hook PL_(( void (*hookfree)() ));
extern void free_hook PL_(( Hook **hookp ));

extern char **dsap_glob PL_(( char *v ));
extern char *ask PL_(( char *fmt, ... ));
extern char * getdsap ( void );
extern int newdata PL_(( char *returnpath, char *envname, char *dirname, char *filename, char *suffix ));
extern char *datafile PL_(( char *envname, char *filename ));
extern char *datapath PL_(( char *envname, char *dirname, char *filename, char *suffix ));
extern char *newcs PL_(( char *f_s, int n_s )) ;
extern char * expand_env ( char *s );

extern Tbl *split PL_(( char *s, int c ));
/* This now seems to be in stdio.h 
extern FILE *zopen PL_(( char *filename, char *permissions ));
*/
extern FILE *gz_open PL_(( char *filename, char *permissions ));
extern FILE *gz_cat PL_(( char *filename, char *permissions ));
extern int abspath PL_(( char *relp, char *absp ));
extern int askyn PL_(( char *fmt, ... ));
extern int blank PL_(( char *s ));
extern void compress PL_(( char *path ));
extern void dirbase PL_(( char *path, char *dir, char *base ));
extern int fixup_env PL_(( char *envsetup[], int nenv, int force ));
extern int envfile ( char *filename );
extern int gethdir PL_(( char *user, char *home ));
extern void isort PL_(( char *pbase, int total_elems, int size, int (*cmp)(), void *private ));
extern int makedir PL_(( char *dir ));
extern int mappath PL_(( char *spath, char *upath ));
extern void quicksort PL_(( char *pbase, size_t total_elems, size_t size, int (*cmp)(), void *private ));
/* modified to match 64bit antelope Jan 2013*/
extern void shellsort PL_(( char *a, long n, int size, int (*compare)(), void *private ));
extern int whitespace PL_(( char *s ));
extern int blank PL_(( char *s ));
extern int whitespace PL_(( char *s ));
extern Tbl *split PL_(( char *s, int c ));
/* Modified to match 64bit antelope jan 2013*/
extern void sncopy PL_(( char *dest, char *source, long n ));
extern void szcopy PL_(( char *dest, char *source, long n ));
extern void copystrip PL_(( char *dest, char *source, long n ));
extern void *memdup PL_(( void *a, long n ));

/* Modified to match 64bit antelope jan 2013*/
extern int bitset PL_(( Bitvector *bvec, long index ));
extern int bitclr PL_(( Bitvector *bvec, long index ));
extern int bittst PL_(( Bitvector *bvec, long index ));
extern long bitmax PL_(( Bitvector *b ));
extern Bitvector * bitand PL_(( Bitvector *b1, Bitvector *b2 ));
extern Bitvector * bitor PL_(( Bitvector *b1, Bitvector *b2 ));
extern Bitvector * bitnot PL_(( Bitvector *b ));
extern Bitvector * bitxor PL_(( Bitvector *b1, Bitvector *b2 ));

extern int fdkey PL_(( int fd ));
extern int fdwait PL_(( int fd, int msec ));

extern void hexdump PL_(( FILE *file, void *memory, int l ));
extern void asciidump PL_(( FILE *file, char *memory, int l ));
extern int hex2int PL_(( char c ));
extern void read_asciidump PL_(( FILE *file, char *memory, int l ));
extern void read_hexdump PL_(( FILE *file, char *memory, int l ));

extern void banner PL_(( char *program, char *version ));

extern double htond PL_(( double val ));
extern double ntohd PL_(( double val ));
extern void htondp PL_(( double * valp_from, double * valp_to ));
extern void ntohdp PL_(( double * valp_from, double * valp_to ));
extern float htonf PL_(( float val ));
extern float ntohf PL_(( float val ));
extern void htonfp PL_(( float * valp_from, float * valp_to ));
extern void ntohfp PL_(( float * valp_from, float * valp_to ));
extern short int revshort PL_(( short int val ));
extern int revint PL_(( int val ));
extern int rev4 PL_(( unsigned char *from, unsigned char *to, int n ));
extern int rev8 PL_(( unsigned char *from, unsigned char *to, int n ));
extern int isnetorder PL_(( void ));

extern int gencompress PL_(( unsigned char **out, int *nout, int *size, int *in, int nsamp, int length ));
extern int genuncompress PL_(( int **out, int *nout, int *size, unsigned char *in, int nbytes ));

extern char * xlatnum PL_(( int def, Xlat *xlat, int nxlat ));
extern int xlatname PL_(( char *name, Xlat *xlat, int nxlat ));

extern int runcmd ( char *argv[], char **result );
extern int verbose_exists (void) ;

extern int set_random ( int intcount );
extern void fill_random ( char *packet, int nchar, int *pp );
extern int check_random ( char *packet, int nchar );

extern void ignoreSIGPIPE ( void );

#ifdef  __cplusplus
}
#endif

#undef PL_


/* When a macro generates multiple output statements, there is a danger
 * that these statements might lead to odd results in a context where a
 * single statement is expected, eg. after an if -- STMT forces the macro
 * output to be a single statement, and the following stuff with ZERO is
 * to make lint work properly. */

#ifdef lint
extern int      ZERO;
#else
#define ZERO 0
#endif

#define STMT(stuff) do { stuff } while (ZERO)

/* The following macros simplify memory allocation and testing
 *
 * allot and reallot are convenient interfaces to malloc and realloc which
 * perform testing on the result without cluttering the code with
 * branches and messages which are seldom if ever used.
 *
 * ALLOTERROR specifies what happens when a malloc or realloc fails -- it
 * may be overridden with a special macro within the file */

/* All these allot defines required a 64bit mod jan 2013*/
#ifdef __STDC__
#define ALLOTDIE(ptr,size)      die(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for " #ptr "\n", __func__, __LINE__, __FILE__, (long)size)
#else
#define ALLOTDIE(ptr,size)      die(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for ptr\n", __func__, __LINE__, __FILE__, (long)size)
#endif

#ifndef BADALLOT
#define BADALLOT -1
#endif

#ifdef lint
extern int ReTuRn(int) ;
#define BADRETURN ReTuRn(BADALLOT)
#else
#define BADRETURN return BADALLOT
#endif

#ifdef __STDC__
#define ALLOTREGISTER_ERROR(ptr,size) \
        STMT(elog_log(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for " #ptr "\n", __func__, __LINE__, __FILE__, (long)size); BADRETURN ;)
#else
#define ALLOTREGISTER_ERROR(ptr,size) \
        STMT(elog_log(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for ptr\n", __func__, __LINE__, __FILE__, (long)size); BADRETURN ;)
#endif

#ifdef __STDC__
#define ALLOTCOMPLAIN(ptr,size) \
        STMT({elog_complain(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for " #ptr "\n", __func__, __LINE__, __FILE__, (long)size); BADRETURN ;})
#else
#define ALLOTCOMPLAIN(ptr,size) \
        STMT({elog_complain(1,"Out of memory in %s at line %d of '%s'\n\tCan't malloc %lu bytes for ptr\n", __func__, __LINE__, __FILE__, (long)size); BADRETURN ;})
#endif

#ifndef ALLOTERROR
#define ALLOTERROR ALLOTDIE
#endif



/* allotted can be used in an if statement when the action in allot is
 * inadequate */
#define allotted(type,ptr,size) \
  ( (ptr=(type) malloc((size_t)((size)*sizeof(*ptr)))) != NULL )

/* allot checks the results of malloc and generates an error message in
 * the case of failures */
#define allot(type,ptr,size)    \
  STMT( if (!allotted(type,ptr,size)) ALLOTERROR(ptr,size);)

/* reallot and reallotted correspond to allot and allotted and are
 * interfaces to realloc. */
#define reallotted(type,ptr,size) \
  ( (ptr=(type) realloc((char *)ptr,(size_t)((size)*sizeof(*ptr)))) != NULL )
#define reallot(type,ptr,size)    \
  STMT( if (!reallotted(type,ptr,size)) ALLOTERROR(ptr,size);)


/* insist provides a concise method for consistency checks which are
 * never expect to fail, avoiding cluttering the code with branches which
 * are seldom if ever taken.
 * 
 * INSISTFAIL specifies what happens when an insist fails -- it may be
 * overridden with a special macro within a file */

#ifndef INSISTFAIL
#define INSISTFAIL die
#endif

#define insist(ex)	STMT( if (!(ex)){ INSISTFAIL(1, "Unexpected failure: file %s, line %d\n", __FILE__, __LINE__ ) ;} )


/* The meaning of the following macros is fairly obvious; however, they
 * are all dangerous in the sense that their arguments will be evaluated
 * multiple times, which can lead to erroneous results. for example,
 * sqr(a++) will give an undefined result which is probably not what you
 * wanted and may differ among compilers. */

#define sqr(d) 		((d)*(d))

#ifndef SQR
#define SQR(d) 		((d)*(d))
#endif

#ifndef ABS
#define ABS(d) 		((d)< 0  ? -(d) : (d))
#endif

#ifndef MAX
#define MAX(a,b)        ((a)>(b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)        ((a)<(b) ? (a) : (b))
#endif

#ifndef SIGN
#define SIGN(d) 	(((d)>0)? 1.0 : ((d)<0) ? -1.0 : 0.0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* The following size is judged to be large enough to avoid problems with
 * overflowing string buffers, so that the various string routines which
 * do no checking can be used without much fear. */
#define STRSZ 1024


#endif


/* $Id: stock.h,v 1.8 1998/02/16 21:26:16 danq Exp $ */
