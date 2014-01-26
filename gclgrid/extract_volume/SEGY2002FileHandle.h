#include "GenericFileHandle.h"
using namespace std;
using namespace SEISPP;
/*! \brief File handle for new SEGY standard (year 2002 document).

  This is an incomplete implementation of a file handle for data in
  the 2002 SEGY standard.  It is currently implemented only as a writer
  used to write pwmig migrated images into a format that can be loaded
  in interpretation packages like Kingdom or Petrel. 
 
 Note the initial implementation will not mess with byte order issues.
 Technically segy 2002 standard says data must be written in big
 endian order, but I suspect Kingdom and Petrel allow violation of
 this dogmatism in the standard since little endian machines have
 won the processor war in the last decade..*/
class SEGY2002FileHandle : public GenericFileHandle
{
    public:
        SEGY2002FileHandle(string fname, 
                list<string> trace_mdlist,Pf *pf,
                bool read_only=false);
        /*! \breif write a time series object. 

          Writes a TimeSeries object to output handle.  
          \param d is the object to be written. */
        int put(TimeSeries& d);
    private:
        /* This is put in a private method to allow additional
           constructors to make use of common code more easily */
        void WriteFileHeaders(Metadata& md); 
};

