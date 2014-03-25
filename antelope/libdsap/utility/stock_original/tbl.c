/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */


/* This set of routines is to implement lists, and a set of
 * standard operations on lists.
 */

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

static int align ( i ) 
int i ;
{
    int index ; 
    index =  i & 0x7l ;
    if ( index ) 
	i += 8 - index ;
    return i ; 
}

static void extend ( tbl, n ) 
Tbl *tbl ; 
int n ; 
{ /* must extend length of the file and remap it */
    int offset2base, offset2null ;
    char *data ;
    int fd, prot ; 
    int mapped_size ;
    int entry_size = tbl->entry_size ;

    if ( (tbl->prot & PROT_WRITE) == 0 )
	die ( 0, "Can't extend read-only list.\n" ) ;

    offset2base = tbl->base.cp - tbl->mapped ;
    if ( tbl->null )
	offset2null = tbl->null - tbl->mapped ;
    else
	offset2null = 0 ;
    prot = tbl->prot ; 
    mapped_size = tbl->mapped_size ;
    fd = tbl->fd ; 

    lseek ( tbl->fd, n * entry_size-1, SEEK_END ) ;
    write ( tbl->fd, tbl->mapped, 1 ) ;

    munmap ( tbl->mapped, mapped_size ) ;     
    mapped_size += n * entry_size ;     
    data = mmap ( 0, mapped_size, prot,            
			MAP_SHARED, fd, 0 ) ;
    if ( data == (char *) -1 )
	die ( 1, "Couldn't remap tbl.\n" ) ;

    tbl->mapped = data ;
    tbl->mapped_size = mapped_size ;
    tbl->base.cp = tbl->mapped + offset2base ;
    if ( offset2null )
	tbl->null = tbl->mapped + offset2null ;
}


/* tbl's grow from left only one at a time. */
void left_growtbl ( tbl )
Tbl *tbl ; 
{
    int entry_size, nright ;
    char *prev ;

    entry_size = tbl->entry_size ;

    nright = tbl->right - tbl->active_right ;
    if ( tbl->base.cp == 0 )
        { /* never allocated space */
        allot ( char *, tbl->base.cp,
            tbl->left_grow*entry_size ) ;
	tbl->base.cp += tbl->left_grow * entry_size ;
        tbl->left -= tbl->left_grow ;
        tbl->left_grow *= 2 ;
        }
    else if ( nright > tbl->left_grow/2 ) 
	{ /* enough free space at right to use that */
	prev = tbl->base.cp ; 
	tbl->base.cp += nright *entry_size ;
	memmove ( tbl->base.cp, prev, tbl->active_right * entry_size ) ; 
	tbl->right      = tbl->active_right ; 
	tbl->left       = -nright ; 
	}
    else
	{
	if ( tbl->mapped )
	    { /* must extend the length of the file and remap it */
	    extend ( tbl, tbl->left_grow ) ; 
	    }
	else
	    {
	    reallot ( char *, tbl->base.cp,
		(tbl->right + tbl->left_grow) * entry_size) ;
	    }
	if ( tbl->right != 0 ) 
	    {
	    memmove( 
		tbl->base.cp + tbl->left_grow * entry_size,
		tbl->base.cp,
		tbl->right * entry_size ) ;
	    }
	tbl->left -= tbl->left_grow ; 
	tbl->base.cp -= tbl->left*entry_size ;
	tbl->left_grow *= 2 ; 
	}
}

void right_growtbl ( tbl, incr )
Tbl *tbl ; 
int	incr ;
{
    int i, entry_size, nleft ;
    char *prev, *cp ;

    entry_size = tbl->entry_size ;

    tbl->right_grow = MAX ( incr, tbl->right_grow ) ; 

    nleft = - tbl->left ;
    if ( tbl->base.cp == 0 ) 
	{ /* never allocated space */
	allot ( char *, tbl->base.cp, 
	    tbl->right_grow * entry_size ) ;
	tbl->right += tbl->right_grow ; 
	tbl->right_grow *= 2 ; 
	}
    else if ( nleft > tbl->right_grow/2 ) 
	{ /* enough free space at left to use that */
	prev = tbl->base.cp ; 
	tbl->base.cp -= nleft * entry_size ; 
	memmove ( tbl->base.cp, prev, tbl->active_right * entry_size ) ; 
	tbl->right     += nleft ; 
	tbl->left  = 0 ; 
	}
    else
	{ /* must allocate more space */
	if ( tbl->mapped ) 
	    { /* must extend length of the file and remap it */
	    extend ( tbl, tbl->right_grow ) ; 
	    }
	else
	    {
	    prev = tbl->base.cp + tbl->left*entry_size ; 
	    reallot ( char *, prev, 
		(tbl->right-tbl->left+tbl->right_grow)*entry_size) ;
	    tbl->base.cp = prev - tbl->left*entry_size ; 
	    }
	tbl->right += tbl->right_grow ;
	tbl->right_grow *= 2 ; 
	}


    cp = tbl->base.cp + tbl->active_right*entry_size ; 
    if ( tbl->null == 0 ) 
	memset ( cp, 0, (tbl->right-tbl->active_right)*entry_size ) ; 
    else
	for ( i=tbl->active_right ; i<tbl->right ; i++, cp+= entry_size ) 
	    memcpy ( cp, tbl->null, entry_size ) ; 
}

Tbl *inittbl ( left_grow, right_grow, direct, null, entry_size ) 
int left_grow, right_grow, direct, entry_size ; 
void *null ; 
{
    Tbl *tbl ; 
    char *null_value=0 ; 
    allot ( Tbl *, tbl, 1 ) ; 

    if ( left_grow <= 0 ) 
	tbl->left_grow = TBL_MIN_INCREMENT ; 
    else
	tbl->left_grow = left_grow ; 

    if ( right_grow <= 0 ) 
	tbl->right_grow = TBL_MIN_INCREMENT ; 
    else
	tbl->right_grow = right_grow ; 

    tbl->direct = direct ; 
    tbl->entry_size = entry_size ; 
    if ( null != 0 ) 
	{
	allot ( char *, null_value, entry_size ) ; 
	memcpy ( null_value, null, entry_size ) ; 
	}
    tbl->null = null_value ; 
    tbl->base.cp = 0 ; 
    tbl->left = 0 ; 
    tbl->active_right = 0 ; 
    tbl->right = 0 ; 

    tbl->mapped = 0 ; 
    tbl->mapped_size = 0 ; 
    tbl->fd = 0 ; 
    tbl->prot = 0 ; 
    tbl->deleted = 0 ; 

    return tbl ; 
}

Tbl *newtbl (size)
int size ; 
{
    return inittbl ( 0, size, 0, (char *) 0, sizeof ( char *) ) ; 
}

int settbl ( tbl, index, value ) 
Tbl *tbl ; 
int index ; 
void *value ; 
{ 
    if ( index<0 ) 
	index = tbl->active_right ; 
    if ( index >= tbl->right ) 
	right_growtbl ( tbl, index-tbl->right+1 ) ;
    if ( tbl->direct ) 
	memcpy ( tbl->base.cp+index*tbl->entry_size, value, tbl->entry_size ) ; 
    else
	(tbl->base.pp)[index] = value ; 
    tbl->active_right = MAX ( tbl->active_right, index+1 ) ; 
    return index ; 
}

void *addrtbl(tbl, index)
Tbl *tbl ; 
int index ;
{
    if ( index >= tbl->active_right 
	|| index < 0) 
	return 0 ; 
    if ( tbl->direct ) 
	return tbl->base.cp+index*tbl->entry_size ; 
    else
	return tbl->base.pp + index ; 
}

void *gettbl ( tbl, index ) 
Tbl *tbl ; 
int index ; 
{
    if ( index >= tbl->active_right 
	|| index < 0) 
	return tbl->null ; 
    if ( tbl->direct ) 
	return tbl->base.cp+index*tbl->entry_size ; 
    else
	return tbl->base.pp[index] ; 
}

static int
not_all_zero ( s, size )
char *s ; 
int size ; 
{
    int i ; 
    for ( i=0 ; i<size ; i++ ) 
	if ( s[i] != 0 ) 
	    return 1 ; 
    return 0 ;
}

void freetbl ( tbl, free_tblval )
Tbl *tbl ; 
void (*free_tblval) () ; 
{
    int i, n ; 
    char *val ; 

    if ( tbl->mapped ) {
	char *data=tbl->mapped ; 
	int fd = tbl->fd, 
	    size = tbl->mapped_size ;
	if ( tbl->deleted != 0 ) 
	    free ( tbl->deleted ) ; 
	if ( (tbl->prot & PROT_WRITE) == 0 ) 
	    free ( tbl ) ; 
	close ( fd ) ; 
	munmap ( data, size ) ; 
    } else {
	if ( free_tblval ) {
	    n = maxtbl ( tbl ) ; 
	    for ( i = 0 ; i<n ; i++ ) {
		val = gettbl ( tbl, i ) ; 
		if ( ! tbl->direct ) {
		    if ( val != tbl->null )
			(*free_tblval) (val);
		} else if (( tbl->null != 0 && (memcmp(tbl->null, val, tbl->entry_size) !=0) )
			|| (tbl->null == 0 && not_all_zero(val, tbl->entry_size))) {
			(*free_tblval) (val);
		}
	    }
	}
	if ( tbl->base.cp != 0 ) 
	    free ( tbl->base.cp + tbl->left*tbl->entry_size ) ;
	if ( tbl->null != 0 ) 
	    free ( tbl->null ) ; 
	if ( tbl->deleted != 0 ) 
	    free ( tbl->deleted ) ; 
	free (tbl) ;
    }
}

void trunctbl ( tbl, last, free_tblval ) 
Tbl *tbl ; 
int last ;
void (*free_tblval) () ; 
{ 
    int i, n ; 
    char *val ; 
    n = maxtbl ( tbl ) ; 

    last = MAX(0,last) ;
    if ( (free_tblval != 0) ) {
	for ( i = last ; i<n ; i++ ) {   
	    val = gettbl ( tbl, i ) ;
	    if ( ! tbl->direct ) {
		if ( val != tbl->null )
		    (*free_tblval) (val);
	    } else if (( tbl->null != 0 && (memcmp(tbl->null, val, tbl->entry_size) !=0) )
		    || (tbl->null == 0 && not_all_zero(val, tbl->entry_size))) {
		    (*free_tblval) (val);
		    if ( tbl->null != 0)
			memcpy(val, tbl->null, tbl->entry_size) ; 
		    else 
			memset(val, 0, tbl->entry_size) ; 
	    }
	}
    }
    tbl->active_right = MIN(last,n) ;
}

void clrtbl ( tbl, free_tblval ) 
Tbl *tbl ; 
void (*free_tblval) () ; 
{ 
    int i, n ; 
    char *val ; 
    if ( free_tblval != 0 ) {
	n = maxtbl ( tbl ) ; 
	for ( i = 0 ; i<n ; i++ ) {   
	    val = gettbl ( tbl, i ) ;
	    if ( ! tbl->direct ) {
		if ( val != tbl->null )
		    (*free_tblval) (val);
	    } else if (( tbl->null != 0 && (memcmp(tbl->null, val, tbl->entry_size) !=0) )
		    || (tbl->null == 0 && not_all_zero(val, tbl->entry_size))) {
		    (*free_tblval) (val);
		    if ( tbl->null != 0)
			memcpy(val, tbl->null, tbl->entry_size) ; 
		    else 
			memset(val, 0, tbl->entry_size) ; 
	    }
	}
    }
    tbl->active_right = 0 ;
}

int applytbl ( tbl, function, private )
Tbl *tbl ; 
int (*function) () ; 
void *private ; 
{
    int i, n, result=0 ; 
    char *val ; 
    n = maxtbl ( tbl ) ; 
    for ( i = 0 ; i<n ; i++ ) 
	{
	val = gettbl ( tbl, i ) ; 
	if ( val != 0 ) 
	    result += (*function) (val, private);
	}
    return result ; 
}

int pushtbl ( tbl, value ) 
Tbl *tbl ; 
void *value ; 
{
    if ( tbl->active_right >= tbl->right )
        right_growtbl ( tbl, 1 ) ;
    if ( tbl->direct )
        memcpy ( tbl->base.cp+tbl->active_right*tbl->entry_size, value, tbl->entry_size ) ;
    else
        (tbl->base.pp)[tbl->active_right] = value ;
    return tbl->active_right++ ;
}

void *poptbl ( tbl ) 
Tbl *tbl ;
{
    if ( tbl->active_right <= 0 ) 
        return tbl->null ;
    if ( tbl->direct )
        return tbl->base.cp + --(tbl->active_right)*tbl->entry_size ;
    else
        return tbl->base.pp[--(tbl->active_right)] ;
}
    
 
int unshifttbl ( tbl, value ) 
Tbl *tbl ; 
void *value ; 
{
    if ( tbl->left >= 0 )
        left_growtbl ( tbl ) ;
    
    tbl->active_right++ ; 
    tbl->right++ ; 
    tbl->left++ ; 
    tbl->base.cp -= tbl->entry_size ; 

    if ( tbl->direct )
        memcpy ( tbl->base.cp, value, tbl->entry_size ) ;
    else
        (tbl->base.pp)[0] = value ;
    return 0 ;
}
 

void *shifttbl ( tbl ) 
Tbl *tbl ;
{
    char *retval ;
    if ( tbl->active_right <= 0 ) 
        return tbl->null ;

    if ( tbl->direct )
        retval =  tbl->base.cp; 
    else
        retval =  tbl->base.pp[0] ;
    
    tbl->active_right-- ; 
    tbl->right-- ; 
    tbl->left-- ; 
    tbl->base.cp += tbl->entry_size ; 

    return retval ;
}

int instbl ( tbl, index, value ) 
Tbl *tbl ; 
int index ; 
void *value ; 
{ 
    int entry_size = tbl->entry_size ; 
    char *cp ;

    if ( index<0 )
	return unshifttbl ( tbl, value ) ; 
    if ( index >= tbl->active_right )
	return pushtbl ( tbl, value ) ;
    if ( tbl->active_right >= tbl->right ) 
	right_growtbl ( tbl, 1 ) ;
    cp = tbl->base.cp + index * entry_size ;
    if ( tbl->active_right - index > 0 ) 
	memmove ( cp+entry_size, cp, (tbl->active_right - index)*entry_size ) ; 
    if ( tbl->direct ) 
	memcpy ( cp, value, entry_size ) ; 
    else
	(tbl->base.pp)[index] = value ; 
    tbl->active_right++ ;
    return index ; 
}

void *deltbl ( tbl, index ) 
Tbl *tbl ; 
int index ; 
{
    int entry_size = tbl->entry_size ;
    char *cp ;
    char *retval ;

    if ( index >= tbl->active_right 
	|| index < 0) 
	return tbl->null ; 
    if ( tbl->direct ) 
	{
	if (tbl->deleted == 0 ) 
	    allot ( char *, tbl->deleted, entry_size ) ; 
	memmove ( tbl->deleted, tbl->base.cp + index *entry_size, entry_size ) ; 
	retval = tbl->deleted ; 
	}
    else
	retval = tbl->base.pp[index] ; 

    cp = tbl->base.cp + index * entry_size ;
    memmove ( cp, cp+entry_size, (tbl->active_right-index-1)*entry_size ) ; 
    tbl->active_right-- ; 
    if ( tbl->null == 0 ) 
	memset ( tbl->base.cp + tbl->active_right*entry_size, 0, entry_size ) ; 
    else
	memmove ( tbl->base.cp + tbl->active_right*entry_size, tbl->null, entry_size ) ; 
    
    return retval ;
}

int cattbl ( tbldest, tblsrc ) 
Tbl *tbldest, *tblsrc ; 
{
    int sizesrc, sizedest ; 
    
    if ( tbldest->entry_size != tblsrc->entry_size )
	{
	register_error ( 0, "tbl lists have different entry sizes: %d %d\n", 
	    tbldest->entry_size, tblsrc->entry_size ) ;
	return 1;
	}

    if ( tbldest->direct != tblsrc->direct ) 
	{
	register_error ( 0, "tbl lists are not both direct or indirect.\n" ) ;
	return 1;
	}
	
    if ( tbldest->null != tblsrc->null 
	&& (   tbldest->null == 0 
	    || tblsrc->null == 0
	    || memcmp ( tbldest->null, tblsrc->null, tbldest->entry_size ) != 0 ) )
	{
	register_error ( 0, "tbl lists have different null values.\n" ) ; 
	return 1;
	}

    tbldest->left_grow = MAX ( tbldest->left_grow, tblsrc->left_grow ) ; 
    tbldest->right_grow = MAX ( tbldest->right_grow, tblsrc->right_grow ) ; 
    if ( tbldest->right - tbldest->active_right < tblsrc->active_right ) 
	right_growtbl ( tbldest, tblsrc->active_right ) ; 

    sizedest = tbldest->active_right * tbldest->entry_size ; 
    sizesrc = tblsrc->active_right * tblsrc->entry_size ; 
    memmove ( tbldest->base.cp+sizedest, tblsrc->base.cp, sizesrc ) ; 
    tbldest->active_right += tblsrc->active_right ; 

    return 0 ; 
}

/*
static int pad ( file, n ) 
FILE *file ; 
int n ; 
{
    if ( n > 0 ) return fwrite ( zeros, n, 1, file ) != 1 ; 
    return 0 ; 
}
*/

static int Zeros[2] = { 0, 0 } ;
#define pad(F,N) (N>0 && (fwrite (Zeros, N, 1, F) != 1))

int writetbl ( tbl, file, s ) 
Tbl *tbl ; 
FILE *file ; 
char *s ; 
{
    int len, n ; 
    char *data ;

    if ( fwrite ( "#tbl", 4, 1, file ) != 1)
	return -1 ; 

    if ( s != 0 ) 
	{
	len = strlen(s) + 1; 
	n = align(len) ;
	}
    else
	n = len = 0 ; 
    if ( fwrite ( &n, sizeof(n), 1, file) != 1)
	return -1 ; 
    if ( s != 0 
	&& ( fwrite ( s, len, 1, file) != 1 
	   || pad ( file, n-len)) )
	return -1 ;

    if ( fwrite ( tbl, sizeof(*tbl), 1, file ) != 1)
	return -1 ; 
    if ( tbl->null ) 
	{
	n = align ( tbl->entry_size ) - tbl->entry_size ; 
	if ( fwrite ( tbl->null, tbl->entry_size, 1, file) != 1)
	    return -1 ; 
	if ( pad ( file, n ) )
	    return -1 ; 
	}
    data = tbl->base.cp + tbl->left * tbl->entry_size ; 
    if ( fwrite ( data, tbl->entry_size, 
			tbl->right - tbl->left,
			file ) 
			!= tbl->right - tbl->left )
			    return -1 ; 
    return 0 ;
}


int savetbl ( tbl, filename, s ) 
Tbl *tbl ; 
char *filename ; 
char *s ; 
{
    FILE *file ;
    int result = 0 ; 
    if ( (file = fopen ( filename, "w+" ) ) == 0 ) 
	{
	register_error ( 1, "Can't open %s to write tbl.\n", filename ) ; 
	return -1 ; 
	}

    result = writetbl(tbl, file, s) ;
    if (result)  
	register_error ( 1, "Errors while writing tbl to %s.\n", filename ) ; 
    fclose ( file ) ; 
    return result ;
}

Tbl *readtbl ( filename, s, rw ) 
char *filename ;
char **s ; 
int rw ;
{
    char *cp ; 
    Tbl *tbl ;
    char *data ;
    struct stat statbuf ; 
    int prot ; 
    char *perm ;
    FILE *file ; 
    int n, fd ; 

    if ( rw ) 
	{
	prot = PROT_READ | PROT_WRITE ; 
	perm = "r+" ; 
	}
    else
	{
	prot = PROT_READ ; 
	perm = "r" ; 
	}

    if ( (file = fopen ( filename, perm ) ) == 0 )
        {
        register_error ( 1, "Can't open %s to read tbl.\n", filename ) ;
        return 0;
        }   

    fd = fileno ( file ) ; 
    if ( fstat ( fd, &statbuf ) != 0 ) 
	{
	register_error ( 1, "Error trying to stat %s", filename ) ; 
	return 0 ; 
	}

    data = mmap(0, statbuf.st_size, prot, MAP_SHARED, fd, 0);
    if ( data == ( char *) -1 ) 
	{
	register_error ( 1, "Error trying to mmap %s.\n", filename ) ; 
	return 0 ; 
	}

    if ( memcmp ( data, "#tbl", 4 ) != 0 ) 
	{
	register_error ( 0, "Bad magic number while trying to read tbl.\n" ) ;  
	munmap ( data, statbuf.st_size ) ; 
	return 0 ; 
	}

    cp = data + 4 ;
    memcpy ( &n, cp, sizeof(int) ) ; 
    cp += 4 ;
    if ( n > 0 ) 
	{
	*s = cp ; 
	cp += n ;
	}
    else 
	*s = 0 ; 

    if ( rw ) 
	tbl = (Tbl *)cp ;
    else 
	{
	tbl = newtbl(0) ;
	memcpy ( tbl, cp, sizeof(*tbl) ) ; 
	}
    cp += sizeof(*tbl) ;

    tbl->deleted = 0 ; 
    if ( tbl->null ) 
	{
	tbl->null = cp; 
	cp += align (tbl->entry_size) ;
	}

    tbl->mapped = data ; 
    tbl->mapped_size = statbuf.st_size ; 
    tbl->fd = fd ; 
    tbl->prot = prot ; 
    tbl->base.cp = cp - tbl->left *tbl->entry_size ; 
    return tbl ; 
}

int cmptbl ( tbl1, tbl2 ) 
Tbl *tbl1, *tbl2 ; 
{ 
    if ( tbl1->active_right != tbl2->active_right 
	|| tbl1->entry_size != tbl2->entry_size
	) 
	return 1 ; 
    return memcmp ( tbl1->base.cp, tbl2->base.cp, tbl1->active_right * tbl1->entry_size ) ; 
}

int searchtbl ( key, tbl, compare, private, ns, ne ) 
char **key ; 
Tbl *tbl ; 
int (*compare) () ; 
void *private ; 
int *ns, *ne ; 
{
    return ssearch ( key, tbl->entry_size, tbl->base.cp, maxtbl(tbl), 0, 
			    compare, private, ns, ne ) ; 
}

void sorttbl ( tbl, compare, private ) 
Tbl *tbl ; 
int (*compare) () ; 
void *private ; 
{
    shellsort ( tbl->base.cp, maxtbl(tbl), tbl->entry_size, compare, private ) ; 
}

Tbl *duptbl ( old, dup_tblval ) 
Tbl *old ; 
void * (*dup_tblval)() ; 
{
    int i, n ;
    Tbl *new ;
    char *val ;

    n = maxtbl ( old ) ; 
    new = inittbl ( old->left_grow, old->right_grow, old->direct, 
	    old->null, old->entry_size ) ;
    cattbl ( new, old ) ; 
    if ( dup_tblval ) { 
	if ( new->direct ) 
	    for ( i=0 ; i<n ; i++ ) {
		val = gettbl(new, i) ; 
		if ( memcmp(val, new->null, new->entry_size) )
		    dup_tblval(val) ; 
	    }
	else 
	    for ( i=0 ; i<n ; i++ ) {
		val = gettbl(new, i) ; 
		if ( val != new->null )
		    settbl(new, i, dup_tblval(val)) ; 
	    }
    }
    return new ; 
}

#ifdef __STDC__
Tbl *strtbl ( char *s, ... ) 
{
    va_list 	args ; 
    Tbl *tbl ; 
    va_start (args, s ) ; 
#else
Tbl *strtbl(va_alist)
va_dcl
{
    char *s ; 
    Tbl *tbl ; 
    va_list	args ; 
    va_start (args) ; 

    s = va_arg(args, char *) ; 
#endif

    tbl = newtbl ( 0 ) ; 
    pushtbl ( tbl, s ) ; 

    while ( (s = va_arg(args, char *) ) != 0 ) 
	pushtbl ( tbl, s ) ; 

    return tbl ; 
}


char           *
jointbl(tbl, separator)
Tbl            *tbl;
char           *separator;
{
    int             length, 
		    i,
                    n;
    char           *s, *out, *in ;

    length = 0 ;
    n = maxtbl(tbl);
    for (i = 0; i < n; i++)
	length += strlen(gettbl(tbl,i)) ; 
    length += n * strlen(separator) + 1 ;
    allot ( char *, s, length ) ; 
    out = s ;
    for (i = 0; i < n; i++) {
	in = gettbl(tbl,i) ;
	while ( *in ) 
	    *out++ = *in++ ; 
	in = separator ;
	while ( *in ) 
	    *out++ = *in++ ; 
    }
    *out = 0 ; 
    return s ;
}

/* $Id: tbl.c,v 1.3 1997/09/22 16:31:06 danq Exp $ */
