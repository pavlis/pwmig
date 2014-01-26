/* Guards and using namespace constructs were intentionally
ommitted from this file as it is not expected to be exported outside
use by this program.
*/

/*! Simple object to hold attributes needed to define a grid stack.

The GridScratchFileHandle object below uses a list of these objects
as a fast method to get weights assigned to grids used in a stack.
*/
class MemberGrid
{
public:
	string gridname;
	string fieldname;
	double baseweight;
	double reswt;
	MemberGrid(string g, string f, double b, double r)
	{
		gridname=g;
		fieldname=f;
		baseweight=b;
		reswt=r;
	};
	MemberGrid(const MemberGrid& parent);
	MemberGrid& operator=(const MemberGrid& parent);
};
/*! \brief Handle to a temp file used in grid stacking.

Three-dimensional grids contain very large amounts of data
that may not fit in a computer's memory.  This object is 
intimately linked to the grid stacking program and is used
to provide a faster mechanism to access member grids than
a database.  This is useful in that program as robust stacking
requires multiple passes through the data.  

The basic concept behind this object is to abstract the process 
of getting data from the scratch file.  The constructor creates
the object by loading all the data from a list of grids into 
the scratch file.  The idea is that once this object is created
a program can work through the scratch file requesting one object
at a time.  This is a general concept of common use in data processing
and there are reasons to think it could be a useful template.  
For now it is a concrete thing for 3d grid objects specialized to 
this particular program.
*/

class GridScratchFileHandle
{
public:
	/*! One and only constructor.

	\param mastergrid - used as reference.  All grids are interpolated onto master
	\param mg is a list used to define grids to be build stack(includes weights)
	\param db Datascope database handle (seispp API object)
	\param normalize controls solid angle normalization.  if true all field values are normalized by 1/solid angle range
	\aram cutoff minimum solid angle to use.  Points with solid angle range less than this will be zeroed and the solid angle
		vector component will be set to -1.0 as a signal that this cell is empty. 
	*/
	GridScratchFileHandle(GCLvectorfield3d& mastergrid,
		list<MemberGrid>& mg, DatascopeHandle& dbh,
		bool normalize, double cutoff);
	// copy constructor and operator= intentionally ommitted
	~GridScratchFileHandle();  
	
	void rewind();
	int load_next(double *d);  // returns count nval from fread
	/*! \brief Test for end of file on scratch file.
	
	The scratch file accessed through this method is assumed
	to always be read sequentially.  This method is used to
	test for end of the file.  Use as test in while or do-while.*/
	bool at_eof();

private:  
	FILE *fp;
	int nval;
	int n1,n2,n3,nv;
	int nmembers;
	int current_member;
	string scratch_file_name;
};
