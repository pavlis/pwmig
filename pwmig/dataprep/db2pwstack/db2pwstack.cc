#include <stdio.h>
#include <string>
#include <list>
#include "seispp.h"
#include "TimeWindow.h"
#include "Metadata.h"
#include "ensemble.h"
#include "dbpp.h"
#include "pwstack_reader.h"
using namespace std;
using namespace SEISPP;
void LoadGatherHeader(ThreeComponentEnsemble& gather,PwstackGatherHeader& gh)
{
    try {
        gh.number_members=gather.member.size();
        long evid=gather.get_long("evid");
        if(SEISPP_verbose) 
            cout << "pwstack file writer: evid="<<evid<<" has "
                <<gh.number_members << " three-component seismograms"<<endl;
        gh.evid=evid;
        gh.lat=gather.get_double("origin.lat");
        gh.lon=gather.get_double("origin.lon");
        gh.depth=gather.get_double("origin.depth");
        gh.origin_time=gather.get_double("origin.time");
    }catch(SeisppError& serr)
    {
        cerr << "LoadGatherHeader procedure:  Probably a problem with ensemble Metadata"
            << endl<<"Error caught:"<<endl;
        serr.log_error();
        cerr << "Contents of ensemble metadata"<<endl
            << dynamic_cast<Metadata&>(gather)<<endl;
        cerr << "Fatal Error - cannot recover"<<endl;
        exit(-1);
    }
}
void LoadTraceHeader(ThreeComponentSeismogram& d,PwstackTraceHeader& th)
{
    try {
        string sta=d.get_string("sta");
        strncpy(th.sta,sta.c_str(),8);
        th.time=d.t0;
        th.endtime=d.endtime();
        th.nsamp=d.ns;
        th.samprate=1.0/d.dt;
        th.atime=d.get_double("arrival.time");
        th.lat=d.get_double("site.lat");
        th.lon=d.get_double("site.lon");
        th.elev=d.get_double("site.elev");
    }catch(...){throw;};
}
int write_ensemble(ThreeComponentEnsemble& g,FILE *fp)
{
    const string base_error("Error in write_ensemble procedure: ");
    try {
        PwstackTraceHeader th;
        int n=g.member.size();
        vector<ThreeComponentSeismogram>::iterator gptr;
        int count;
        for(gptr=g.member.begin(),count=0;gptr!=g.member.end();++gptr,++count)
        {
            LoadTraceHeader(*gptr,th);
            if(fwrite(&th,sizeof(PwstackTraceHeader),1,fp)!=1)
                throw SeisppError(base_error
                        + "fwrite error writing trace header for station "
                        + th.sta);
            double *dptr=gptr->u.get_address(0,0);
            int nstotal=3*(gptr->ns);
            if(fwrite(dptr,sizeof(double),nstotal,fp)!=nstotal)
                throw SeisppError(base_error
                        + "fwrite error writing sample data for station "
                        + th.sta);
        }
        return(count);
    } catch(...){throw;};
}

            
void usage()
{
    cerr << "db2pwstack db outfile [-v -pf pffile]"<<endl;
    exit(-1);
}
void write_directory(long *ids,long *foffs, int nevents,FILE *fp)
{
    const string base_error("write_error procedure:  ");
    /* Probably should do a seek to end of file, but not necessary 
       with current logic */
    long diroffset=ftell(fp);
    if(fwrite(ids,sizeof(long),nevents,fp)!=nevents)
        throw(base_error + "fwrite failed writing id vector");
    if(fwrite(foffs,sizeof(long),nevents,fp)!=nevents)
        throw(base_error + "fwrite failed writing foff vector");
    rewind(fp);
    if(fwrite(&diroffset,sizeof(long),1,fp)!=1)
        throw(base_error + "fwrite failed writing director offset address");
    // Converted to long to avoid mixed types for this pair of values
    // written at top of the file
    long nevout=(long)nevents;
    if(fwrite(&nevout,sizeof(long),1,fp)!=1) 
        throw(base_error + "fwrite failed writing nevents value");
}

bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
    ios::sync_with_stdio();
    if(argc<3) usage();
    string dbname(argv[1]);
    string outfile(argv[2]);
    string pffile("db2pwstack");
    int i;
    for(i=3;i<argc;++i)
    {
        string sarg(argv[i]);
        if(sarg=="-pf")
        {
            ++i;
            if(i>=argc) usage();
            pffile=string(argv[i]);
        }
        else if(sarg=="-v")
            SEISPP_verbose=true;
        else
            usage();
    }
    Pf *pf;
    if(pfread(const_cast<char *>(pffile.c_str()),&pf))
    {
        cerr << "pfread failed for pffile="<<pffile<<endl;
        usage();
    }
    FILE *fp;
    fp=fopen(outfile.c_str(),"w");
    if(fp==NULL) 
    {
        cerr << "fopen failed trying to open output file="<<outfile<<endl;
        exit(-1);
    }
    try{
        DatascopeHandle dbh(dbname, true);
        Metadata control(pf);
        string arrival_phase=control.get_string("phase_key");
	string dbviewmode(control.get_string("database_view_mode"));
	if(dbviewmode=="dbprocess")
        	dbh=DatascopeHandle(dbh,pf,string("dbprocess_commands"));
	else if(dbviewmode=="use_wfdisc")
	{
		dbh.lookup("arrival");
                dbh.subset("phase=~/"+arrival_phase+"/");
		list<string> j1,j2;
		j1.push_back("sta");
		j1.push_back("wfdisc.time::wfdisc.endtime");
		j2.push_back("sta");
		j2.push_back("arrival.time");
		dbh.leftjoin("wfdisc",j1,j2);
		dbh.natural_join("assoc");
		dbh.natural_join("origin");
		dbh.natural_join("event");
		dbh.subset("orid==prefor");
		dbh.natural_join("sitechan");
		dbh.natural_join("site");
		if(SEISPP_verbose) cout << "working view size="
			<<dbh.number_tuples()<<endl;
		list<string> sortkeys;
		sortkeys.push_back("evid");
		sortkeys.push_back("sta");
		sortkeys.push_back("chan");
		dbh.sort(sortkeys);
		list<string> gkey;
		gkey.push_back("evid");
		gkey.push_back("sta");
		dbh.group(gkey);
	}
	else if(dbviewmode=="use_wfprocess")
	{
		dbh.lookup("event");
		dbh.natural_join("origin");
		dbh.subset("orid==prefor");
		dbh.natural_join("assoc");
		dbh.natural_join("arrival");
                dbh.subset("phase=~/"+arrival_phase+"/");
                if(SEISPP_verbose) cout << "Catalog view size="<<dbh.number_tuples()<<endl;
		DatascopeHandle ljhandle(dbh);
		ljhandle.lookup("wfprocess");
		ljhandle.natural_join("sclink");
		ljhandle.natural_join("evlink");
                if(SEISPP_verbose)cout << "Left join table size="<<ljhandle.number_tuples()<<endl;
		list<string> jk,sjk;
		jk.push_back("evid");
		jk.push_back("sta");
		dbh.join(ljhandle,jk,jk);
                if(SEISPP_verbose)cout << "Working table size="<<dbh.number_tuples()<<endl;
                sjk.push_back("sta");
		dbh.join(string("site"),sjk,sjk);
                if(SEISPP_verbose)cout << "After site join size="<<dbh.number_tuples()<<endl;
		list<string> sortkeys;
		sortkeys.push_back("evid");
		sortkeys.push_back("sta");
		dbh.sort(sortkeys);
	}
	else
	{
		cerr << "Illegal option for parameter database_view_mode="<<dbviewmode;
		exit(-1);
	}
        MetadataList station_mdl=pfget_mdlist(pf,"station_metadata");
        MetadataList ensemble_mdl=pfget_mdlist(pf,"ensemble_metadata");
        AttributeMap InputAM("css3.0");
        double tsfull, tefull;
        tsfull = control.get_double("data_time_window_start");
        tefull = control.get_double("data_time_window_end");
        TimeWindow data_window(tsfull,tefull);
	cout << "Processing begins on database " 
		<<  dbname << endl
		<<"Number of rows in working database view== "<<dbh.number_tuples() <<endl;
        list<string> group_keys;
        group_keys.push_back("evid");
        dbh.group(group_keys);
        dbh.rewind();
        int nevents=dbh.number_tuples();
	cout << "This run will process data from "<<nevents<<" events."<<endl;
        PwstackGatherHeader gh;
        long *ids=new long[nevents];
        long *foffs=new long[nevents];
        long diroffset(0);
        /* This is a dummy write for now - filled in as last step*/
        fwrite(&diroffset,sizeof(long),1,fp);
        fwrite(&diroffset,sizeof(long),1,fp);
        int rec;
        for(rec=0,dbh.rewind();rec<dbh.number_tuples();++rec,++dbh)
        {
            /* This requires segmented data */
            ThreeComponentEnsemble *din = new ThreeComponentEnsemble(
                                           dynamic_cast<DatabaseHandle&>(dbh),
                                             station_mdl, ensemble_mdl,InputAM);
            auto_ptr<ThreeComponentEnsemble> ensemble = ArrivalTimeReference(*din,
                    "arrival.time",data_window);
            delete din;
            LoadGatherHeader(*ensemble,gh);
            long pos=ftell(fp);
            long evid=ensemble->get_long("evid");
            ids[rec]=evid;
            foffs[rec]=pos;
            if(fwrite(&gh,sizeof(PwstackGatherHeader),1,fp)!=1)
            {
                cerr << "fwrite failed writing ensemble header event for evid="
                    <<evid<<endl;
                exit(-1);
            }
            int nseis=write_ensemble(*ensemble,fp);
            if(SEISPP_verbose)
            {
                cout << "Event id ="<<evid<<endl
                    << "Wrote "<<nseis<<" seismograms of expected "<<ensemble->member.size()<<endl;
            }
        }
        write_directory(ids,foffs,nevents,fp);
        delete [] ids;
        delete [] foffs;
    }catch(std::exception& excp)
    {
        cerr << excp.what()<<endl;
    }
    catch(SeisppError& serr)
    {
        serr.log_error();
    }
}
