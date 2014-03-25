#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#undef abs
#include "stock.h"

#define TRUE_ (1)
#define FALSE_ (0)
#define VOID void

#include "elog.h"

#define jsrc	150
#define jseg	30
#define jbrn	100
#define jout	2250
#define jtsm 	350
#define jxsm	jbrn
#define jbrnu	jbrn
#define jbrna	jbrn
#define jrec	(jtsm+jxsm)
#define jtsm0	(jtsm+1)

struct Brkc {
    double zs, pk[30], pu[702]	/* was [351][2] */, pux[200]	/* 
	    was [100][2] */, tauu[700]	/* was [350][2] */, xu[200]	/* 
	    was [100][2] */, px[200]	/* was [100][2] */, xt[200]	/* 
	    was [100][2] */, taut[2250], coef[11250]	/* was [5][2250] */, 
	    tauc[350], xc[100], tcoef[1000]	/* was [5][100][2] */, tp[200]
	    	/* was [100][2] */;
    float odep, fcs[90]	/* was [30][3] */;
    int nin, nph0, int0[2], ki, msrc[2], isrc[2], nseg, nbrn, ku[2], km[2]
	    , nafl[90]	/* was [30][3] */, indx[60]	/* was [30][2] */, 
	    kndx[60]	/* was [30][2] */, iidx[30], jidx[100], kk[30];
} ;

struct Pcdc {
    char phcd[800];
} ;

struct Prtflc {
    int segmsk[30], prnt[2];
} ;

struct Umdc {
    double pm[300]	/* was [150][2] */, zm[300]	/* was [150][2] */;
    int ndex[300]	/* was [150][2] */, mt[2];
} ;

struct Tabc {
    double us[2], pt[2250], tau[9000]	/* was [4][2250] */, xlim[
	    4500]	/* was [2][2250] */, xbrn[300]	/* was [100][3] */, 
	    dbrn[200]	/* was [100][2] */;
    float xn, pn, tn, dn, hn;
    int jndx[200]	/* was [100][2] */, idel[300]	/* was [100][3] */, 
	    mbr1, mbr2;
} ;

struct Pdec {
    double ua[10]	/* was [5][2] */, taua[10]	/* was [5][2] */;
    float deplim;
    int ka;
} ;

#if 0
extern char *tbl_data ;
extern int tbl_data_size ;
extern int tbl_record_len ;
extern int tbl_maxrec ;
#endif

/* $Id: _taup.h,v 1.1.1.1 1997/04/12 04:18:47 danq Exp $ */
