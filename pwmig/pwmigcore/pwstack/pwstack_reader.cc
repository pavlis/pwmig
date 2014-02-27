#include <stdio.h>
#include <vector>
#include <sstream>
#include "seispp.h"
#include "SeisppError.h"
#include "Metadata.h"
#include "pwstack_reader.h"
using namespace std;
using namespace SEISPP;
/* Helpers.

   This one takes the gather header structure and returns a Metadata 
   object that can be passed to the 3C constructor.  A bit inefficient, but
   a stable solution */
Metadata PGHtoMD(PwStackGatherHeader& gh)
{
    /* The names here are frozen and a maintenance issue is matching
       these to get routines in pwstack code. */
    Metadata result;
    result.put("evid",static_cast<long>(gh.evid));
    /* assumed lon and lat are radians and depth is km */
    result.put("lon",gh.lon);
    result.put("lat",gh.lat);
    result.put("depth",gh.depth);
    return(result);
}
/* comparable routine for trace headers */
Metadata THtoMD(PwstackTraceHeader& th)
{
    Metadata result;
    result.put("sta",th.sta);
    result.put("time",th.time);
    result.put("endtime",th.endtime);
    result.put("nsamp",static_cast<int>(th.nsamp));
    result.put("samprate",th.samprate);
    /* assumed this is relative time - maintenance issue warning*/
    result.put("atime",th.atime);
    result.put("lon",th.lon);
    result.put("lat",th.lat);
    result.put("elev",th.elev);
    return(result);
}
PwstackBinaryFileReader::PwstackBinaryFileReader(string fname)
{
    const string base_error("PwstackBinaryFileReader constructor:  ");
    fp=fopen(fname.c_str(),"r");
    if(fp==NULL) throw SeisppError(base_error
            + "fopen failed for input file="+fname);
    /* Load the index vectors.  First read a directory seek position */
    long diroffset;
    if(fread(&diroffset,sizeof(long),1,fp)!=1) 
        throw SeisppError(base_error+"fread failed reading diroffset");
    if(fseek(fp,diroffset,SEEK_SET))
        throw SeisppError(base_error+"fseek error to diroffset value read");
    int nevents;
    if(fread(&nevents,sizeof(int),1,fp)!=1)
        throw SeisppError(base_error+"attempt to read nevents of index failed");
    int64_t *idin,*foffin;
    idin = new int64_t[nevents];
    foffin = new int64_t[nevents];
    if(fread(idin,sizeof(int64_t),nevents,fp)!=nevents)
        throw SeisppError(base_error+"attempt to read evid vector in index failed");
    if(fread(foffin,sizeof(int64_t),nevents,fp)!=nevents)
        throw SeisppError(base_error+"attempt to read file offset vector in index failed");
    /* A potential memory lead above if either of the two throws are
       executed.  Did this intentionally because in this use if any of these
       errors are thown pwstack will abort so there is no risk of a memory 
       leak. */
    int i;
    ids.reserve(nevents);
    for(i=0;i<nevents;++i) ids.push_back(idin[i]);
    foffs.reserve(nevents);
    for(i=0;i<nevents;++i) foffs.push_back(foffin[i]);
    delete [] idin;
    delete [] foffin;
}
PwstackBinaryFileReader::~PwstackBinaryFileReader()
{
    fclose(fp);
}
/* id is the vector index for foffs */
ThreeComponentEnsemble *PwstackBinaryFileReader::read_gather(int id)
{
    ThreeComponentEnsemble *result;
    const string base_error("PwstackBinaryFileReader::read_gather method:  ");
    if(fseek(fp,foffs[id],SEEK_SET))
    {
        stringstream ss;
        ss << base_error <<" fseek to offset "<<foffs[id]<<" failed for "
            << "evid="<<ids[id]<<endl;
        throw SeisppError(ss.str());
    }
    PwstackGatherHeader gh;
    if(fread(&gh,sizeof(PwstackGatherHeader),1,fp)!=1)
    {
        stringstream ss;
        ss << base_error << "fread error while attempting to read "
            <<"gather header for evid="<<ids[id];
        throw SeisppError(ss.str());
    }
    if(ids[id]!=gh.evid)
    {
        stringstream ss;
        ss << base_error << "evid mismatch.  "
            <<"gather header evid="<<gh.evid
            <<"   index evid expects evid="<<ids[id]<<endl;
        throw SeisppError(ss.str());
    }
    try {
        Metadata ghmd=PGHtoMD(gh);
        result=new ThreeComponentEnsemble(ghmd,gh.number_members);
        for(int i=0;i<gh.number_members;++i)
        {
            PwstackTraceHeader th;
            if(fread(&th,sizeof(PwstackTraceHeader),1,fp))
            {
                stringstream ss;
                ss << base_error <<" fread failed reading trace header for "
                    <<i<<"th member of ensemble of size"<<gh.number_members;
                delete result;
                throw SeisppError(ss.str());
            }
            Metadata trmd=THtoMD(th);
            ThreeComponentSeismogram seis(trmd,false);
            /* Set required parameters.  Some of these may reset attributes 
               set by constructor.  Intentional to make this more stable*/
            seis.dt=1.0/th.time;
            seis.t0=th.time;
            seis.ns=th.nsamp;
            seis.tref=relative;  // We assume arrival time reference frame
            seis.live=true;
            /* temporary pointer just to make this less obscure.
               Used to point to first byte of u matrix memory block*/
            double *uptr=seis.u.get_address(0,0);
            if(fread(uptr,sizeof(double),th.nsamp,fp)!=th.nsamp)
            {
                stringstream ss;
                ss << base_error <<" fread failed reading 3C data samples for "
                    <<i<<"th member of ensemble of size"<<gh.number_members
                    <<"This is data for station name "<<th.sta;
                delete result;
                throw SeisppError(ss.str());
            }
            result->member.push_back(seis);
        }
        return result;
    }catch(...)
    {
        delete result;
        throw;
    };
}
