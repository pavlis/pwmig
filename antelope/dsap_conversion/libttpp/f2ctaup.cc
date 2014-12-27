#include <unistd.h>
/* struct stat is defined in this sys file.  This include may not be necessary
but added it to document this issue that initially confused me */
#include <sys/stat.h>
#include "swapbytes_pwmig.h"

#define TRUE_ (1)
#define FALSE_ (0)
#define VOID void
#define JSRC	150
#define JSEG	30
#define JBRN	100
#define JOUT	2250
#define JTSM 	350
#define JXSM	JBRN
#define JBRNU	JBRN
#define JBRNA	JBRN
#define JREC	(JTSM+JXSM)
#define JTSM0	(JTSM+1)

#include "TauPCalculator.h"

static char    *tbl_data;
static int      tbl_data_size;
/* This set of original f2c abominations
static int      c__1 = 1;
static int      c__2 = 2;
static float    c_b3 = (float) -1.;
static double   c_b10 = 1.;
static double   m_pi_2 = m_pi_2; 

where converted to the following: */

int c__1(1);
int c__2(2);
float c_b3(-1.0);
double c_b10(1.0);
double m_pi_2(M_PI_2);
/* All of these should be const, but idioms that occur in the fortran
conversion with pass by reference make the code not copile if they 
are (correctly) declared const */
using namespace std;
using namespace PWMIG;

int i_indx (char *a, char *b, int la, int lb)

{
    int             i,
                    n;
    char           *s,
                   *t,
                   *bend;
    n = la - lb + 1;
    bend = b + lb;
    for (i = 0; i < n; ++i) {
	s = a + i;
	t = b;
	while (t < bend)
	    if (*s++ != *t++)
		goto no;
	return (i + 1);
no:	;
    }
    return (0);
}

int i_len (char *s, int n)
{
    return (n);
}
void s_cat (char *lp, char *rpp[], int rnp[], int *np, int ll)
{
    int             i,
                    nc;
    char           *rp;
    int             n = *np;
    for (i = 0; i < n; ++i) {
	nc = ll;
	if (rnp[i] < nc)
	    nc = rnp[i];
	ll -= nc;
	rp = rpp[i];
	while (--nc >= 0)
	    *lp++ = *rp++;
    }
    while (--ll >= 0)
	*lp++ = ' ';
} 
int i_sign (int *a, int *b)
{
    int             x;
    x = (*a >= 0 ? *a : -*a);
    return (*b >= 0 ? x : -x);
}
double d_sign (double *a, double *b)
{
    double          x;
    x = (*a >= 0 ? *a : -*a);
    return (*b >= 0 ? x : -x);
}
double d_mod (double *x, double *y)
{
    double          quotient;
    if ((quotient = *x / *y) >= 0)
	quotient = floor (quotient);
    else
	quotient = -floor (-quotient);
    return (*x - (*y) * quotient);
} 
double pow_ri (float *ap, int *bp)
{
    double          pow,
                    x;
    int             n;
    unsigned long   u;
    pow = 1;
    x = *ap;
    n = *bp;
    if (n != 0) {
	if (n < 0) {
	    n = -n;
	    x = 1 / x;
	}
	for (u = n;;) {
	    if (u & 01)
		pow *= x;
	    if (u >>= 1)
		x *= x;
	    else
		break;
	}
    }
    return (pow);
} 
int pow_ii (int *ap, int *bp)
{
    int             pow,
                    x,
                    n;
    unsigned long   u;
    x = *ap;
    n = *bp;
    if (n <= 0) {
	if (n == 0 || x == 1)
	    return 1;
	if (x != -1)
	    return x == 0 ? 1 / x : 0;
	n = -n;
    }
    u = n;
    for (pow = 1;;) {
	if (u & 01)
	    pow *= x;
	if (u >>= 1)
	    x *= x;
	else
	    break;
    }
    return (pow);
} 
int s_cmp (char *a0, char *b0, int la, int lb)
{
    register unsigned char *a,
                   *aend,
                   *b,
                   *bend;
    a = (unsigned char *) a0;
    b = (unsigned char *) b0;
    aend = a + la;
    bend = b + lb;
    if (la <= lb) {
	while (a < aend)
	    if (*a != *b)
		return (*a - *b);
	    else {
		++a;
		++b;
	    }
	while (b < bend)
	    if (*b != ' ')
		return (' ' - *b);
	    else
		++b;
    } else {
	while (b < bend)
	    if (*a == *b) {
		++a;
		++b;
	    } else
		return (*a - *b);
	while (a < aend)
	    if (*a != ' ')
		return (*a - ' ');
	    else
		++a;
    }
    return (0);
}

void s_copy (char *a, char *b, int la, int lb)
{
    char  *aend, *bend;
    aend = a + la;
    if (la <= lb)
	while (a < aend)
	    *a++ = *b++;
    else {
	bend = b + lb;
	while (b < bend)
	    *a++ = *b++;
	while (a < aend)
	    *a++ = ' ';
    }
}
int taupmapafile (char *filename, char **buffer, int *bufsize, int flag)
{
    int             fd;
    struct stat     statbuf;
    void *mmap_data;
    int             prot;
    if (flag & 1)
	fd = open (filename, O_RDWR);
    else
	fd = open (filename, O_RDONLY);
    if (fd < 0) {
	elog_log (1, "Couldn't open %s\n", filename);
	return -1;
    }
    if (fstat (fd, &statbuf)) {
	elog_log (1, "fstat failed for '%s'\n", filename);
	return -1;
    }
    if (flag & 1)
	prot = PROT_READ | PROT_WRITE;
    else
	prot = PROT_READ;
    *bufsize = statbuf.st_size;
    mmap_data = mmap (NULL, statbuf.st_size,
		      prot, MAP_SHARED, fd, 0);
    if (mmap_data == MAP_FAILED) {
	elog_log (1, "Can't memory map '%s'\n", filename);
	return -1;
    }
    *buffer = (char *)mmap_data;
    if (flag & 2) {
	if (lockf (fd, F_TLOCK, statbuf.st_size)) {
	    elog_log (1, "Can't lock '%s' for exclusive use\n", filename);
	    return -1;
	}
    }
    return fd;
} 
int TauPCalculator::bkin_ (int *lu, int *nrec, int *len, double *buf)
{
    if (*nrec < 0 || *nrec >= tbl_maxrec) {
	elog_die (0, "bkin called with record #=%d, MAX record =%d\n", *nrec, tbl_maxrec);
    }
    if (*nrec == 0)
	memset (buf, 0, *len * sizeof (double));
    else {
	unsigned char *p8 ;
	p8 = (unsigned char *) (tbl_data + (*nrec - 1) * tbl_record_len); 
	//md2hd(&p8, buf, *len) ;
	md2hd((double *)p8, buf, *len) ;
    }
    return 0;
}
#define READ(A) if(fread(&(A),sizeof(A),1,fin)!=1)error=1;
#ifdef WORDS_BIGENDIAN
#define SWAP4(X)
#else
/* Original for old c implementation
#define SWAP4(X)  swap4(&X,&X,1)
Simplified version for C++ */
#define SWAP4(X) swap4(&X)
#define FSWAP4(X) swap4_float(&X)
#endif

void TauPCalculator::tabin_(int *in, char *modelname)
{
    char            phdif[64];
    int             i,
                    k,
                    error = 0;
    int             i__,
                    i__2,
                    i__3,
                    j,
                    l,
                    ind,
                    nph;
    int             nl,
                    len2;
    int             reclen1,
                    reclen2;
    char            filename[1024];
    FILE           *fin;
    int             fd;
    unsigned char  *p8 ; 

    strcpy (phdif, "P       "
	    "S       "
	    "pP      "
	    "sP      "
	    "pS      "
	    "sS      ");
    strcpy (filename, modelname);
    strcat (filename, ".hed");
    if ((fin = fopen (filename, "r")) == 0)
	elog_die(1, "Can't open '%s'\n", filename);
    for (i = 0; i < JTSM; i++)
	brkc_.tauc[i] = 0.0;
    for (i = 0; i < JXSM; i++)
	brkc_.xc[i] = 0.0;
    READ (reclen1);
    SWAP4(reclen1) ;
    READ (tbl_record_len);
    SWAP4(tbl_record_len) ;
    READ (nl);
    SWAP4(nl) ;
    READ (len2);
    SWAP4(len2) ;
/* casts to int required to match swap4 function supplied by glp.
Original did not seem to need this */
    READ (tabc_.xn);
    FSWAP4(tabc_.xn) ;
    READ (tabc_.pn);
    FSWAP4(tabc_.pn) ;
    READ (tabc_.tn);
    FSWAP4(tabc_.tn) ;
    READ (umdc_.mt);
#ifndef WORDS_BIGENDIAN
    vectorswap4(umdc_.mt, umdc_.mt,2) ;
#endif
    READ (brkc_.nseg);
    SWAP4(brkc_.nseg) ;
    READ (brkc_.nbrn);
    SWAP4(brkc_.nbrn) ;
    READ (brkc_.ku);
#ifndef WORDS_BIGENDIAN
    vectorswap4(brkc_.ku, brkc_.ku, 2) ;
#endif
    READ (brkc_.km);
#ifndef WORDS_BIGENDIAN
    vectorswap4(brkc_.km, brkc_.km, 2) ;
#endif
    READ (brkc_.fcs);
#ifndef WORDS_BIGENDIAN
    //vectorswap4(brkc_.fcs, brkc_.fcs, 90) ;
    vectorswap4((int*)brkc_.fcs, (int *)brkc_.fcs, 90) ;
#endif
    READ (brkc_.nafl);
#ifndef WORDS_BIGENDIAN
    vectorswap4(brkc_.nafl, brkc_.nafl, 90) ;
#endif
    READ (brkc_.indx);
#ifndef WORDS_BIGENDIAN
    vectorswap4(brkc_.indx, brkc_.indx, 60) ;
#endif
    READ (brkc_.kndx);
#ifndef WORDS_BIGENDIAN
    vectorswap4(brkc_.kndx, brkc_.kndx, 60) ;
#endif
    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    READ (reclen1);
    SWAP4(reclen1) ;
    READ (umdc_.pm);
    p8 = (unsigned char *) umdc_.pm ; 
    //md2hd(&p8, (double *) p8, 300) ;
    md2hd((double *)p8, (double *) p8, 300) ;
    READ (umdc_.zm);
    p8 = (unsigned char *) umdc_.zm ; 
    //md2hd(&p8, (double *) p8, 300) ;
    md2hd((double *)p8, (double *) p8, 300) ;
    READ (umdc_.ndex);
#ifndef WORDS_BIGENDIAN
    vectorswap4(umdc_.ndex, umdc_.ndex, 300) ;
#endif
    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    READ (reclen1);
    SWAP4(reclen1) ;
    READ (brkc_.pu);
    p8 = (unsigned char *) brkc_.pu ; 
    //md2hd(&p8, (double *) p8, 702) ;
    md2hd((double *)p8, (double *) p8, 702) ;
    READ (brkc_.pux);
    p8 = (unsigned char *) brkc_.pux ; 
    //md2hd(&p8, (double *) p8, 200) ;
    md2hd((double *)p8, (double *) p8, 200) ;
    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    READ (reclen1);
    SWAP4(reclen1) ;
    READ (pcdc_.phcd);
    READ (brkc_.px);
    p8 = (unsigned char *) brkc_.px ; 
    //md2hd(&p8, (double *) p8, 200) ;
    md2hd((double *)p8, (double *) p8, 200) ;
    READ (brkc_.xt);
    p8 = (unsigned char *) brkc_.xt ; 
    //md2hd(&p8, (double *) p8, 200) ;
    md2hd((double *)p8, (double *) p8, 200) ;
    READ (tabc_.jndx);
#ifndef WORDS_BIGENDIAN
    vectorswap4(tabc_.jndx, tabc_.jndx, 200) ;
#endif
    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    READ (reclen1);
    SWAP4(reclen1) ;
    READ (tabc_.pt);
    p8 = (unsigned char *) tabc_.pt ; 
    //md2hd(&p8, (double *) p8, 2250) ;
    md2hd((double *)p8, (double *) p8, 2250) ;
    READ (brkc_.taut);
    p8 = (unsigned char *) brkc_.taut ; 
    //md2hd(&p8, (double *) p8, 2250) ;
    md2hd((double *)p8, (double *) p8, 2250) ;

    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    READ (reclen1);
    SWAP4(reclen1) ;
    READ (brkc_.coef);
    p8 = (unsigned char *) brkc_.coef ; 
    //md2hd(&p8, (double *) p8, 11250) ;
    md2hd((double *)p8, (double *) p8, 11250) ;
    READ (reclen2);
    SWAP4(reclen2) ;
    if (reclen1 != reclen2)
	elog_die(0, "record lengths don't match -- can't read '%s'\n",
	     filename);

    if (error != 0)
	elog_die(1, "failure reading header file '%s'\n", filename);
	fclose(fin) ; 
    strcpy (filename + strlen (filename) - 3, "tbl");
    if (tbl_data != 0) {
	munmap (tbl_data, tbl_data_size);
	tbl_data = 0;
    }
    if ((fd = taupmapafile (filename, &tbl_data, &tbl_data_size, 0)) < 0)
				elog_die(1, "Couldn't open table file '%s'\n", filename);
    close (fd);
    tbl_maxrec = tbl_data_size / tbl_record_len;
    for (nph = 1; nph <= 2; ++nph) {
	brkc_.pu[brkc_.ku[nph - 1] + 1 + nph * 351 - 352] = umdc_.pm[nph *
								 150 - 150];
    }
    tabc_.tn = (float) 1. / tabc_.tn;
    tabc_.dn = (float) 3.1415927 / (tabc_.pn * (float) 180. * tabc_.xn);
    brkc_.odep = (float) -1.;
    brkc_.ki = 0;
    brkc_.msrc[0] = 0;
    brkc_.msrc[1] = 0;
    k = 1;

    i__2 = brkc_.nbrn;
    for (i__ = 1; i__ <= i__2; ++i__) {
	brkc_.jidx[i__ - 1] = tabc_.jndx[i__ + 99];
	for (j = 1; j <= 2; ++j) {
	    tabc_.dbrn[i__ + j * 100 - 101] = -1.;
	}
L8:
	if (tabc_.jndx[i__ + 99] <= brkc_.indx[k + 29]) {
	    goto L7;
	}
	++k;
	goto L8;
L7:
	if (brkc_.nafl[k + 29] > 0) {
	    goto L9;
	}
	ind = brkc_.nafl[k - 1];
	l = 0;
	i__3 = tabc_.jndx[i__ + 99];
	for (j = tabc_.jndx[i__ - 1]; j <= i__3; ++j) {
	    ++l;
	    brkc_.tp[l + ind * 100 - 101] = tabc_.pt[j - 1];
	}
L9:
	if (brkc_.nafl[k - 1] > 0 && (*(unsigned char *) &pcdc_.phcd[(i__ -
		1) * 8] == 'P' || *(unsigned char *) &pcdc_.phcd[(i__ - 1) *
							       8] == 'S')) {
	    goto L3;
	}
	for (j = 1; j <= 6; ++j) {

	    if (s_cmp (pcdc_.phcd + ((i__ - 1) << 3), phdif + ((j - 1) << 3), 8L,
		       8L) == 0) {
		goto L6;
	    }
	}
	return;
L6:
	tabc_.dbrn[i__ - 1] = 1.;
	/* There are large numbers of these that were fixed to silence warnings about
            dangerous parenthesis by gcc.   This is the original:
	s_copy (phdif + (j - 1 << 3), " ", 8L, 1L);
	This is the edited version */
	s_copy (phdif + ((j - 1) << 3), " ", 8L, 1L);
	goto L3;
L3:
        ;
    }
    return;
}

int TauPCalculator::brnset_(int *nn, char *pcntl,int *prflg,int pcntl_len)
{
    static char     cmdcd[8 * 4 + 1] = "P       P+      basic   S+      ";
    static char     cmdlst[8 * 16 + 1] = "P       PKiKP   PcP     pP      pPKiKP  sP\
      sPKiKP  ScP     SKP     PKKP    SKKP    PP      S       ScS     sS    \
  pS      ";
    static int      ncmpt[8] = {1, 2, 1, 7, 1, 13, 13, 16};
    int             i__1,
                    i__2,
                    i__3;
    static int      kseg,
                    i__,
                    j,
                    l,
                    k;
    static char     segcd[8 * 100],
                    phtmp[8],
                    phlst[8 * 30];
    static int      nsgpt[100],
                    j1,
                    j2,
                    no;
    static int      fnd,
                    all;
    pcntl -= pcntl_len;
    --prflg;
    no = MIN (*nn, 30);
    i__1 = no;
    for (i__ = 1; i__ <= i__1; ++i__) {
	s_copy (phlst + ((i__ - 1) << 3), pcntl + i__ * pcntl_len, 8L, pcntl_len);
    }
    all = FALSE_;
    if (no == 1 && (s_cmp (phlst, "all", 8L, 3L) == 0 || s_cmp (phlst, "ALL",
							    8L, 3L) == 0)) {
	all = TRUE_;
    }
    if (all) {
	return 0;
    }
    kseg = 0;
    j = 0;
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!all) {
	    prtflc_.segmsk[i__ - 1] = FALSE_;
	}
L9:
	++j;
	s_copy (phtmp, pcdc_.phcd + ((j - 1) << 3), 8L, 8L);
	for (l = 2; l <= 8; ++l) {
    L6:
	    if (*(unsigned char *) &phtmp[l - 1] == ' ') {
		goto L4;
	    }
	    if (*(unsigned char *) &phtmp[l - 1] != 'g' && *(unsigned char *) &
		  phtmp[l - 1] != 'b' && *(unsigned char *) &phtmp[l - 1] !=
		    'n') {
		goto L5;
	    }
	    if (l < 8) {
		i__2 = l;
		s_copy (phtmp + (l - 1), phtmp + i__2, 8 - (l - 1), 8 - i__2);
	    }
	    if (l >= 8) {
		s_copy (phtmp + (l - 1), " ", 8 - (l - 1), 1L);
	    }
	    goto L6;
    L5:
	    if (l >= 8) {
		goto L3;
	    }
	    if (s_cmp (phtmp + (l - 1), "ab", 2L, 2L) != 0 && s_cmp (phtmp + (l
		   - 1), "ac", 2L, 2L) != 0 && s_cmp (phtmp + (l - 1), "df",
						      2L, 2L) != 0) {
		goto L3;
	    }
	    s_copy (phtmp + (l - 1), " ", 8 - (l - 1), 1L);
	    goto L4;
    L3:
	    ;
	}
L4:
	if (kseg < 1) {
	    goto L7;
	}
	if (s_cmp (phtmp, segcd + ((kseg - 1) << 3), 8L, 8L) == 0) {
	    goto L8;
	}
L7:
	++kseg;
	s_copy (segcd + ((kseg - 1) << 3), phtmp, 8L, 8L);
	nsgpt[kseg - 1] = i__;
L8:
	if (brkc_.jidx[j - 1] < brkc_.indx[i__ + 29]) {
	    goto L9;
	}
    }
    if (all) {
	goto L24;
    }
    i__1 = no;
    for (i__ = 1; i__ <= i__1; ++i__) {
	for (j = 1; j <= 4; ++j) {
	    if (s_cmp (phlst + ((i__ - 1) << 3), cmdcd + ((j - 1) << 3), 8L, 8L) ==
		    0) {
		goto L12;
	    }
	}
	fnd = FALSE_;
	i__2 = kseg;
	for (k = 1; k <= i__2; ++k) {
	    if (s_cmp (phlst + ((i__ - 1) << 3), segcd + ((k - 1) << 3), 8L, 8L) !=
		    0) {
		goto L14;
	    }
	    fnd = TRUE_;
	    l = nsgpt[k - 1];
	    prtflc_.segmsk[l - 1] = TRUE_;
    L14:
	    ;
	}
	if (!fnd) {
	    elog_log (0, "Brnset:  phase %s not found", phlst + ((i__ - 1) << 3));
	}
	goto L10;
L12:
	j1 = ncmpt[(j << 1) - 2];
	j2 = ncmpt[(j << 1) - 1];
	i__2 = j2;
	for (j = j1; j <= i__2; ++j) {
	    i__3 = kseg;
	    for (k = 1; k <= i__3; ++k) {
		if (s_cmp (cmdlst + ((j - 1) << 3), segcd + ((k - 1) << 3), 8L, 8L)
			!= 0) {
		    goto L15;
		}
		l = nsgpt[k - 1];
		prtflc_.segmsk[l - 1] = TRUE_;
	L15:
		;
	    }
	}
L10:
	;
    }
L24:
    return 0;
}


double TauPCalculator::umod_0_(int n__,double *zs,int *isrc,int *nph,double *uend,int *js)
{
    const double   dtol(1e-6);
    int             i__1;
    double          ret_val,
                    d__1,
                    d__2;
    static int      i__,
                    j,
                    m1;
    static float    dep;
    if (isrc) {
	--isrc;
    }
    switch (n__) {
    case 1:
	goto L_zmod;
    }
    m1 = umdc_.mt[*nph - 1];
    i__1 = m1;
    for (i__ = 2; i__ <= i__1; ++i__) {
	if (umdc_.zm[i__ + *nph * 150 - 151] <= *zs) {
	    goto L2;
	}
    }
    dep = (1. - exp (*zs)) / tabc_.xn;
    elog_die (0, "Source depth %6.1f too deep", dep);
L2:
    if ((d__1 = *zs - umdc_.zm[i__ + *nph * 150 - 151], ABS (d__1)) <= dtol &&
	    (d__2 = umdc_.zm[i__ + *nph * 150 - 151] - umdc_.zm[i__ + 1 + *
				    nph * 150 - 151], ABS (d__2)) <= dtol) {
	goto L3;
    }
    j = i__ - 1;
    isrc[*nph] = j;
    ret_val = umdc_.pm[j + *nph * 150 - 151] + (umdc_.pm[i__ + *nph * 150 -
	     151] - umdc_.pm[j + *nph * 150 - 151]) * (exp (*zs - umdc_.zm[j
	      + *nph * 150 - 151]) - 1.) / (exp (umdc_.zm[i__ + *nph * 150 -
			       151] - umdc_.zm[j + *nph * 150 - 151]) - 1.);
    return ret_val;
L3:
    isrc[*nph] = i__;
    ret_val = umdc_.pm[i__ + 1 + *nph * 150 - 151];
    return ret_val;
L_zmod:
    i__ = *js + 1;
    d__1 = (*uend - umdc_.pm[*js + *nph * 150 - 151]) * (exp (umdc_.zm[i__ +
	     *nph * 150 - 151] - umdc_.zm[*js + *nph * 150 - 151]) - 1.) / (
	      umdc_.pm[i__ + *nph * 150 - 151] - umdc_.pm[*js + *nph * 150 -
							  151]) + 1.;
    ret_val = umdc_.zm[*js + *nph * 150 - 151] + log ((MAX (d__1, 1e-30)));
    return ret_val;
}
/* umod_ and zmod_ are the original fortran function mames.  zmod is a secondary
entry point, which leads to this rather weird interface created by f2c.   
Note arg 1 of umod_0_ is different from umod and zmod, which is how the 
entry statement was translated. */
double TauPCalculator::umod_(double *zs,int *isrc,int *nph)
{
    double result;
    result=this->umod_0_ (0, zs, isrc, nph, (double *) 0, (int *) 0);
    return result;
}

double TauPCalculator::zmod_(double *uend,int *js, int *nph)
{
    double result;
    result=this->umod_0_ (1, (double *) 0, (int *) 0, nph, uend, js);
    return result;
}

int tauint_(double *ptk, double *ptj, double *pti,
		double *zj, double *zi, double *tau, double *x)
{
    double          d__1,
                    d__2,
                    d__3,
                    d__4,
                    d__5,
                    d__6;
    static double   b,
                    xx,
                    sqb,
                    sqk,
                    sqi,
                    sqj;
    if ((d__1 = *zj - *zi, ABS (d__1)) <= 1e-9) {
	goto L13;
    }
    if ((d__1 = *ptj - *pti, ABS (d__1)) > 1e-9) {
	goto L10;
    }
    if ((d__1 = *ptk - *pti, ABS (d__1)) <= 1e-9) {
	goto L13;
    }
    b = (d__1 = *zj - *zi, ABS (d__1));
    sqj = sqrt ((d__1 = *ptj * *ptj - *ptk * *ptk, ABS (d__1)));
    *tau = b * sqj;
    *x = b * *ptk / sqj;
    goto L4;
L10:
    if (*ptk > 1e-9 || *pti > 1e-9) {
	goto L1;
    }
    *tau = *ptj;
    *x = 1.5707963267948966;
    goto L4;
L1:
    b = *ptj - (*pti - *ptj) / (exp (*zi - *zj) - 1.);
    if (*ptk > 1e-9) {
	goto L2;
    }
    d__1 = (*ptj - b) * *pti / ((*pti - b) * *ptj);
    *tau = -(*pti - *ptj + b * log (*pti / *ptj) - b * log ((MAX (d__1, 1e-30))));
    *x = 0.;
    goto L4;
L2:
    if (*ptk == *pti) {
	goto L3;
    }
    if (*ptk == *ptj) {
	goto L11;
    }
    sqk = *ptk * *ptk;
    sqi = sqrt ((d__1 = *pti * *pti - sqk, ABS (d__1)));
    sqj = sqrt ((d__1 = *ptj * *ptj - sqk, ABS (d__1)));
    sqb = sqrt ((d__1 = b * b - sqk, ABS (d__1)));
    if (sqb > 1e-30) {
	goto L5;
    }
    xx = 0.;
    *x = *ptk * (sqrt ((d__1 = (*pti + b) / (*pti - b), ABS (d__1))) - sqrt ((
			  d__2 = (*ptj + b) / (*ptj - b), ABS (d__2)))) / b;
    goto L6;
L5:
    if (b * b < sqk) {
	goto L7;
    }
    d__1 = (*ptj - b) * (sqb * sqi + b * *pti - sqk) / ((*pti - b) * (sqb *
						     sqj + b * *ptj - sqk));
    xx = log ((MAX (d__1, 1e-30)));
    *x = *ptk * xx / sqb;
    goto L6;
L7:
    d__4 = (b * *pti - sqk) / (*ptk * (d__1 = *pti - b, ABS (d__1)));
    d__3 = MIN (d__4, 1.);
    d__6 = (b * *ptj - sqk) / (*ptk * (d__2 = *ptj - b, ABS (d__2)));
    d__5 = MIN (d__6, 1.);
    xx = asin ((MAX (d__3, -1.))) - asin ((MAX (d__5, -1.)));
    *x = -(*ptk) * xx / sqb;
L6:
    *tau = -(sqi - sqj + b * log ((*pti + sqi) / (*ptj + sqj)) - sqb * xx);
    goto L4;
L3:
    sqk = *pti * *pti;
    sqj = sqrt ((d__1 = *ptj * *ptj - sqk, ABS (d__1)));
    sqb = sqrt ((d__1 = b * b - sqk, ABS (d__1)));
    if (b * b < sqk) {
	goto L8;
    }
    d__1 = (*ptj - b) * (b * *pti - sqk) / ((*pti - b) * (sqb * sqj + b * *
							  ptj - sqk));
    xx = log ((MAX (d__1, 1e-30)));
    *x = *pti * xx / sqb;
    goto L9;
L8:
    d__2 = b - *pti;
    d__4 = (b * *ptj - sqk) / (*pti * (d__1 = *ptj - b, ABS (d__1)));
    d__3 = MIN (d__4, 1.);
    xx = d_sign (&m_pi_2, &d__2) - asin ((MAX (d__3, -1.)));
    *x = -(*pti) * xx / sqb;
L9:
    *tau = -(b * log (*pti / (*ptj + sqj)) - sqj - sqb * xx);
    goto L4;
L11:
    sqk = *ptj * *ptj;
    sqi = sqrt ((d__1 = *pti * *pti - sqk, ABS (d__1)));
    sqb = sqrt ((d__1 = b * b - sqk, ABS (d__1)));
    if (b * b < sqk) {
	goto L12;
    }
    d__1 = (*ptj - b) * (sqb * sqi + b * *pti - sqk) / ((*pti - b) * (b * *
								ptj - sqk));
    xx = log ((MAX (d__1, 1e-30)));
    *x = *ptj * xx / sqb;
    goto L14;
L12:
    d__3 = (b * *pti - sqk) / (*ptj * (d__1 = *pti - b, ABS (d__1)));
    d__2 = MIN (d__3, 1.);
    d__4 = b - *ptj;
    xx = asin ((MAX (d__2, -1.))) - d_sign (&m_pi_2, &d__4);
    *x = -(*ptj) * xx / sqb;
L14:
    *tau = -(b * log ((*pti + sqi) / *ptj) + sqi - sqb * xx);
L4:
    if (*x >= -1e-10) {
	goto L15;
    }
    elog_log (0, "Bad range: %12.4f %12.4f %12.4f %12.4f %12.4f", *ptk, *ptj,
	      *pti, *tau, *x);
L15:
    if (*tau >= -1e-10) {
	goto L16;
    }
    elog_log (0, "Bad tau: %12.4f %12.4f %12.4f %12.4f %12.4f", *ptk, *ptj, *pti,
	      *tau, *x);
L16:
    return 0;
L13:
    *tau = 0.;
    *x = 0.;
    return 0;
}

int TauPCalculator::depcor_(int *nph)
{
    static float    tol = (float) .01;
    static double   dtol = 1e-6;
    static int      lpower = 7;
    int             i__1,
                    i__2;
    float           r__1;
    double          d__1,
                    d__2;
    static double   xus1[2],
                    xus2[2];
    static double   umin;
    static double   ttau;
    static float    ztol;
    static int      i__,
                    j,
                    k,
                    l,
                    m;
    static int      noend;
    static int      i1,
                    i2,
                    k1;
    static int      noext;
    static int      k2,
                    n1;
    static double   u0,
                    u1,
                    z0,
                    z1,
                    tauus1[2],
                    tauus2[2],
                    du;
    static int      ks,
                    ms;
    static double   tx;
    static int      mu,
                    is,
                    lp;
    static double   fac;
    static int      iph,
                    kph;
    static double   sgn;
#define tup brkc_.tauc
    pdec_.deplim = 1.1;
    //pdec_.ka = 1.1;
    pdec_.ka = 1;
    if (*nph == brkc_.nph0) {
	goto L1;
    }
    brkc_.nph0 = *nph;
    tabc_.us[*nph - 1] = umod_ (&brkc_.zs, brkc_.isrc, nph);
    umin = tabc_.us[*nph - 1];
    ks = brkc_.isrc[*nph - 1];
    i__1 = ks;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (umdc_.pm[i__ + *nph * 150 - 151] > umin) {
	    goto L2;
	}
	umin = umdc_.pm[i__ + *nph * 150 - 151];
L2:
	;
    }
    n1 = brkc_.ku[*nph - 1] + 1;
    i__1 = n1;
    for (i__ = 2; i__ <= i__1; ++i__) {
	if (brkc_.pu[i__ + *nph * 351 - 352] > umin) {
	    goto L4;
	}
    }
    k2 = n1;
    if (brkc_.pu[n1 + *nph * 351 - 352] == umin) {
	goto L50;
    }
    elog_die (0, "Source slowness too large.");
L4:
    k2 = i__;
L50:
    noext = FALSE_;
    sgn = 1.;
    if (brkc_.msrc[*nph - 1] == 0) {
	brkc_.msrc[*nph - 1] = 1;
    }
    ztol = tabc_.xn * tol / ((float) 1. - tabc_.xn * brkc_.odep);
    if ((d__1 = brkc_.zs - umdc_.zm[ks + 1 + *nph * 150 - 151], ABS (d__1))
	    > ztol) {
	goto L5;
    }
    ++ks;
    goto L6;
L5:
    if ((d__1 = brkc_.zs - umdc_.zm[ks + *nph * 150 - 151], ABS (d__1)) >
	    ztol) {
	goto L7;
    }
L6:
    noext = TRUE_;
    if (brkc_.msrc[*nph - 1] == ks) {
	goto L8;
    }
    i__1 = brkc_.ku[*nph - 1] + brkc_.km[*nph - 1];
    bkin_ (&brkc_.nin, &umdc_.ndex[ks + *nph * 150 - 151], &i__1, tup);
    goto L11;
L7:
    if (brkc_.msrc[*nph - 1] != ks + 1) {
	goto L9;
    }
    ++ks;
    sgn = -1.;
    goto L8;
L9:
    if (brkc_.msrc[*nph - 1] == ks) {
	goto L8;
    }
    if ((d__1 = umdc_.zm[ks + *nph * 150 - 151] - brkc_.zs, ABS (d__1)) <= (
	   d__2 = umdc_.zm[ks + 1 + *nph * 150 - 151] - brkc_.zs, ABS (d__2)
									)) {
	goto L10;
    }
    ++ks;
    sgn = -1.;
L10:
    i__1 = brkc_.ku[*nph - 1] + brkc_.km[*nph - 1];
    bkin_ (&brkc_.nin, &umdc_.ndex[ks + *nph * 150 - 151], &i__1, tup);
L11:
    i__1 = brkc_.ku[*nph - 1];
    for (i__ = 1; i__ <= i__1; ++i__) {
	brkc_.tauu[i__ + *nph * 350 - 351] = tup[i__ - 1];
    }
    k = brkc_.ku[*nph - 1];
    i__1 = brkc_.km[*nph - 1];
    for (i__ = 1; i__ <= i__1; ++i__) {
	++k;
	brkc_.xc[i__ - 1] = tup[k - 1];
	brkc_.xu[i__ + *nph * 100 - 101] = tup[k - 1];
    }
L8:
    brkc_.msrc[*nph - 1] = ks;
    noend = FALSE_;
    if ((d__1 = umin - brkc_.pu[k2 - 1 + *nph * 351 - 352], ABS (d__1)) <=
	    dtol * umin) {
	--k2;
    }
    if ((d__1 = umin - brkc_.pu[k2 + *nph * 351 - 352], ABS (d__1)) <= dtol *
	    umin) {
	noend = TRUE_;
    }
    if (brkc_.msrc[*nph - 1] <= 1 && noext) {
	brkc_.msrc[*nph - 1] = 0;
    }
    k1 = k2 - 1;
    if (noend) {
	k1 = k2;
    }
    if (noext) {
	goto L14;
    }
    ms = brkc_.msrc[*nph - 1];
    if (sgn >= 0.) {
	goto L16;
    } else {
	goto L15;
    }
L16:
    u0 = umdc_.pm[ms + *nph * 150 - 151];
    z0 = umdc_.zm[ms + *nph * 150 - 151];
    u1 = tabc_.us[*nph - 1];
    z1 = brkc_.zs;
    goto L17;
L15:
    u0 = tabc_.us[*nph - 1];
    z0 = brkc_.zs;
    u1 = umdc_.pm[ms + *nph * 150 - 151];
    z1 = umdc_.zm[ms + *nph * 150 - 151];
L17:
    mu = 1;
    i__1 = k1;
    for (k = 1; k <= i__1; ++k) {
	tauint_ (&brkc_.pu[k + *nph * 351 - 352], &u0, &u1, &z0, &z1, &ttau, &
		 tx);
	brkc_.tauc[k - 1] = brkc_.tauu[k + *nph * 350 - 351] + sgn * ttau;
	if ((d__1 = brkc_.pu[k + *nph * 351 - 352] - brkc_.pux[mu + *nph *
					   100 - 101], ABS (d__1)) > dtol) {
	    goto L18;
	}
	brkc_.xc[mu - 1] = brkc_.xu[mu + *nph * 100 - 101] + sgn * tx;
	++mu;
L18:
	;
    }
    goto L39;
L14:
    mu = 1;
    i__1 = k1;
    for (k = 1; k <= i__1; ++k) {
	brkc_.tauc[k - 1] = brkc_.tauu[k + *nph * 350 - 351];
	if ((d__1 = brkc_.pu[k + *nph * 351 - 352] - brkc_.pux[mu + *nph *
					   100 - 101], ABS (d__1)) > dtol) {
	    goto L40;
	}
	brkc_.xc[mu - 1] = brkc_.xu[mu + *nph * 100 - 101];
	++mu;
L40:
	;
    }
L39:
    xus1[*nph - 1] = 0.;
    xus2[*nph - 1] = 0.;
    --mu;
    if ((d__1 = umin - tabc_.us[*nph - 1], ABS (d__1)) > dtol && (d__2 = umin
		  - brkc_.pux[mu + *nph * 100 - 101], ABS (d__2)) <= dtol) {
	--mu;
    }
    if (brkc_.msrc[*nph - 1] <= 0) {
	goto L1;
    }
    is = brkc_.isrc[*nph - 1];
    tauus2[*nph - 1] = 0.;
    if ((d__1 = brkc_.pux[mu + *nph * 100 - 101] - umin, ABS (d__1)) > dtol ||
	    (d__2 = tabc_.us[*nph - 1] - umin, ABS (d__2)) > dtol) {
	goto L48;
    }
    tauus1[*nph - 1] = brkc_.tauc[k1 - 1];
    xus1[*nph - 1] = brkc_.xc[mu - 1];
    goto L33;
L48:
    tauus1[*nph - 1] = 0.;
    j = 1;
    if (is < 2) {
	goto L42;
    }
    i__1 = is;
    for (i__ = 2; i__ <= i__1; ++i__) {
	tauint_ (&umin, &umdc_.pm[j + *nph * 150 - 151], &umdc_.pm[i__ + *
			nph * 150 - 151], &umdc_.zm[j + *nph * 150 - 151], &
		 umdc_.zm[i__ + *nph * 150 - 151], &ttau, &tx);
	tauus1[*nph - 1] += ttau;
	xus1[*nph - 1] += tx;
	j = i__;
    }
L42:
    if ((d__1 = umdc_.zm[is + *nph * 150 - 151] - brkc_.zs, ABS (d__1)) <=
	    dtol) {
	goto L33;
    }
    tauint_ (&umin, &umdc_.pm[is + *nph * 150 - 151], &tabc_.us[*nph - 1], &
	     umdc_.zm[is + *nph * 150 - 151], &brkc_.zs, &ttau, &tx);
    tauus1[*nph - 1] += ttau;
    xus1[*nph - 1] += tx;
L33:
    if (umdc_.pm[is + 1 + *nph * 150 - 151] < umin) {
	goto L41;
    }
    u1 = tabc_.us[*nph - 1];
    z1 = brkc_.zs;
    i__1 = umdc_.mt[*nph - 1];
    for (i__ = is + 1; i__ <= i__1; ++i__) {
	u0 = u1;
	z0 = z1;
	u1 = umdc_.pm[i__ + *nph * 150 - 151];
	z1 = umdc_.zm[i__ + *nph * 150 - 151];
	if (u1 < umin) {
	    goto L36;
	}
	tauint_ (&umin, &u0, &u1, &z0, &z1, &ttau, &tx);
	tauus2[*nph - 1] += ttau;
	xus2[*nph - 1] += tx;
    }
L36:
    i__1 = i__ - 1;
    z1 = zmod_ (&umin, &i__1, nph);
    if ((d__1 = z0 - z1, ABS (d__1)) <= dtol) {
	goto L41;
    }
    tauint_ (&umin, &u0, &umin, &z0, &z1, &ttau, &tx);
    tauus2[*nph - 1] += ttau;
    xus2[*nph - 1] += tx;
L41:
    iph = *nph % 2 + 1;
    xus1[iph - 1] = 0.;
    xus2[iph - 1] = 0.;
    tauus1[iph - 1] = 0.;
    tauus2[iph - 1] = 0.;
    switch ((int) *nph) {
    case 1:
	goto L59;
    case 2:
	goto L61;
    }
L61:
    if (umin > brkc_.pu[brkc_.ku[0]]) {
	goto L53;
    }
    i__1 = brkc_.nbrn;
    for (j = 1; j <= i__1; ++j) {
	if ((s_cmp (pcdc_.phcd + ((j - 1) << 3), "sP", 2L, 2L) != 0 && s_cmp (
		 pcdc_.phcd + ((j - 1) << 3), "SP", 2L, 2L) != 0) || brkc_.px[j
							      + 99] <= 0.) {
	    goto L44;
	}
	if (umin >= brkc_.px[j - 1] && umin < brkc_.px[j + 99]) {
	    goto L45;
	}
L44:
	;
    }
    goto L53;
L59:
    i__1 = brkc_.nbrn;
    for (j = 1; j <= i__1; ++j) {
	if ((s_cmp (pcdc_.phcd + ((j - 1) << 3), "pS", 2L, 2L) != 0 && s_cmp (
		 pcdc_.phcd + ((j - 1) << 3), "PS", 2L, 2L) != 0) || brkc_.px[j
							      + 99] <= 0.) {
	    goto L60;
	}
	if (umin >= brkc_.px[j - 1] && umin < brkc_.px[j + 99]) {
	    goto L45;
	}
L60:
	;
    }
    goto L53;
L45:
    j = 1;
    i__1 = umdc_.mt[iph - 1];
    for (i__ = 2; i__ <= i__1; ++i__) {
	if (umin >= umdc_.pm[i__ + iph * 150 - 151]) {
	    goto L47;
	}
	tauint_ (&umin, &umdc_.pm[j + iph * 150 - 151], &umdc_.pm[i__ + iph *
		   150 - 151], &umdc_.zm[j + iph * 150 - 151], &umdc_.zm[i__
					    + iph * 150 - 151], &ttau, &tx);
	tauus1[iph - 1] += ttau;
	xus1[iph - 1] += tx;
	j = i__;
    }
L47:
    z1 = zmod_ (&umin, &j, &iph);
    if ((d__1 = umdc_.zm[j + iph * 150 - 151] - z1, ABS (d__1)) <= dtol) {
	goto L53;
    }
    tauint_ (&umin, &umdc_.pm[j + iph * 150 - 151], &umin, &umdc_.zm[j + iph
					     * 150 - 151], &z1, &ttau, &tx);
    tauus1[iph - 1] += ttau;
    xus1[iph - 1] += tx;
L53:
    pdec_.ua[*nph * 5 - 5] = -1.;
    if (brkc_.odep >= pdec_.deplim) {
	goto L43;
    }
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!prtflc_.segmsk[i__ - 1]) {
	    goto L57;
	}
	if (brkc_.nafl[i__ - 1] == *nph && brkc_.nafl[i__ + 29] == 0 &&
		brkc_.iidx[i__ - 1] <= 0) {
	    goto L58;
	}
L57:
	;
    }
    goto L43;
L58:
    r__1 = (brkc_.odep - (float) .4) * (float) 2e-5 + (float) 1e-5;
    du = MIN (r__1, (float) 1e-5);
    lp = lpower;
    k = 0;
    for (l = pdec_.ka; l >= 1; --l) {
	++k;
	pdec_.ua[k + *nph * 5 - 6] = tabc_.us[*nph - 1] - pow_ii (&l, &lp) *
	    du;
	--lp;
	pdec_.taua[k + *nph * 5 - 6] = 0.;
	j = 1;
	if (is < 2) {
	    goto L54;
	}
	i__1 = is;
	for (i__ = 2; i__ <= i__1; ++i__) {
	    tauint_ (&pdec_.ua[k + *nph * 5 - 6], &umdc_.pm[j + *nph * 150 -
		    151], &umdc_.pm[i__ + *nph * 150 - 151], &umdc_.zm[j + *
		      nph * 150 - 151], &umdc_.zm[i__ + *nph * 150 - 151], &
		     ttau, &tx);
	    pdec_.taua[k + *nph * 5 - 6] += ttau;
	    j = i__;
	}
L54:
	if ((d__1 = umdc_.zm[is + *nph * 150 - 151] - brkc_.zs, ABS (d__1))
		<= dtol) {
	    goto L56;
	}
	tauint_ (&pdec_.ua[k + *nph * 5 - 6], &umdc_.pm[is + *nph * 150 -
		151], &tabc_.us[*nph - 1], &umdc_.zm[is + *nph * 150 - 151],
		 &brkc_.zs, &ttau, &tx);
	pdec_.taua[k + *nph * 5 - 6] += ttau;
L56:
	;
    }
    goto L43;
L1:
    ++mu;
L43:
    j = 1;
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!prtflc_.segmsk[i__ - 1]) {
	    goto L20;
	}
	if (brkc_.iidx[i__ - 1] > 0 || (i__2 = brkc_.nafl[i__ - 1], ABS (
		  i__2)) != *nph || (brkc_.msrc[*nph - 1] <= 0 && brkc_.nafl[
							     i__ - 1] > 0)) {
	    goto L20;
	}
	iph = brkc_.nafl[i__ + 29];
	kph = brkc_.nafl[i__ + 59];
	if (iph <= 0) {
	    iph = *nph;
	}
	if (kph <= 0) {
	    kph = *nph;
	}
	sgn = (double) i_sign (&c__1, &brkc_.nafl[i__ - 1]);
	i1 = brkc_.indx[i__ - 1];
	i2 = brkc_.indx[i__ + 29];
	m = 1;
	i__2 = i2;
	for (k = i1; k <= i__2; ++k) {
	    if (tabc_.pt[k - 1] > umin) {
		goto L22;
	    }
    L23:
	    if ((d__1 = tabc_.pt[k - 1] - brkc_.pu[m + *nph * 351 - 352],
		 ABS (d__1)) <= dtol) {
		goto L21;
	    }
	    ++m;
	    goto L23;
    L21:
	    tabc_.tau[(k << 2) - 4] = brkc_.taut[k - 1] + sgn * brkc_.tauc[
								     m - 1];
	}
	k = i2;
	goto L24;
L22:
	if ((d__1 = tabc_.pt[k - 2] - umin, ABS (d__1)) <= dtol) {
	    --k;
	}
	++brkc_.ki;
	brkc_.kk[brkc_.ki - 1] = k;
	brkc_.pk[brkc_.ki - 1] = tabc_.pt[k - 1];
	tabc_.pt[k - 1] = umin;
	fac = brkc_.fcs[i__ - 1];
	tabc_.tau[(k << 2) - 4] = fac * (tauus1[iph - 1] + tauus2[iph - 1] +
		tauus1[kph - 1] + tauus2[kph - 1]) + sgn * tauus1[*nph - 1];
L24:
	m = 1;
L26:
	if (tabc_.jndx[j - 1] >= brkc_.indx[i__ - 1]) {
	    goto L25;
	}
	++j;
	goto L26;
L25:
	i__2 = brkc_.jidx[j - 1];
	tabc_.jndx[j + 99] = MIN (i__2, k);
	if (tabc_.jndx[j - 1] < tabc_.jndx[j + 99]) {
	    goto L37;
	}
	tabc_.jndx[j + 99] = -1;
	goto L20;
L37:
	for (l = 1; l <= 2; ++l) {
    L28:
	    if ((d__1 = brkc_.pux[m + *nph * 100 - 101] - brkc_.px[j + l *
					  100 - 101], ABS (d__1)) <= dtol) {
		goto L27;
	    }
	    if (m >= mu) {
		goto L29;
	    }
	    ++m;
	    goto L28;
    L27:
	    tabc_.xbrn[j + l * 100 - 101] = brkc_.xt[j + l * 100 - 101] +
		sgn * brkc_.xc[m - 1];
	    goto L30;
    L29:
	    tabc_.xbrn[j + l * 100 - 101] = fac * (xus1[iph - 1] + xus2[iph
		   - 1] + xus1[kph - 1] + xus2[kph - 1]) + sgn * xus1[*nph -
								      1];
    L30:
	    ;
	}
	if (j >= brkc_.nbrn) {
	    goto L20;
	}
	++j;
	if (tabc_.jndx[j - 1] <= k) {
	    goto L25;
	}
L20:
	;
    }
    return 0;
}
/* This is really obnoxious.   tup is defined above as an alias for
part of a struct.  Retain this undef to avoid a difficult search, but 
future maintainers note.*/
#undef tup

int tauspl_(int *i1, int *i2, double *pt, double *coef)              
{
    int             i__1;
    double          d__1;
    static double   deli[5],
                    sdel[5],
                    d__[4];
    static int      i__,
                    j,
                    k,
                    l,
                    m,
                    n2,
                    is;
    static double   b1h,
                    d1h[4],
                    b3h,
                    d3h[4],
                    bih,
                    del[5],
                    dih[4],
                    ali,
                    alr,
                    th2m,
                    th0p,
                    th2p,
                    th3p;
    coef -= 6;
    --pt;
    n2 = *i2 - *i1 - 1;
    if (n2 <= -1) {
	return 0;
    }
    is = *i1 + 1;
    del[1] = pt[*i2] - pt[*i1] + (pt[is] - pt[*i1]) * 3.;
    sdel[1] = sqrt ((ABS (del[1])));
    deli[1] = 1. / sdel[1];
    m = 2;
    for (k = 3; k <= 5; ++k) {
	del[k - 1] = pt[*i2] - pt[*i1] + (5 - k) * (pt[is] - pt[*i1]);
	sdel[k - 1] = sqrt ((d__1 = del[k - 1], ABS (d__1)));
	deli[k - 1] = 1. / sdel[k - 1];
	d3h[m - 1] = del[k - 1] * sdel[k - 1] - del[m - 1] * sdel[m - 1];
	d1h[m - 1] = sdel[k - 1] - sdel[m - 1];
	dih[m - 1] = deli[k - 1] - deli[m - 1];
	m = k;
    }
    l = *i1 - 1;
    if (n2 <= 0) {
	goto L10;
    }
    i__1 = n2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	m = 1;
	for (k = 2; k <= 5; ++k) {
	    del[m - 1] = del[k - 1];
	    sdel[m - 1] = sdel[k - 1];
	    deli[m - 1] = deli[k - 1];
	    if (k >= 5) {
		goto L3;
	    }
	    d3h[m - 1] = d3h[k - 1];
	    d1h[m - 1] = d1h[k - 1];
	    dih[m - 1] = dih[k - 1];
    L3:
	    m = k;
	}
	++l;
	del[4] = pt[*i2] - pt[l + 1];
	sdel[4] = sqrt ((ABS (del[4])));
	deli[4] = 1. / sdel[4];
	d3h[3] = del[4] * sdel[4] - del[3] * sdel[3];
	d1h[3] = sdel[4] - sdel[3];
	dih[3] = deli[4] - deli[3];
	ali = 1. / (d3h[0] * .125 - (d1h[0] * .75 + dih[0] * .375 * del[2]) *
		    del[2]);
	alr = ali * (del[1] * .125 * sdel[1] - (sdel[1] * .75 + del[2] * .375
					     * deli[1] - sdel[2]) * del[2]);
	b3h = d3h[1] + alr * d3h[0];
	b1h = d1h[1] + alr * d1h[0];
	bih = dih[1] + alr * dih[0];
	th0p = d1h[0] * b3h - d3h[0] * b1h;
	th2p = d1h[2] * b3h - d3h[2] * b1h;
	th3p = d1h[3] * b3h - d3h[3] * b1h;
	th2m = dih[2] * b3h - d3h[2] * bih;
	d__[3] = ali * ((dih[0] * b3h - d3h[0] * bih) * th2p - th2m * th0p) /
	    ((dih[3] * b3h - d3h[3] * bih) * th2p - th2m * th3p);
	d__[2] = (th0p * ali - th3p * d__[3]) / th2p;
	d__[1] = (d3h[0] * ali - d3h[2] * d__[2] - d3h[3] * d__[3]) / b3h;
	d__[0] = alr * d__[1] - ali;
	coef[l * 5 + 1] = (del[4] * .125 * sdel[4] - (sdel[4] * .75 + deli[4]
			      * .375 * del[3] - sdel[3]) * del[3]) * d__[3];
	if (i__ >= 3) {
	    coef[(l - 2) * 5 + 2] = (del[0] * .125 * sdel[0] - (sdel[0] * .75
		    + deli[0] * .375 * del[1] - sdel[1]) * del[1]) * d__[0];
	}
	coef[l * 5 + 3] = (sdel[4] + deli[4] * del[3] - sdel[3] * 2.) * -.75 *
	    d__[3];
	if (i__ >= 2) {
	    coef[(l - 1) * 5 + 4] = ((sdel[1] + deli[1] * del[2] - sdel[2] *
		       2.) * d__[1] - (d1h[0] + dih[0] * del[2]) * d__[0]) *
		-.75;
	}
	if (i__ >= 3) {
	    coef[(l - 2) * 5 + 5] = (sdel[0] + deli[0] * del[1] - sdel[1] *
				     2.) * -.75 * d__[0];
	}
    }
L10:
    for (j = 1; j <= 4; ++j) {
	m = 1;
	for (k = 2; k <= 5; ++k) {
	    del[m - 1] = del[k - 1];
	    sdel[m - 1] = sdel[k - 1];
	    deli[m - 1] = deli[k - 1];
	    if (k >= 5) {
		goto L5;
	    }
	    d3h[m - 1] = d3h[k - 1];
	    d1h[m - 1] = d1h[k - 1];
	    dih[m - 1] = dih[k - 1];
    L5:
	    m = k;
	}
	++l;
	del[4] = 0.;
	sdel[4] = 0.;
	deli[4] = 0.;
	if (j < 4) {
	    goto L6;
	}
	d__[0] = 2. / (del[0] * sdel[0]);
	goto L9;
L6:
	alr = (sdel[1] + deli[1] * del[2] - sdel[2] * 2.) / (d1h[0] + dih[0] *
							     del[2]);
	d__[1] = 1. / (del[1] * .125 * sdel[1] - (sdel[1] * .75 + deli[1] *
	      .375 * del[2] - sdel[2]) * del[2] - (d3h[0] * .125 - (d1h[0] *
			     .75 + dih[0] * .375 * del[2]) * del[2]) * alr);
	d__[0] = alr * d__[1];
	if ((i__1 = j - 2) < 0) {
	    goto L8;
	} else if (i__1 == 0) {
	    goto L7;
	} else {
	    goto L9;
	}
L7:
	d__[2] = (d3h[1] * d__[1] + 2. + d3h[0] * d__[0]) / (del[2] * sdel[2]);
	goto L9;
L8:
	d__[2] = -((d3h[1] - d1h[1] * del[3]) * d__[1] + (d3h[0] - d1h[0] *
			     del[3]) * d__[0]) / (d3h[2] - d1h[2] * del[3]);
	d__[3] = (d3h[2] * d__[2] + d3h[1] * d__[1] + d3h[0] * d__[0]) / (del[
							      3] * sdel[3]);
L9:
	if (j <= 2) {
	    coef[l * 5 + 1] = (del[2] * .125 * sdel[2] - (sdel[2] * .75 +
		   deli[2] * .375 * del[3] - sdel[3]) * del[3]) * d__[2] - (
		   d3h[1] * .125 - (d1h[1] * .75 + dih[1] * .375 * del[3]) *
		  del[3]) * d__[1] - (d3h[0] * .125 - (d1h[0] * .75 + dih[0]
					* .375 * del[3]) * del[3]) * d__[0];
	}
	if (l - *i1 > 1) {
	    coef[(l - 2) * 5 + 2] = (del[0] * .125 * sdel[0] - (sdel[0] * .75
		    + deli[0] * .375 * del[1] - sdel[1]) * del[1]) * d__[0];
	}
	if (j <= 2) {
	    coef[l * 5 + 3] = ((sdel[2] + deli[2] * del[3] - sdel[3] * 2.) *
		    d__[2] - (d1h[1] + dih[1] * del[3]) * d__[1] - (d1h[0] +
					  dih[0] * del[3]) * d__[0]) * -.75;
	}
	if (j <= 3 && l - *i1 > 0) {
	    coef[(l - 1) * 5 + 4] = 0.;
	}
	if (l - *i1 > 1) {
	    coef[(l - 2) * 5 + 5] = (sdel[0] + deli[0] * del[1] - sdel[1] *
				     2.) * -.75 * d__[0];
	}
    }
    return 0;
}

int fitspl_(int *i1, int *i2, double *tau, double *x1, double *xn, double *coef)
{
    int             i__1;
    static double   a[120],
                    b[60];
    static int      i__,
                    j,
                    n;
    static int      n1,
                    ie;
    static double   ap[3],
                    gn;
    static int      is;
    static double   alr;
    coef -= 6;
    tau -= 5;
    if ((i__1 = *i2 - *i1) < 0) {
	goto L13;
    } else if (i__1 == 0) {
	goto L1;
    } else {
	goto L2;
    }
L1:
    tau[(*i1 << 2) + 2] = *x1;
L13:
    return 0;
L2:
    n = 0;
    i__1 = *i2;
    for (i__ = *i1; i__ <= i__1; ++i__) {
	++n;
	b[n - 1] = tau[(i__ << 2) + 1];
	for (j = 1; j <= 2; ++j) {
	    a[j + (n << 1) - 3] = coef[j + i__ * 5];
	}
    }
    for (j = 1; j <= 3; ++j) {
	ap[j - 1] = coef[j + 2 + *i2 * 5];
    }
    n1 = n - 1;
    alr = a[0] / coef[*i1 * 5 + 3];
    a[0] = 1. - coef[*i1 * 5 + 4] * alr;
    a[1] -= coef[*i1 * 5 + 5] * alr;
    b[0] -= *x1 * alr;
    j = 1;
    i__1 = n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	alr = a[(i__ << 1) - 2] / a[(j << 1) - 2];
	a[(i__ << 1) - 2] = 1. - a[(j << 1) - 1] * alr;
	b[i__ - 1] -= b[j - 1] * alr;
	j = i__;
    }
    alr = ap[0] / a[(n1 << 1) - 2];
    ap[1] -= a[(n1 << 1) - 1] * alr;
    gn = *xn - b[n1 - 1] * alr;
    alr = ap[1] / a[(n << 1) - 2];
    gn = (gn - b[n - 1] * alr) / (ap[2] - a[(n << 1) - 1] * alr);
    b[n - 1] = (b[n - 1] - gn * a[(n << 1) - 1]) / a[(n << 1) - 2];
    j = n;
    for (i__ = n1; i__ >= 1; --i__) {
	b[i__ - 1] = (b[i__ - 1] - b[j - 1] * a[(i__ << 1) - 1]) / a[(i__ <<
								    1) - 2];
	j = i__;
    }
    tau[(*i1 << 2) + 2] = *x1;
    is = *i1 + 1;
    ie = *i2 - 1;
    j = 1;
    i__1 = ie;
    for (i__ = is; i__ <= i__1; ++i__) {
	++j;
	tau[(i__ << 2) + 2] = coef[i__ * 5 + 3] * b[j - 2] + coef[i__ * 5 + 4]
	    * b[j - 1] + coef[i__ * 5 + 5] * b[j];
    }
    tau[(*i2 << 2) + 2] = *xn;
    return 0;
}

int TauPCalculator::pdecu_(int *i1, int *i2, double *x0, double *x1, double *xmin,
								int *int__, int *len)
{
    int             i__1,
                    i__2;
    double          d__1;
    static int      i__,
                    j,
                    k,
                    m,
                    n;
    static double   x,
                    h1,
                    h2;
    static int      ie;
    static double   hh,
                    dx;
    static int      is;
    static double   xm,
                    xs,
                    dx2,
                    rnd,
                    axm,
                    sgn;
    if (pdec_.ua[*int__ * 5 - 5] <= 0.) {
	goto L17;
    }
    k = *i1 + 1;
    i__1 = pdec_.ka;
    for (i__ = 1; i__ <= i__1; ++i__) {
	tabc_.pt[k - 1] = pdec_.ua[i__ + *int__ * 5 - 6];
	tabc_.tau[(k << 2) - 4] = pdec_.taua[i__ + *int__ * 5 - 6];
	++k;
    }
    tabc_.pt[k - 1] = tabc_.pt[*i2 - 1];
    tabc_.tau[(k << 2) - 4] = tabc_.tau[(*i2 << 2) - 4];
    goto L19;
L17:
    is = *i1 + 1;
    ie = *i2 - 1;
    xs = *x1;
    i__1 = *i1;
    for (i__ = ie; i__ >= i__1; --i__) {
	x = xs;
	if (i__ != *i1) {
	    goto L12;
	}
	xs = *x0;
	goto L14;
L12:
	h1 = tabc_.pt[i__ - 2] - tabc_.pt[i__ - 1];
	h2 = tabc_.pt[i__] - tabc_.pt[i__ - 1];
	hh = h1 * h2 * (h1 - h2);
	h1 *= h1;
	h2 = -h2 * h2;
	xs = -(h2 * tabc_.tau[((i__ - 1) << 2) - 4] - (h2 + h1) * tabc_.tau[(
		  i__ << 2) - 4] + h1 * tabc_.tau[((i__ + 1) << 2) - 4]) / hh;
L14:
	if ((d__1 = x - xs, ABS (d__1)) <= *xmin) {
	    goto L15;
	}
    }
    *len = *i2;
    return 0;
L15:
    ie = i__;
    if ((d__1 = x - xs, ABS (d__1)) > *xmin * .75 || ie == *i2) {
	goto L16;
    }
    xs = x;
    ++ie;
L16:
    i__1 = (int) ((d__1 = xs - *x0, ABS (d__1)) / *xmin + .8);
    n = MAX (i__1, 1);
    dx = (xs - *x0) / n;
    dx2 = (d__1 = dx * .5, ABS (d__1));
    sgn = d_sign (&c_b10, &dx);
    rnd = 0.;
    if (sgn > 0.) {
	rnd = 1.;
    }
    xm = *x0 + dx;
    k = *i1;
    m = is;
    axm = 1e10;
    i__1 = ie;
    for (i__ = is; i__ <= i__1; ++i__) {
	if (i__ < ie) {
	    goto L8;
	}
	x = xs;
	goto L5;
L8:
	h1 = tabc_.pt[i__ - 2] - tabc_.pt[i__ - 1];
	h2 = tabc_.pt[i__] - tabc_.pt[i__ - 1];
	hh = h1 * h2 * (h1 - h2);
	h1 *= h1;
	h2 = -h2 * h2;
	x = -(h2 * tabc_.tau[((i__ - 1) << 2) - 4] - (h2 + h1) * tabc_.tau[(
		  i__ << 2) - 4] + h1 * tabc_.tau[((i__ + 1) << 2) - 4]) / hh;
L5:
	if (sgn * (x - xm) <= dx2) {
	    goto L2;
	}
	if (k < m) {
	    goto L3;
	}
	i__2 = k;
	for (j = m; j <= i__2; ++j) {
	    tabc_.pt[j - 1] = -1.;
	}
L3:
	m = k + 2;
	k = i__ - 1;
	axm = 1e10;
	xm += dx * (int) ((x - xm - dx2) / dx + rnd);
L2:
	if ((d__1 = x - xm, ABS (d__1)) >= axm) {
	    goto L1;
	}
	axm = (d__1 = x - xm, ABS (d__1));
	k = i__ - 1;
L1:
	;
    }
    if (k < m) {
	goto L9;
    }
    i__1 = k;
    for (j = m; j <= i__1; ++j) {
	tabc_.pt[j - 1] = -1.;
    }
L9:
    k = *i1;
    i__1 = *i2;
    for (i__ = is; i__ <= i__1; ++i__) {
	if (tabc_.pt[i__ - 1] < 0.) {
	    goto L10;
	}
	++k;
	tabc_.pt[k - 1] = tabc_.pt[i__ - 1];
	tabc_.tau[(k << 2) - 4] = tabc_.tau[(i__ << 2) - 4];
L10:
	;
    }
L19:
    *len = k;
    return 0;
}
int TauPCalculator::spfit_(int *jb, int *int__)
{
    static double   dbrnch = 2.5307274;
    static double   x180 = 3.1415927;
    static double   x360 = 6.2831853;
    static double   xmin = .00392403;
    static double   dtol = 1e-6;
    static double   ptol = 2e-6;
    int             i__1;
    float           r__1,
                    r__2;
    double          d__1,
                    d__2;
    static char     disc[3];
    static double   dtau,
                    scpe0,
                    scpe1;
    static int      i__,
                    j,
                    k;
    static int      mncnt,
                    mxcnt,
                    i1,
                    i2;
    static double   p0,
                    p1,
                    x0,
                    x1,
                    hm,
                    pe;
    static int      nn;
    static int      makgrd;
    static int      is;
    static int      newgrd;
    static double   pe0,
                    pe1;
    static double   dpe,
                    dmn,
                    shm,
                    dmx,
                    pmn,
                    thm,
                    spe0,
                    spe1,
                    tau0,
                    tau1;
    i1 = tabc_.jndx[*jb - 1];
    i2 = tabc_.jndx[*jb + 99];
    if (i2 - i1 > 1 || (d__1 = tabc_.pt[i2 - 1] - tabc_.pt[i1 - 1], ABS (
							    d__1)) > ptol) {
	goto L14;
    }
    tabc_.jndx[*jb + 99] = -1;
    return 0;
L14:
    newgrd = FALSE_;
    makgrd = FALSE_;
    if ((d__1 = brkc_.px[*jb + 99] - tabc_.pt[i2 - 1], ABS (d__1)) > dtol) {
	newgrd = TRUE_;
    }
    if (!newgrd) {
	goto L10;
    }
    k = (*int__ - 1) % 2 + 1;
    if (*int__ != brkc_.int0[k - 1]) {
	makgrd = TRUE_;
    }
    if (*int__ > 2) {
	goto L12;
    }
    r__2 = brkc_.odep * (float) 2.;
    r__1 = MAX (r__2, (float) 2.);
    xmin = tabc_.xn * MIN (r__1, (float) 25.);
    pdecu_ (&i1, &i2, &tabc_.xbrn[*jb - 1], &tabc_.xbrn[*jb + 99], &xmin,
	    int__, &i2);
    tabc_.jndx[*jb + 99] = i2;
L12:
    nn = i2 - i1 + 1;
    if (makgrd) {
	tauspl_ (&c__1, &nn, &tabc_.pt[i1 - 1], &brkc_.tcoef[(k * 100 + 1) *
							     5 - 505]);
    }
    fitspl_ (&c__1, &nn, &tabc_.tau[(i1 << 2) - 4], &tabc_.xbrn[*jb - 1], &
	     tabc_.xbrn[*jb + 99], &brkc_.tcoef[(k * 100 + 1) * 5 - 505]);
    brkc_.int0[k - 1] = *int__;
    goto L11;
L10:
    fitspl_ (&i1, &i2, tabc_.tau, &tabc_.xbrn[*jb - 1], &tabc_.xbrn[*jb +
							   99], brkc_.coef);
L11:
    pmn = tabc_.pt[i1 - 1];
    dmn = tabc_.xbrn[*jb - 1];
    dmx = dmn;
    mxcnt = 0;
    mncnt = 0;
    pe = tabc_.pt[i2 - 1];
    p1 = tabc_.pt[i1 - 1];
    tau1 = tabc_.tau[(i1 << 2) - 4];
    x1 = tabc_.tau[(i1 << 2) - 3];
    pe1 = pe - p1;
    spe1 = sqrt ((ABS (pe1)));
    scpe1 = pe1 * spe1;
    j = i1;
    is = i1 + 1;
    i__1 = i2;
    for (i__ = is; i__ <= i__1; ++i__) {
	p0 = p1;
	p1 = tabc_.pt[i__ - 1];
	tau0 = tau1;
	tau1 = tabc_.tau[(i__ << 2) - 4];
	x0 = x1;
	x1 = tabc_.tau[(i__ << 2) - 3];
	dpe = p0 - p1;
	dtau = tau1 - tau0;
	pe0 = pe1;
	pe1 = pe - p1;
	spe0 = spe1;
	spe1 = sqrt ((ABS (pe1)));
	scpe0 = scpe1;
	scpe1 = pe1 * spe1;
	tabc_.tau[(j << 2) - 1] = (dtau * 2. - dpe * (x1 + x0)) / ((scpe1 -
			   scpe0) * .5 - spe1 * 1.5 * spe0 * (spe1 - spe0));
	tabc_.tau[(j << 2) - 2] = (dtau - dpe * x0 - (scpe1 + scpe0 * .5 -
		 pe1 * 1.5 * spe0) * tabc_.tau[(j << 2) - 1]) / (dpe * dpe);
	tabc_.tau[(j << 2) - 3] = (dtau - (pe1 * pe1 - pe0 * pe0) *
		 tabc_.tau[(j << 2) - 2] - (scpe1 - scpe0) * tabc_.tau[(j <<
							     2) - 1]) / dpe;
	tabc_.tau[(j << 2) - 4] = tau0 - scpe0 * tabc_.tau[(j << 2) - 1] -
	    pe0 * (pe0 * tabc_.tau[(j << 2) - 2] + tabc_.tau[(j << 2) -
							     3]);
	tabc_.xlim[(j << 1) - 2] = MIN (x0, x1);
	tabc_.xlim[(j << 1) - 1] = MAX (x0, x1);
	if (tabc_.xlim[(j << 1) - 2] >= dmn) {
	    goto L5;
	}
	dmn = tabc_.xlim[(j << 1) - 2];
	pmn = tabc_.pt[j - 1];
	if (x1 < x0) {
	    pmn = tabc_.pt[i__ - 1];
	}
L5:
	s_copy (disc, " ", 3L, 1L);
	if ((d__1 = tabc_.tau[(j << 2) - 2], ABS (d__1)) <= 1e-30) {
	    goto L4;
	}
	shm = tabc_.tau[(j << 2) - 1] * -.375 / tabc_.tau[(j << 2) - 2];
	hm = shm * shm;
	if (shm <= 0. || (hm <= pe1 || hm >= pe0)) {
	    goto L4;
	}
	thm = tabc_.tau[(j << 2) - 3] + shm * (shm * 2. * tabc_.tau[(j << 2)
				      - 2] + tabc_.tau[(j << 2) - 1] * 1.5);
	d__1 = tabc_.xlim[(j << 1) - 2];
	tabc_.xlim[(j << 1) - 2] = MIN (d__1, thm);
	d__1 = tabc_.xlim[(j << 1) - 1];
	tabc_.xlim[(j << 1) - 1] = MAX (d__1, thm);
	if (thm >= dmn) {
	    goto L6;
	}
	dmn = thm;
	pmn = pe - hm;
L6:
	s_copy (disc, "MAX", 3L, 3L);
	if (tabc_.tau[(j << 2) - 1] < 0.) {
	    s_copy (disc, "MIN", 3L, 3L);
	}
	if (s_cmp (disc, "MAX", 3L, 3L) == 0) {
	    ++mxcnt;
	}
	if (s_cmp (disc, "MIN", 3L, 3L) == 0) {
	    ++mncnt;
	}
L4:
	d__1 = dmx, d__2 = tabc_.xlim[(j << 1) - 1];
	dmx = MAX (d__1, d__2);
	j = i__;
    }
    tabc_.xbrn[*jb - 1] = dmn;
    tabc_.xbrn[*jb + 99] = dmx;
    tabc_.xbrn[*jb + 199] = pmn;
    tabc_.idel[*jb - 1] = 1;
    tabc_.idel[*jb + 99] = 1;
    if (tabc_.xbrn[*jb - 1] > x180) {
	tabc_.idel[*jb - 1] = 2;
    }
    if (tabc_.xbrn[*jb + 99] > x180) {
	tabc_.idel[*jb + 99] = 2;
    }
    if (tabc_.xbrn[*jb - 1] > x360) {
	tabc_.idel[*jb - 1] = 3;
    }
    if (tabc_.xbrn[*jb + 99] > x360) {
	tabc_.idel[*jb + 99] = 3;
    }
    if (*int__ > 2) {
	goto L1;
    }
    s_copy (pcdc_.phcd + ((*jb - 1) << 3), pcdc_.phcd + ((*jb - 1) << 3), 8L, 1L);
    i__ = *jb;
    i__1 = brkc_.nbrn;
    for (j = 1; j <= i__1; ++j) {
	i__ = i__ % brkc_.nbrn + 1;
	if (s_cmp (pcdc_.phcd + ((i__ - 1) << 3), pcdc_.phcd + ((*jb - 1) << 3),
	       1L, 8L) == 0 && *(unsigned char *) &pcdc_.phcd[((i__ - 1) << 3)
		  + 1] != 'P' && (pe >= brkc_.px[i__ - 1] && pe <= brkc_.px[
							       i__ + 99])) {
	    goto L9;
	}
    }
    goto L1;
L9:
    s_copy (pcdc_.phcd + ((*jb - 1) << 3), pcdc_.phcd + ((i__ - 1) << 3), 8L, 8L);
    if ((d__1 = tabc_.pt[i2 - 1] - tabc_.pt[tabc_.jndx[i__ - 1] - 1], ABS (
							   d__1)) <= dtol) {
	s_copy (pcdc_.phcd + ((*jb - 1) << 3), pcdc_.phcd + ((i__ - 2) << 3), 8L,
		8L);
    }
L1:
    if (tabc_.dbrn[*jb - 1] <= 0.) {
	goto L3;
    }
    tabc_.dbrn[*jb - 1] = dmx;
    tabc_.dbrn[*jb + 99] = dbrnch;
    goto L15;
L3:
L15:
    if (mxcnt > mncnt || mncnt > mxcnt + 1) {
	char            s[9];
	strncpy (s, pcdc_.phcd + ((*jb - 1) << 3), 8);
	s[8] = 0;
	elog_log (0, "Bad interpolation on %s\n", s);
    }
    return 0;
}

int TauPCalculator::depset_(float *dep, float *usrc)
{
    int             i__1,
                    i__2,
                    i__3;
    float           r__1,
                    r__2;
    static float    rdep;
    static int      i__,
                    j,
                    k;
    static int      ind;
    static int      dop,
                    dos;
    static int      nph,
                    int__;
    --usrc;
    if (MAX (*dep, (float) .011) != brkc_.odep) {
	goto L1;
    }
    dop = FALSE_;
    dos = FALSE_;
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!prtflc_.segmsk[i__ - 1] || brkc_.iidx[i__ - 1] > 0) {
	    goto L2;
	}
	if ((i__2 = brkc_.nafl[i__ - 1], ABS (i__2)) <= 1) {
	    dop = TRUE_;
	}
	if ((i__2 = brkc_.nafl[i__ - 1], ABS (i__2)) >= 2) {
	    dos = TRUE_;
	}
L2:
	;
    }
    if (!dop && !dos) {
	return 0;
    }
    goto L3;
L1:
    brkc_.nph0 = 0;
    brkc_.int0[0] = 0;
    brkc_.int0[1] = 0;
    tabc_.mbr1 = brkc_.nbrn + 1;
    tabc_.mbr2 = 0;
    dop = FALSE_;
    dos = FALSE_;
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!prtflc_.segmsk[i__ - 1]) {
	    goto L4;
	}
	if ((i__2 = brkc_.nafl[i__ - 1], ABS (i__2)) <= 1) {
	    dop = TRUE_;
	}
	if ((i__2 = brkc_.nafl[i__ - 1], ABS (i__2)) >= 2) {
	    dos = TRUE_;
	}
L4:
	;
    }
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (brkc_.nafl[i__ + 29] > 0 || brkc_.odep < (float) 0.) {
	    goto L5;
	}
	ind = brkc_.nafl[i__ - 1];
	k = 0;
	i__2 = brkc_.indx[i__ + 29];
	for (j = brkc_.indx[i__ - 1]; j <= i__2; ++j) {
	    ++k;
	    tabc_.pt[j - 1] = brkc_.tp[k + ind * 100 - 101];
	}
L5:
	brkc_.iidx[i__ - 1] = -1;
    }
    i__1 = brkc_.nbrn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	tabc_.jndx[i__ + 99] = -1;
    }
    if (brkc_.ki <= 0) {
	goto L7;
    }
    i__1 = brkc_.ki;
    for (i__ = 1; i__ <= i__1; ++i__) {
	j = brkc_.kk[i__ - 1];
	tabc_.pt[j - 1] = brkc_.pk[i__ - 1];
    }
    brkc_.ki = 0;
L7:
    brkc_.odep = MAX (*dep, (float) .011);
    rdep = *dep;
    if (rdep < (float) .011) {
	rdep = (float) 0.;
    }
    r__2 = (float) 1. - rdep * tabc_.xn;
    r__1 = log ((MAX (r__2, (float) 1e-30)));
    brkc_.zs = MIN (r__1, (float) 0.);
    tabc_.hn = (float) 1. / (tabc_.pn * ((float) 1. - rdep * tabc_.xn));
L3:
    if (brkc_.nph0 > 1) {
	goto L12;
    }
    if (dop) {
	depcor_ (&c__1);
    }
    if (dos) {
	depcor_ (&c__2);
    }
    goto L14;
L12:
    if (dos) {
	depcor_ (&c__2);
    }
    if (dop) {
	depcor_ (&c__1);
    }
L14:
    j = 1;
    i__1 = brkc_.nseg;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (!prtflc_.segmsk[i__ - 1]) {
	    goto L9;
	}
	nph = (i__2 = brkc_.nafl[i__ - 1], ABS (i__2));
	if (brkc_.iidx[i__ - 1] > 0 || (brkc_.msrc[nph - 1] <= 0 &&
		brkc_.nafl[i__ - 1] > 0)) {
	    goto L9;
	}
	brkc_.iidx[i__ - 1] = 1;
	if (brkc_.nafl[i__ + 29] <= 0) {
	    int__ = brkc_.nafl[i__ - 1];
	}
	if (brkc_.nafl[i__ + 29] > 0 && brkc_.nafl[i__ + 29] == (i__2 =
					 brkc_.nafl[i__ - 1], ABS (i__2))) {
	    int__ = brkc_.nafl[i__ + 29] + 2;
	}
	if (brkc_.nafl[i__ + 29] > 0 && brkc_.nafl[i__ + 29] != (i__2 =
					 brkc_.nafl[i__ - 1], ABS (i__2))) {
	    int__ = (i__3 = brkc_.nafl[i__ - 1], ABS (i__3)) + 4;
	}
	if (brkc_.nafl[i__ + 29] > 0 && brkc_.nafl[i__ + 29] != brkc_.nafl[
								i__ + 59]) {
	    int__ = brkc_.nafl[i__ + 29] + 6;
	}
L11:
	if (tabc_.jndx[j - 1] >= brkc_.indx[i__ - 1]) {
	    goto L10;
	}
	++j;
	goto L11;
L10:
	tabc_.idel[j + 199] = brkc_.nafl[i__ - 1];
	spfit_ (&j, &int__);
	tabc_.mbr1 = MIN (tabc_.mbr1, j);
	tabc_.mbr2 = MAX (tabc_.mbr2, j);
	if (j >= brkc_.nbrn) {
	    goto L9;
	}
	++j;
	if (brkc_.jidx[j - 1] <= brkc_.indx[i__ + 29] && tabc_.jndx[j + 99]
		> 0) {
	    goto L10;
	}
L9:
	;
    }
    usrc[1] = tabc_.us[0] / tabc_.pn;
    usrc[2] = tabc_.us[1] / tabc_.pn;
    return 0;
}
int TauPCalculator::findtt_(int *jb, double *x0, int *max__, int *n, 
        float *tt, float *dtdd, float *dtdh, float *dddp, char *phnm, int phnm_len)
{
    const double tol(3e-6);
    const double deps(1e-10);
    char           *a__1[2];
    int             i__1,
                    i__2,
                    i__3[2];
    float           r__1;
    double          d__1;
    static double   delp;
    static float    dsgn,
                    hsgn;
    static int      i__,
                    j;
    static double   x;
    static int      i3;
    static double   p0,
                    p1;
    static int      ie,
                    ij;
    static double   dp;
    static int      jj,
                    in,
                    ln,
                    is;
    static double   ps;
    static double   arg;
    static float    dpn;
    static double   dps;
    static int      nph;
    --x0;
    phnm -= phnm_len;
    --dddp;
    --dtdh;
    --dtdd;
    --tt;
    nph = (i__1 = tabc_.idel[*jb + 199], ABS (i__1));
    hsgn = i_sign (&c__1, &tabc_.idel[*jb + 199]) * tabc_.hn;
    dsgn = pow_ri (&c_b3, &tabc_.idel[*jb - 1]) * tabc_.dn;
    dpn = (float) -1. / tabc_.tn;
    i__1 = tabc_.idel[*jb + 99];
    for (ij = tabc_.idel[*jb - 1]; ij <= i__1; ++ij) {
	x = x0[ij];
	dsgn = -dsgn;
	if (x < tabc_.xbrn[*jb - 1] || x > tabc_.xbrn[*jb + 99]) {
	    goto L12;
	}
	j = tabc_.jndx[*jb - 1];
	is = j + 1;
	ie = tabc_.jndx[*jb + 99];
	i__2 = ie;
	for (i__ = is; i__ <= i__2; ++i__) {
	    if (x <= tabc_.xlim[(j << 1) - 2] || x > tabc_.xlim[(j << 1) -
								1]) {
		goto L8;
	    }
	    p0 = tabc_.pt[ie - 1] - tabc_.pt[j - 1];
	    p1 = tabc_.pt[ie - 1] - tabc_.pt[i__ - 1];
	    d__1 = tol * (tabc_.pt[i__ - 1] - tabc_.pt[j - 1]);
	    delp = MAX (d__1, .001);
	    if ((d__1 = tabc_.tau[(j << 2) - 2], ABS (d__1)) > 1e-30) {
		goto L2;
	    }
	    dps = (x - tabc_.tau[(j << 2) - 3]) / (tabc_.tau[(j << 2) - 1] *
						   1.5);
	    d__1 = dps * dps;
	    dp = d_sign (&d__1, &dps);
	    if (dp < p1 - delp || dp > p0 + delp) {
		goto L9;
	    }
	    if (*n >= *max__) {
		goto L13;
	    }
	    ++(*n);
	    ps = tabc_.pt[ie - 1] - dp;
	    tt[*n] = tabc_.tn * (tabc_.tau[(j << 2) - 4] + dp * (tabc_.tau[
		   (j << 2) - 3] + dps * tabc_.tau[(j << 2) - 1]) + ps * x);
	    dtdd[*n] = dsgn * ps;
	    dtdh[*n] = hsgn * sqrt ((r__1 = (float) (tabc_.us[nph - 1] *
				 tabc_.us[nph - 1] - ps * ps), ABS (r__1)));
	    d__1 = ABS (dps);
	    dddp[*n] = dpn * .75 * tabc_.tau[(j << 2) - 1] / MAX (d__1, deps);
	    s_copy (phnm + *n * phnm_len, pcdc_.phcd + ((*jb - 1) << 3),
		    phnm_len, 8L);
	    in = i_indx (phnm + *n * phnm_len, "ab", phnm_len, 2L);
	    if (in <= 0) {
		goto L8;
	    }
	    if (ps <= tabc_.xbrn[*jb + 199]) {
		s_copy (phnm + (*n * phnm_len + (in - 1)), "bc", phnm_len - (
							       in - 1), 2L);
	    }
	    goto L8;
    L2:
	    for (jj = 1; jj <= 2; ++jj) {
		switch ((int) jj) {
		case 1:
		    goto L5;
		case 2:
		    goto L6;
		}
	L5:
		arg = tabc_.tau[(j << 2) - 1] * 9. * tabc_.tau[(j << 2) - 1]
		    + tabc_.tau[(j << 2) - 2] * 32. * (x - tabc_.tau[(
							      j << 2) - 3]);
		d__1 = sqrt ((ABS (arg)));
		dps = -(tabc_.tau[(j << 2) - 1] * 3. + d_sign (&d__1, &
			tabc_.tau[(j << 2) - 1])) / (tabc_.tau[(j << 2) - 2]
						     * 8.);
		d__1 = dps * dps;
		dp = d_sign (&d__1, &dps);
		goto L7;
	L6:
		dps = (tabc_.tau[(j << 2) - 3] - x) / (tabc_.tau[(j << 2) -
							     2] * 2. * dps);
		d__1 = dps * dps;
		dp = d_sign (&d__1, &dps);
	L7:
		if (dp < p1 - delp || dp > p0 + delp) {
		    goto L4;
		}
		if (*n >= *max__) {
		    goto L13;
		}
		++(*n);
		ps = tabc_.pt[ie - 1] - dp;
		tt[*n] = tabc_.tn * (tabc_.tau[(j << 2) - 4] + dp * (
			 tabc_.tau[(j << 2) - 3] + dp * tabc_.tau[(j << 2) -
			      2] + dps * tabc_.tau[(j << 2) - 1]) + ps * x);
		dtdd[*n] = dsgn * ps;
		dtdh[*n] = hsgn * sqrt ((r__1 = (float) (tabc_.us[nph - 1] *
				 tabc_.us[nph - 1] - ps * ps), ABS (r__1)));
		d__1 = ABS (dps);
		dddp[*n] = dpn * (tabc_.tau[(j << 2) - 2] * 2. + tabc_.tau[(
				     j << 2) - 1] * .75 / MAX (d__1, deps));
		s_copy (phnm + *n * phnm_len, pcdc_.phcd + ((*jb - 1) << 3),
			phnm_len, 8L);
		in = i_indx (phnm + *n * phnm_len, "ab", phnm_len, 2L);
		if (in <= 0) {
		    goto L4;
		}
		if (ps <= tabc_.xbrn[*jb + 199]) {
		    s_copy (phnm + (*n * phnm_len + (in - 1)), "bc", phnm_len
			    - (in - 1), 2L);
		}
	L4:
		;
	    }
    L9:
    L8:
	    j = i__;
	}
L12:
	if (x < tabc_.dbrn[*jb - 1] || x > tabc_.dbrn[*jb + 99]) {
	    goto L10;
	}
	if (*n >= *max__) {
	    goto L13;
	}
	j = tabc_.jndx[*jb - 1];
	i__ = tabc_.jndx[*jb + 99];
	dp = tabc_.pt[i__ - 1] - tabc_.pt[j - 1];
	dps = sqrt ((ABS (dp)));
	++(*n);
	tt[*n] = tabc_.tn * (tabc_.tau[(j << 2) - 4] + dp * (tabc_.tau[(j
		<< 2) - 3] + dp * tabc_.tau[(j << 2) - 2] + dps * tabc_.tau[
				      (j << 2) - 1]) + tabc_.pt[j - 1] * x);
	dtdd[*n] = dsgn * (float) tabc_.pt[j - 1];
	dtdh[*n] = hsgn * sqrt ((r__1 = (float) (tabc_.us[nph - 1] * tabc_.us[
		nph - 1] - tabc_.pt[j - 1] * tabc_.pt[j - 1]), ABS (r__1)));
	dddp[*n] = dpn * (tabc_.tau[(j << 2) - 2] * 2. + tabc_.tau[(j << 2)
					      - 1] * .75 / MAX (dps, deps));
	ln = i_indx (pcdc_.phcd + ((*jb - 1) << 3), " ", 8L, 1L) - 1;
	if (ln <= 0) {
	    ln = i_len (pcdc_.phcd + ((*jb - 1) << 3), 8L);
	}
	i__3[0] = ln, a__1[0] = pcdc_.phcd + ((*jb - 1) << 3);
	i__3[1] = 4, a__1[1] = "diff";
	s_cat (phnm + *n * phnm_len, a__1, i__3, &c__2, phnm_len);
L10:
	;
    }
    return 0;
L13:
    elog_log (0, "More than %d arrivals found.", i3);
    return 0;
}

int r4sort_(int *n, float *rkey, int *iptr)
{
    int             i__1;
    static int      i__,
                    j,
                    k,
                    l,
                    m;
    static float    r__;
    static int      ib,
                    ij,
                    il[10],
                    kk,
                    iu[10],
                    it;
    static float    tmpkey;
    --iptr;
    --rkey;
    if (*n <= 0) {
	return 0;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	iptr[i__] = i__;
    }
    if (*n <= 1) {
	return 0;
    }
    r__ = (float) .375;
    m = 1;
    i__ = 1;
    j = *n;
L5:
    if (i__ >= j) {
	goto L70;
    }
L10:
    k = i__;
    if (r__ > (float) .58984375) {
	goto L11;
    }
    r__ += (float) .0390625;
    goto L12;
L11:
    r__ += (float) -.21875;
L12:
    ij = i__ + (j - i__) * r__;
    if (rkey[iptr[i__]] <= rkey[iptr[ij]]) {
	goto L20;
    }
    it = iptr[ij];
    iptr[ij] = iptr[i__];
    iptr[i__] = it;
L20:
    l = j;
    if (rkey[iptr[j]] >= rkey[iptr[ij]]) {
	goto L39;
    }
    it = iptr[ij];
    iptr[ij] = iptr[j];
    iptr[j] = it;
    if (rkey[iptr[i__]] <= rkey[iptr[ij]]) {
	goto L39;
    }
    it = iptr[ij];
    iptr[ij] = iptr[i__];
    iptr[i__] = it;
L39:
    tmpkey = rkey[iptr[ij]];
    goto L40;
L30:
    it = iptr[l];
    iptr[l] = iptr[k];
    iptr[k] = it;
L40:
    --l;
    if (rkey[iptr[l]] > tmpkey) {
	goto L40;
    }
L50:
    ++k;
    if (rkey[iptr[k]] < tmpkey) {
	goto L50;
    }
    if (k <= l) {
	goto L30;
    }
    if (l - i__ <= j - k) {
	goto L60;
    }
    il[m - 1] = i__;
    iu[m - 1] = l;
    i__ = k;
    ++m;
    goto L80;
L60:
    il[m - 1] = k;
    iu[m - 1] = j;
    j = l;
    ++m;
    goto L80;
L70:
    --m;
    if (m == 0) {
	return 0;
    }
    i__ = il[m - 1];
    j = iu[m - 1];
L80:
    if (j - i__ >= 11) {
	goto L10;
    }
    if (i__ == 1) {
	goto L5;
    }
    --i__;
L90:
    ++i__;
    if (i__ == j) {
	goto L70;
    }
    if (rkey[iptr[i__]] <= rkey[iptr[i__ + 1]]) {
	goto L90;
    }
    k = i__;
    kk = k + 1;
    ib = iptr[kk];
L100:
    iptr[kk] = iptr[k];
    kk = k;
    --k;
    if (rkey[ib] < rkey[iptr[k]]) {
	goto L100;
    }
    iptr[kk] = ib;
    goto L90;
}
/* Note on C++ conversion.  Name collision was created in difference
in scope rules when this had to be assimilated as a private method in
the TauPCalculator object.   The C interface routines from Quinlan 
refer to the output of this procedure by the same names previously here:  tt, dtdd,
dtdd, and dtdh.  Here is what I started from as the declaration of this function.

int TauPCalculator::trtm_(float *delta, int *max__, int *n, 
		float *tt, float *dtdd, float *dtdh, float *dddp,
		char *phnm, int phnm_len)
		
These names are local only to this procedure so the workaround was to 
rename them with the signature below and then edit the code for this procedure.

GLP:  Dec 2014
*/
int TauPCalculator::trtm_(float *delta, int *max__, int *n, 
		float *ttout, float *dtddout, float *dtdhout, float *dddpout,
		char *phnm, int phnm_len)
{
    static double   cn = .017453292519943296;
    static double   dtol = 1e-6;
    static float    atol = (float) .005;
    static double   pi = 3.1415926535897932;
    static double   pi2 = 6.2831853071795865;
    int             i__1;
    float           r__1;
    double          d__1,
                    d__2;
    static char     ctmp[8 * 60];
    static int      iptr[60],
                    i__,
                    j,
                    k;
    static double   x[3];
    static float    tmp[240];
    phnm -= phnm_len;
    --dddpout;
    --dtdhout;
    --dtddout;
    --ttout;
    *n = 0;
    if (tabc_.mbr2 <= 0) {
	return 0;
    }
    d__2 = (d__1 = cn * *delta, ABS (d__1));
    x[0] = d_mod (&d__2, &pi2);
    if (x[0] > pi) {
	x[0] = pi2 - x[0];
    }
    x[1] = pi2 - x[0];
    x[2] = x[0] + pi2;
    if (ABS (x[0]) > dtol) {
	goto L9;
    }
    x[0] = dtol;
    x[2] = -10.;
L9:
    if ((d__1 = x[0] - pi, ABS (d__1)) > dtol) {
	goto L7;
    }
    x[0] = pi - dtol;
    x[1] = -10.;
L7:
    i__1 = tabc_.mbr2;
    for (j = tabc_.mbr1; j <= i__1; ++j) {
	if (tabc_.jndx[j + 99] > 0) {
	    findtt_ (&j, x, max__, n, tmp, &tmp[60], &tmp[120], &tmp[180],
		     ctmp, 8L);
	}
    }
    if ((i__1 = *n - 1) < 0) {
	goto L3;
    } else if (i__1 == 0) {
	goto L4;
    } else {
	goto L5;
    }
L4:
    iptr[0] = 1;
    goto L6;
L5:
    r4sort_ (n, tmp, iptr);
L6:
    k = 0;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	j = iptr[i__ - 1];
	if (k <= 0) {
	    goto L8;
	}
	if (s_cmp (phnm + k * phnm_len, ctmp + ((j - 1) << 3), phnm_len, 8L) ==
		0 && (r__1 = ttout[k] - tmp[j - 1], ABS (r__1)) <= atol) {
	    goto L2;
	}
L8:
	++k;
	ttout[k] = tmp[j - 1];
	dtddout[k] = tmp[j + 59];
	dtdhout[k] = tmp[j + 119];
	dddpout[k] = tmp[j + 179];
	s_copy (phnm + k * phnm_len, ctmp + ((j - 1) << 3), phnm_len, 8L);
L2:
	;
    }
    *n = k;
L3:
    return 0;
}
