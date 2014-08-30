#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>
#include <math.h>

#include "tttaup.h"
#include "stock.h"
#include "coords.h"

#define TBL_DIR        "tables/taup_ttimes/"
#define TBL_TBL         "iasp91"
#define	SIZE_PHCD	16
#define	MAXPHASES		60

static char tblpath[MAXPATHLEN] = "";

char *tt_strf2c();

int
tt_taup_set_table (table_path)

char *table_path;

{
    char fname[MAXPATHLEN], *tablebase, tablename[MAXPATHLEN];

    if ( table_path !=  0 && *table_path == '/' ) 
	strcpy ( tblpath, table_path ) ; 
    else {
	if ( table_path != 0 && *table_path != 0 ) 
	    tablebase = table_path ; 
	else if ( ( tablebase= getenv("TAUP_TABLE")) == NULL ) 
	    tablebase = TBL_TBL ; 
	if ( *tablebase != '/' ) 
	    strcpy ( tablename, TBL_DIR ) ; 
	strcat ( tablename, tablebase ) ; 
	strcat ( tablename, ".hed" ) ; 
	table_path = datafile ( "TAUP_PATH", tablename ) ;
	if ( table_path != 0 ) {
	    strcpy ( tblpath, table_path ) ; 
	    tblpath[strlen(tblpath)-4] = 0 ; 
	    free(table_path) ; 
	} else { 
	    complain ( 0, "Can't find travel time table for '%s'\n", 
		tablename ) ;
	    return -1 ;
	}
    }

    strcpy ( fname, tblpath ) ; 
    strcat ( fname, ".hed" ) ; 
    return (access(fname, R_OK));
}

int
tt_taup_get_modname (modname)

char *modname;

{
	int i;

	if (!tblpath[0]) 
	    tt_taup_set_table ((char *) 0) ;
	for (i=strlen(tblpath); i>=0; i--) if (tblpath[i] == '/') {i++;break;}
	strcpy (modname, &tblpath[i]);
	return 0 ; 
}


int
taup_setup (model, phases)

char *model ;
char *phases;
{
	static int ttopen=0;
	static int one=1;
	static int prnt[3] = {0, 0, 0};
	static int nn;
	static char pcntl[MAXPHASES][SIZE_PHCD];
	static char phass[512];
	static char old_phases[512] ;
	char *phase;
	int k;

	strcpy (phass, phases);
	strcat (phass, ",");
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		if (!phase[0]) continue;
		strcpy (&pcntl[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) pcntl[nn][k] = ' ';
		nn++;
	}

	if ( tt_taup_set_table (model) != 0 )
	    return -4 ;

	if (!tblpath[0])
	    return -4 ;

	tabin_ (&one, tblpath ) ; 
	brnset_ (&nn, pcntl, prnt, SIZE_PHCD);
	ttopen = 1;

	return (0);
}

int
tt_taup_set_phases (phases)

char *    phases;

{
	static int ttopen=0;
	static int one=1;
	static int prnt[3] = {0, 0, 0};
	static int nn;
	static char pcntl[MAXPHASES][SIZE_PHCD];
	static char phass[512];
	static char old_phases[512] ;
	char *phase;
	int k;

	if ( strcmp(old_phases, phases) == NULL ) 
	    return 1 ;
	strcpy ( old_phases, phases) ; 
	strcpy (phass, phases);
	strcat (phass, ",");
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		if (!phase[0]) continue;
		strcpy (&pcntl[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) pcntl[nn][k] = ' ';
		nn++;
	}
	if (!tblpath[0]) 
	    tt_taup_set_table ((char *) 0) ;
	if (!tblpath[0])
	    return 0 ;

	tabin_ (&one, tblpath) ;
	brnset_ (&nn, pcntl, prnt, SIZE_PHCD);
	ttopen = 1;
	return (1);
}

int
tt_taup_set_event_depth (edepth)

double                     edepth;

{
	static float depth;
	static float usrc[2];

	depth = edepth;
	if (depth < 0.001) depth = 0.001;
	depset_ (&depth, usrc);
	return (1);
}

int
tt_taup (elat, elon, slat, slon, selev, vp_cor, vs_cor, 
		nph, times, phs)

double     elat;
double           elon;
double                 slat;
double                       slon;
double                             selev;
double                                    vp_cor;
double                                            vs_cor;
int *          nph;
double **           times;
char ***                    phs;

{
	static int maxphases;
	static int nphase;
	static float del, tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
	static char phcd[MAXPHASES][SIZE_PHCD];
	static char *phss[MAXPHASES];
	static double tts[MAXPHASES];
	int k, l;

	*nph = 0;
	maxphases = MAXPHASES;

	del = ddistance (elat, elon, slat, slon);
	trtm_ (&del, &maxphases, &nphase, &tt[(*nph)], &dtdd[(*nph)],
			&dtdh[(*nph)], &dddp[(*nph)], &(phcd[(*nph)]), SIZE_PHCD);
	if (nphase <= 0) return (1);
	for (k=(*nph); k<(*nph)+nphase; k++) {
		for (l=0; l<SIZE_PHCD-1; l++) if (phcd[k][l] == ' ') break;
		phcd[k][l] = '\0';
	}
	(*nph) += nphase;
	for (k=0; k<(*nph); k++) {
		phss[k] = &(phcd[k][0]);
		tts[k] = tt[k];
	}
	*times = tts;
	*phs = phss;
	return (1);
}

int
tt_taup_p (elat, elon, slat, slon, selev, vp_cor, vs_cor, 
		nph, times, ps, dtdhs, phs)

double     elat;
double           elon;
double                 slat;
double                       slon;
double                             selev;
double                                    vp_cor;
double                                            vs_cor;
int *          nph;
double **           times;
double **           ps;
double **           dtdhs;
char ***                    phs;

{
	static int maxphases;
	static int nphase;
	static float del, tt[MAXPHASES], dtdd[MAXPHASES], dtdh[MAXPHASES], dddp[MAXPHASES];
	static char phcd[MAXPHASES][SIZE_PHCD];
	static char *phss[MAXPHASES];
	static double tts[MAXPHASES];
	static double pss[MAXPHASES];
	static double dtdhss[MAXPHASES];
	int k, l;

	*nph = 0;
	maxphases = MAXPHASES;
	del = ddistance (elat, elon, slat, slon);
	trtm_ (&del, &maxphases, &nphase, tt, dtdd,
			dtdh, dddp, phcd, SIZE_PHCD);
	if (nphase <= 0) return (1);
	for (k=(*nph); k<(*nph)+nphase; k++) {
		for (l=0; l<SIZE_PHCD-1; l++) if (phcd[k][l] == ' ') break;
		phcd[k][l] = '\0';
	}
	(*nph) += nphase;
	for (k=0; k<(*nph); k++) {
		phss[k] = &(phcd[k][0]);
		tts[k] = tt[k];
		pss[k] = dtdd[k];
		dtdhss[k] = dtdh[k];
	}
	*times = tts;
	*phs = phss;
	*ps = pss;
	*dtdhs = dtdhss;
	return (1);
}

void
tt_taup_set_table_ (table_path, len_table_path)

char *table_path;
int len_table_path;

{
	tt_taup_set_table (tt_strf2c(table_path, len_table_path));
}

void
tt_taup_set_phases_ (phases, len_phases)

char *phases;
int len_phases;

{
	tt_taup_set_phases (tt_strf2c(phases, len_phases));
}

void
tt_taup_set_event_depth_ (edepth)

double *edepth;

{
	tt_taup_set_event_depth (*edepth);
}

void
tt_taup_ (elat, elon, slat, slon, selev, vp_cor, vs_cor, 
		nph, times, phs, len_phs)

double     *elat;
double           *elon;
double                 *slat;
double                       *slon;
double                             *selev;
double                                    *vp_cor;
double                                            *vs_cor;
int *          nph;
double *           times;
char *                    phs;
int			len_phs;

{
	int nphi;
	double *timesi;
	char **phsi;
	int nmax;
	int i, j;

	nmax = *nph;
	*nph = 0;
	tt_taup (*elat, *elon, *slat, *slon, *selev, *vp_cor, *vs_cor, 
		&nphi, &timesi, &phsi);
	if (nphi < 1) return;
	if (nphi > nmax) nphi = nmax;
	for (i=0; i<nphi; i++) {
		times[i] = timesi[i];
		for (j=0; j<len_phs; j++) {
			phs[j+i*len_phs] = ' ';
			if (j < strlen(phsi[i])) phs[j+i*len_phs] = phsi[i][j];
		}
	}
	*nph = nphi;
}

char *
tt_strf2c (string, len)

char *string;
int len;

{
	static char out[512];
	int i, j, k;

	out[0] = '\0';
	if (len < 1) return (out);
	for (i=0; i<len; i++) if (string[i] != ' ') break;
	if (i == len) return (out);
	for (j=len; j>0; j--) if (string[j-1] != ' ') break;
	if (j == 0) return (out);
	for (k=0; i<j; i++) out[k++] = string[i];
	out[k] = '\0';
	return (out);
}

/* $Id: tt_taup.c,v 1.1.1.1 1997/04/12 04:18:48 danq Exp $ */
