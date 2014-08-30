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

void TauPCalculator::tt_taup_set_table(const char *table_path)
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
}

string TauPCalculator::taup_get_modname()
{
	int i;

        if(strlen(tblpath)<=0)
		throw SeisppError(string("TauPCalculator::taup_get_modname:  ")
			+ "Travel time data path not set.  Initialization problem");
	for (i=strlen(tblpath); i>=0; i--) if (tblpath[i] == '/') {i++;break;}
	return(string(&(tblpath[i])));
}


void TauPCalculator::taup_setup(const char *model, const char *phases)
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


void TauPCalculator::tt_taup_set_event_depth(double edepth)
{
	/*
	static float depth;
	static float usrc[2];
	*/

	depth = edepth;
	if (depth < 0.001) depth = 0.001;
	depset_ (&depth, usrc);
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

