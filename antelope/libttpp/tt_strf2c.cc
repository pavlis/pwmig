/* Retain these for now - will delete much of this eventually.   
Issue is if this file should be separate or just assimilated into the 
TauPCalculator file. 

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>
#include <math.h>

#include "tttaup.h"
#include "stock.h"
#include "coords.h"
*/

#define TBL_DIR        "tables/taup_ttimes/"
#define TBL_TBL         "iasp91"
#define	SIZE_PHCD	16
#define	MAXPHASES		60

static char tblpath[MAXPATHLEN] = "";

char *tt_strf2c();

int TauPCalculator::tt_taup_set_table(const char *table_path)
{
    string base_error("TauPCalculator::tt_taup_set_table:  ");
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
            throw SeisppError(base_error
			+ "Cannot find travel time table for "
			+ tablename);
	}
    }

    strcpy ( fname, tblpath ) ; 
    strcat ( fname, ".hed" ) ; 
    if (access(fname, R_OK))
	throw SeisppError(base_error+"Permission denied for file"
		+ fname);
    /* null return always - legacy of parentage of this.  This procedure
    could be void but for now will leave it this way.*/
    return(0);
}

int TauPCalculator::taup_get_modname(char *modname)
{
	int i;

        if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_get_modname:  ")
			+ "Travel time data path not set.  Initialization problem");
	for (i=strlen(tblpath); i>=0; i--) if (tblpath[i] == '/') {i++;break;}
	strcpy (modname, &tblpath[i]);
	return 0 ; 
}


int TauPCalculator::taup_setup(const char *model, const char *phases)
{
	/* This was the original C approach - now these are part of the object.
	static int ttopen=0;
	static int one=1;
	static int prnt[3] = {0, 0, 0};
	static int nn;
	static char pcntl[MAXPHASES][SIZE_PHCD];
	static char phass[512];
	static char old_phases[512] ;
       */

	char *phase;
	int k;

        /* Input phases array is a comma separated list.  This section 
         breaks these tokens up and puts them in the pcntl array of char*/
	strcpy (phass, phases);
	strcat (phass, ",");
	nn = 0;
	for (phase=strtok(phass, ","); phase!=NULL; phase=strtok(NULL,",")) {
		//if (!phase[0]) continue;
		if (phase[0]!='\0') continue;
		strcpy (&pcntl[nn][0], phase);
		for (k=strlen(phase); k<SIZE_PHCD; k++) pcntl[nn][k] = ' ';
		nn++;
	}
        try {
            nn=tt_taup_set_table(model);
	} catch(SeisppError& serr){throw serr;};
	if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_setup:  ")
			+ "travel time table path variable was not initialized");

	tabin_ (&one, tblpath ) ; 
	brnset_ (&nn, pcntl, prnt, SIZE_PHCD);
	ttopen = 1;

	/* like others this is not useless*/
	return (0);
}
/* Not sure this routine is required.   Current code only uses 
a test for sending it "all", which is not worth implementing anyway*/

/*

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
*/


int TauPCalculator::tt_taup_set_event_depth(double edepth)
{
	/*
	static float depth;
	static float usrc[2];
	*/

	depth = edepth;
	if (depth < 0.001) depth = 0.001;
	depset_ (&depth, usrc);
	return (1);
}
vector<TauPComputedTime>  TauPCalculator::tt_taup(double del)
{
/*
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
*/
	int nphases;   // static above, but does not seem necessary 
	trtm_ (&del, &maxphases, &nphases, &(this->tt[0]),&(this->dtdd[0]),
		&(this->dtdh[0]), &(this->dddp[0]), &(this->dpcd[0]),
		SIZE_PHCD);
	//if (nphase <= 0) return (1);
	if(nphase < 0) 
	   throw SeisppError(base_error 
		+ "taup trtm function returned no results from phase list "
		+ phass);
	vector<TauPComputedTime> result;
	result.reserve(nphase);
	TauPComputedTime ttk;
	for(k=0;i<nphase;++k)
	{
	    /* It seems phcd tokens are blank terminated so we have to scan
	    for that instead of a null */
	    char phstmp[SIZE_PHCD];
	    for(l=0;l<SIZE_PHCD;++l)
	        if( (phcd[k][l] == ' ') || (l==SIZE_PHCD-1) )
		{
		    phcd[k][l]='\0';
		    break;
		}
	   ttk.phase=string(phcd[k]);
	   ttk.ttime=tt[k];
	   result.push_back(ttk);
	}
	return(result);
}

void
tt_taup_set_table_ (table_path, len_table_path)

char *table_path;
int len_table_path;

void tt_taup_set_table_
{
	string phs=tt_strf2c(table_path, len_table_path);
	tt_taup_set_table (phs.c_str());
}

void
tt_taup_set_phases_ (phases, len_phases)

char *phases;
int len_phases;

{
	string phs=tt_strf2c(phases,len_phases);
	tt_taup_set_phases (phs.c_str());
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

string tt_strf2c(const char *string, int len)
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
	return (string(out));
}
