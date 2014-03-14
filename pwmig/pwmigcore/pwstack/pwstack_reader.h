#include <stdio.h>
#include <vector>
#include <string>
using namespace std;
/*! \brief Abstract base class of pwstack data reader.

This is a virtual base class that allows pwstack to use polymorphism to 
sort out the correct reader.  This provides a handy way to allow multiple
input data formats.  Useful example of that type of abstraction of nothing else.
*/
class pwstack_reader
{
public:
    virtual ThreeComponentEnsemble *read_gather(int id)=0;
};
class PwstackBinaryFileReader : public pwstack_reader
{
public:
    PwstackBinaryFileReader(string fname);
    ~PwstackBinaryFileReader();
    ThreeComponentEnsemble *read_gather(int id);
    int number_events(){return(ids.size());};
private:
    FILE *fp;
    vector<long> ids;
    vector<long> foffs;
};
typedef struct PwstackGatherHeader
{
    int number_members;
    long evid;
    /* These are event location  (radians) */
    double lon;
    double lat;
    double depth;
    double origin_time;
} PwStackGatherHeader;
typedef struct PwstackTraceHeader
{
    char sta[8];
    double time;
    double endtime;
    int nsamp;
    double samprate;
    double atime;
    /* These hold receiver location (lat lon in radians internally)*/
    double lon;
    double lat;
    double elev;
} PwstackTraceHeader;
