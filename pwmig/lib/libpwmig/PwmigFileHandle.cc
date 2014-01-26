#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <algorithm>
#include "PwmigFileHandle.h"
using namespace std;
/* This appears necessary on quarray to allow large file support */
#define _FILE_OFFSET_BITS 64
/* used to sort records by gridid */
struct comp_gridid : public binary_function<PwmigFileRecord,PwmigFileRecord,bool> {
    bool operator()(PwmigFileRecord x, PwmigFileRecord y)
    {
	if(x.gridid<y.gridid)
		return true;
	else
		return false;
    }
};
PwmigFileHandle::PwmigFileHandle(string fname, bool rmode, bool smode)
{
	const string base_error("PwmigFileHandle constructor:  ");

	/* dfile_ext and hdr_ext come from the PwmigFileHandle.h include file */
	string dfile=fname+"."+dfile_ext;
	string hfile=fname+"."+hdr_ext;

	/* Get the easy stuff out of the way immediately */
	scalar_mode=smode;
	readmode=rmode;
	if(readmode)
	{
#ifdef _LARGEFILE64_SOURCE
		datafd=open(dfile.c_str(),O_RDONLY | O_LARGEFILE);
#else
		datafd=open(dfile.c_str(),O_RDONLY);
#endif
		if(datafd<0)
			throw SeisppError(base_error
			 + string("Cannot open input data file=")
			 + dfile);
#ifdef _LARGEFILE64_SOURCE
		hdrfd=open(hfile.c_str(),O_RDONLY | O_LARGEFILE);
#else
		hdrfd=open(hfile.c_str(),O_RDONLY);
#endif
		if(hdrfd<0)
		{
			close(datafd);
			throw SeisppError(base_error
			 + string("Cannot open input header data file=")
			 + dfile);
		}
		/* First read the global data block */
		
		ssize_t read_count;
		read_count=read(hdrfd,static_cast<void *>(&filehdr),sizeof(PwmigFileGlobals));
		if(read_count!=sizeof(PwmigFileGlobals))
			throw SeisppError(base_error
			 + string("fread error on file = ")+hfile
			 + string("\nError on first read attempt to read globals parameters") );
		PwmigFileRecord thisrec;
		/* Load up all the header data */
		while(read(hdrfd,static_cast<void *>(&thisrec),sizeof(PwmigFileRecord))>0)
		{
			recs.push_back(thisrec);
		}
		if(recs.size()<=0) throw SeisppError(base_error
			+ string("fread error reading header data from file=")
			+hfile);
		/* Sort the recs list by gridid so we can read in data for each plane
		wave component together */
		sort(recs.begin(),recs.end(),comp_gridid());
		current_record=recs.begin();
	}
	else
	{
		mode_t fumask=0775;
		/* In output mode all we do is open the file in write mode */
#ifdef _LARGEFILE64_SOURCE
		datafd=open(dfile.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,fumask);
#else
		datafd=open(dfile.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC, fumask);
#endif
		if(datafd<0)
			throw SeisppError(base_error
			 + string("Cannot open output data file=")
			 + dfile);
#ifdef _LARGEFILE64_SOURCE
		hdrfd=open(hfile.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,fumask);
#else
		hdrfd=open(hfile.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC,fumask);
#endif
		if(hdrfd<0)
		{
			close(datafd);
			throw SeisppError(base_error
			 + string("Cannot open output header data file=") );
		}
	}
}
PwmigFileHandle::~PwmigFileHandle()
{
	close(datafd);
	if(!readmode)
	{
		const string message("PwmigFileHandle::destructor:  error dumping output header data");
		ssize_t test;
		test=write(hdrfd,static_cast<void *>(&filehdr),sizeof(PwmigFileGlobals) );
		if(test!=sizeof(PwmigFileGlobals) )
		{
			close(hdrfd);
                        ostringstream ess;
                        ess << message <<endl
                            << "Tried to write PwmigFileGlobals block of size "
                            << sizeof(PwmigFileGlobals) <<endl
                            << "binary write function returned count of "
                            << test << "  Mismatch is fatal error."<<endl;
			throw SeisppError(ess.str());
		}
		for(current_record=recs.begin();current_record!=recs.end();++current_record)
		{
			test=write(hdrfd,static_cast<void *>(&(*current_record)),
				sizeof(PwmigFileRecord) );
			if(test!=sizeof(PwmigFileRecord))
			{
				close(hdrfd);
                        ostringstream ess;
                        ess << message <<endl
                            << "Tried to write PwmigFileRecord block of size "
                            << sizeof(PwmigFileRecord) <<endl
                            << "binary write function returned count of "
                            << test << "  Mismatch is fatal error."<<endl;
				throw SeisppError(message);
			}
		}
	}
}
template <class T> void PwmigFileRecord_load(T& ts, 
	PwmigFileRecord& hdr) 
{
	try {
		hdr.gridid=ts.get_int("gridid");
		hdr.ix1=ts.get_int("ix1");
		hdr.ix2=ts.get_int("ix2");
		hdr.ux0=ts.get_double("ux0");
		hdr.uy0=ts.get_double("uy0");
		hdr.ux=ts.get_double("ux");
		hdr.uy=ts.get_double("uy");
		hdr.elev=ts.get_double("elev");
		hdr.nsamp=ts.ns;
		hdr.dt=ts.dt;
		hdr.t0=ts.t0;
	} catch (MetadataGetError mderr)
	{
		throw mderr;
	}
}
/* complicated error function needed for errors from lseek */
string buildlseek_message(off_t foff)
{
	char buf[256];
	ostringstream ss(buf);
	ss<<"PwmigFileHandle::save method:  lseek error"<<endl
		<<"lseek returned off_t="<<foff <<endl;
	switch (errno)
	{
	case EBADF:
		ss <<"errno = EBADF - not an open file descriptor";
		break;
	case ESPIPE:
		ss <<"errno=ESPIPE - filedes is a pipe, socket of FIFO";
		break;
	case EINVAL:
		ss<<"errno=EINVAL - whence arguement invalid";
		break;
	case EOVERFLOW:
		ss << "errno=EOVERFLOW - file offset exceeds word length"
		  << endl<<"Probably a large file error";
		break;
	default:
		ss << "Unknown value for errno - "
			<< "program bug has probably overwritten data segment";
	}
	return(ss.str());
}
void PwmigFileHandle::save(TimeSeries& ts)
{
	PwmigFileRecord hdr;
	try {
		PwmigFileRecord_load<TimeSeries>(ts,hdr);
	} catch (MetadataGetError mderr)
	{
		throw mderr;
	}
	/* Load the current file position as foff for read method */
	hdr.foff=lseek(datafd,0,SEEK_CUR);
	if(hdr.foff<0)
	{
		string errmess=buildlseek_message(hdr.foff);
		throw SeisppError(errmess);
	}
	/* Assume a seek to end is not necessary and all saves will be appends */
	ssize_t test;
	void *sptr=static_cast<void *>(&(ts.s[0]));
	test=write(datafd,sptr,(ts.ns)*sizeof(double));
	if(test!=(ts.ns)*sizeof(double) )
        {
            ostringstream ess;
            ess << "PwmigFileHandle::save(scalar data):  write error" <<endl
                << "Tried to write "<< (ts.ns)*sizeof(double) << "bytes" <<endl
                << "write function return count = "<<test<<endl
                << "Mismatch is a fatal error"<<endl;
		throw SeisppError(ess.str());
        }
	recs.push_back(hdr);
}
void PwmigFileHandle::save(ThreeComponentSeismogram& tcs)
{
	PwmigFileRecord hdr;
	try {
		PwmigFileRecord_load<ThreeComponentSeismogram>(tcs,hdr);
	} catch (MetadataGetError mderr)
	{
		throw mderr;
	}
	/* Load the current file position as foff for read method */
	hdr.foff=lseek(datafd,0,SEEK_CUR);
	if(hdr.foff<0)
	{
		string errmess=buildlseek_message(hdr.foff);
		throw SeisppError(errmess);
	}
	recs.push_back(hdr);
	/* similar to above, but here we use the fact that in this implementation
	the data in a 3c seismogram is a contiguous block 3xnsamp long */
	ssize_t test;
	double *sptr=tcs.u.get_address(0,0);
	test=write(datafd,static_cast<void *>(sptr),
		3*sizeof(double)*tcs.ns);
	if(test!=(3*sizeof(double)*tcs.ns) )
        {
            ostringstream ess;
            ess << "PwmigFileHandle::save(3Cdata):  write error" <<endl
                << "Tried to write "<< 3*(tcs.ns)*sizeof(double) << "bytes" <<endl
                << "write function return count = "<<test<<endl
                << "Mismatch is a fatal error"<<endl;
		throw SeisppError(ess.str());
        }
}
/* This is the reciprocal of PwmigFileRecord_load above */
template <class T> void PwmigFileRecord_load_Metadata(PwmigFileRecord& hdr, T& d) 
{
	d.put("gridid",hdr.gridid);
	d.put("ix1",hdr.ix1);
	d.put("ix2",hdr.ix2);
	d.put("ux0",hdr.ux0);
	d.put("uy0",hdr.uy0);
	d.put("ux",hdr.ux);
	d.put("uy",hdr.uy);
	d.put("elev",hdr.elev);
	d.put("nsamp",hdr.nsamp);
	d.ns=hdr.nsamp;
	d.t0=hdr.t0;
	d.dt=hdr.dt;
}
/* similar for global parameters */
template <class T> void PwmigFileHandle_load_global(PwmigFileGlobals& g,T *d)
{
	d->put("evid",g.evid);
	d->put("origin.lat",g.slat);
	d->put("origin.lon",g.slon);
	d->put("origin.depth",g.sdepth);
	d->put("origin.time",g.stime);
}
/* companion to immediately below.  Loads on seismogram from file handle record */
ThreeComponentSeismogram *load_3c_seis(PwmigFileRecord& hdr, int fd)
{
	ThreeComponentSeismogram *seis=new ThreeComponentSeismogram(hdr.nsamp);
	PwmigFileRecord_load_Metadata<ThreeComponentSeismogram>(hdr,*seis);
	lseek(fd,hdr.foff,SEEK_SET);
	ssize_t test;
	test=read(fd,static_cast<void *>(seis->u.get_address(0,0)),
			(3*hdr.nsamp)*sizeof(double));
	if(test!=(3*hdr.nsamp)*sizeof(double) )
        {
            ostringstream ess;
            ess << "PwmigFileHandle load_3c_seis procedure:  read error"
                <<endl
                << "Tried to read "<< (3*hdr.nsamp)*sizeof(double) << " bytes"
                <<endl
                << "read function return value = "<<test
                << endl
                << "Mismatch is a fatal error"<<endl;
		throw SeisppError(ess.str());
        }
	return seis;
}
TimeSeries *load_seis(PwmigFileRecord& hdr, int fd)
{
	TimeSeries *seis=new TimeSeries(hdr.nsamp);
	PwmigFileRecord_load_Metadata<TimeSeries>(hdr,*seis);
	lseek(fd,hdr.foff,SEEK_SET);
	ssize_t test;
	double *ptr;
	ptr=&(seis->s[0]);
	test=read(fd,static_cast<void *>(ptr),(hdr.nsamp)*sizeof(double));
	if(test!=(hdr.nsamp)*sizeof(double) )
        {
            ostringstream ess;
            ess << "PwmigFileHandle load_seis procedure:  read error"
                <<endl
                << "Tried to read "<< (hdr.nsamp)*sizeof(double) << " bytes"
                <<endl
                << "read function return value = "<<test
                << endl
                << "Mismatch is a fatal error"<<endl;
		throw SeisppError(ess.str());
        }
	return seis;
}
ThreeComponentEnsemble *PwmigFileHandle::load_next_3ce()
{
	if(scalar_mode) throw SeisppError(
		string("PwmigFileHandle::load_next:  attempt to load ")
		+ string("three component data from a scalar file\n")
		+ string("Coding error") );
	if(current_record==recs.end()) return(NULL);
	ThreeComponentEnsemble *ens=new ThreeComponentEnsemble();
	PwmigFileHandle_load_global<ThreeComponentEnsemble>(filehdr,ens);
	int count;
	int current_gridid;
	double ux,uy;
	current_gridid=current_record->gridid;
	ThreeComponentSeismogram *tcs;
	count=0;
	do {
		// load the ensemble metadata on the first pass
		if(count==0)
		{
			ens->put("gridid",current_gridid);
			ens->put("ux",current_record->ux);
			ens->put("uy",current_record->uy);
			ens->put("ux0",current_record->ux0);
			ens->put("uy0",current_record->uy0);
		}
		try {
			tcs=load_3c_seis(*current_record,datafd);
		} catch (SeisppError serr)
		{
			serr.log_error();
			cerr << "Problems reading data for gridid="<<current_gridid<<endl
				<< "Grid index values="<<current_record->ix1
				<< " " << current_record->ix2<<endl;
			delete tcs;
		}
		tcs->live=true;
		tcs->tref=relative;
		ens->member.push_back(*tcs);
		delete tcs;
		++current_record;
		++count;
	} while( (current_record->gridid == current_gridid)
		&& (current_record!=recs.end()) );
	return(ens);
}
TimeSeriesEnsemble *PwmigFileHandle::load_next_tse()
{
	if(!scalar_mode) throw SeisppError(
		string("PwmigFileHandle::load_next:  attempt to load ")
		+ string("scalar data from a three component file\n")
		+ string("Coding error") );
	if(current_record==recs.end()) return(NULL);
	TimeSeriesEnsemble *ens=new TimeSeriesEnsemble();
	PwmigFileHandle_load_global<TimeSeriesEnsemble>(filehdr,ens);
	int count;
	int current_gridid;
	double ux,uy;
	current_gridid=current_record->gridid;
	TimeSeries *ts;
	count=0;
	do {
		// load the ensemble metadata on the first pass
		if(count==0)
		{
			ens->put("gridid",current_gridid);
			ens->put("ux",current_record->ux);
			ens->put("uy",current_record->uy);
			ens->put("ux0",current_record->ux0);
			ens->put("uy0",current_record->uy0);
		}
		try {
			ts=load_seis(*current_record,datafd);
		} catch (SeisppError serr)
		{
			serr.log_error();
			cerr << "Problems reading data for gridid="<<current_gridid<<endl
				<< "Grid index values="<<current_record->ix1
				<< " " << current_record->ix2<<endl;
			delete ts;
		}
		ts->live=true;
		ts->tref=relative;
		ens->member.push_back(*ts);
		delete ts;
		++current_record;
		++count;
	} while( (current_record->gridid == current_gridid)
		&& (current_record!=recs.end()) );
	return(ens);
}
