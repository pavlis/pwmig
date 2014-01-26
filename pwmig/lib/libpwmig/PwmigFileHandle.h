#ifndef _PWMIGFILEHANDLE_
#define _PWMIGFILEHANDLE_
#include <stdio.h>
#include <vector>
#include "TimeSeries.h"
#include "ThreeComponentSeismogram.h"
#include "ensemble.h"
using namespace std;
using namespace SEISPP;

const string dfile_ext("dat");
const string hdr_ext("hdr");
/*Constants used to create file names for pwstack/pwmig from a root name */
const string DataFileExtension("_pwdata");
const string Coh3CExtension("_coh3c");
const string CohExtension("_coh");

/* The binary file defined by this set of code writes this as a reel
header (top of file).  It contains global for the whole data set. */
typedef struct PwmigFileGlobal_
{
	int evid;
	double slat;
	double slon;
	double sdepth;
	double stime;
	/* consistent with schema used in older version of this */
	char gridname[16];
	bool ThreeComp; //Set true when data in this file are 3 components 
} PwmigFileGlobals;
/* One of these is written for each record */
typedef struct PwmigRecords_ 
{
	int gridid;
	int ix1;
	int ix2;
	double ux0;
	double uy0;
	double ux;
	double uy;
	double elev;
	int nsamp;
	double t0;
	double dt; // could be global, but conceivable might need to to be variable someday
	/* Note off_t in the standards means a 64 it address.  Required for large file support */
	off_t foff;
} PwmigFileRecord;

class PwmigFileHandle
{
public:
	PwmigFileGlobals filehdr;
	PwmigFileHandle(string fname,bool rmode,bool smode); 
	/* In writing mode this will do a weird thing and dump
	the header contents to disk when it exits.  In read
	mode it just closes files */
	~PwmigFileHandle();
	void save(TimeSeries& ts);
	void save(ThreeComponentSeismogram& tcs);
	ThreeComponentEnsemble *load_next_3ce();
	TimeSeriesEnsemble *load_next_tse();
private:
	int datafd;
	int hdrfd;
	bool readmode;
	bool scalar_mode;
	vector<PwmigFileRecord> recs;
	vector<PwmigFileRecord>::iterator current_record;
};
#endif
