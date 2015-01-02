#ifndef _PWMIGFILEHANDLE_
#define _PWMIGFILEHANDLE_
#include <stdio.h>
#include <vector>
#include "TimeSeries.h"
#include "ThreeComponentSeismogram.h"
#include "ensemble.h"
#include "SlownessVectorMatrix.h"
using namespace std;
using namespace SEISPP;
using namespace PWMIG;

const string dfile_ext("dat");
const string hdr_ext("hdr");
/*Constants used to create file names for pwstack/pwmig from a root name */
const string DataFileExtension("_pwdata");
const string Coh3CExtension("_coh3c");
const string CohExtension("_coh");
const string SlownessFileExtension("_slowness");   // always add .dat to this name

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
        /*! \brief Primary constructor usinga  file name.

          \param - fname is the root name used to construct a set of files created by this
                beast.
          \param rmode - when true create the handle in read only mode.
          \param smode - when true assume output will be scalar data (false implies 3C data).
          */
	PwmigFileHandle(string fname,bool rmode,bool smode); 
	/* In writing mode this will do a weird thing and dump
	the header contents to disk when it exits.  In read
	mode it just closes files */
	~PwmigFileHandle();
	void save(TimeSeries& ts);
	void save(ThreeComponentSeismogram& tcs);
        /*! Save the slowness vector data used in read with plane_wave_slowness_vectors method.
         
         \param svm is the grid of incident wave slowness vectors.
         \param ugrid is the grid of slowness vectors added to incident slowness at east point */
        void save_slowness_vectors(SlownessVectorMatrix& svm, RectangularSlownessGrid& ugrid);
	ThreeComponentEnsemble *load_next_3ce();
	TimeSeriesEnsemble *load_next_tse();
        /*! Return the stagrid n1xn2 matrix of slowness vectors for each pseudostation.

          \param iux - requested ux value of RectangularSlownessGrid.  
          \param iuy - requested uy value of RectangularSlownessGrid.   
          (In pwmig this defines the n1xn2 matrix of slowness vectors for the plane
          wave component at slowness grid position iux,iuy.)

          \exception - will throw a SeisppError if iux and iuy are outside range. */
        SlownessVectorMatrix plane_wave_slowness_vectors(int iux, int iuy);
        /*! Set a flag to destroy open files on exit.  This is a cleanup routine
          that is useful when writers realizes the output is empty.   Necessary because
          the file format writes somethings before pwstack knows if it will have any
          valid output */
        void mark_to_delete(){
            delete_files_on_exit=true;
        };
private:
	int datafd;
	int hdrfd;
	bool readmode;
	bool scalar_mode;
        bool delete_files_on_exit;
        string rootname;
	vector<PwmigFileRecord> recs;
	vector<PwmigFileRecord>::iterator current_record;
};
#endif
