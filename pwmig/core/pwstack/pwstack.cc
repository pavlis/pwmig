#include <string>
#include <sstream>
#include "stock.h"
#include "elog.h"
#include "pf.h"
#include "glputil.h"
#include "gclgrid.h"
#include "Hypocenter.h"
#include "seispp.h"
#include "PwmigFileHandle.h"
#include "pwstack.h"

using namespace std;
using namespace SEISPP;

bool Verbose;

void usage()
{
    cbanner((char *)"$Revision: 1.17 $ $Date: 2010/02/09 11:53:30 $",
        (char *)"dbin [-np np -rank rank -v -V -pf pfname]",
        (char *)"Gary Pavlis",
        (char *)"Indiana University",
        (char *)"pavlis@indiana.edu") ;
    exit(-1);
}


void load_file_globals(PwmigFileHandle& fh,int evid, double olat, double olon, 
	double odepth, double otime, string gridname)
{
	fh.filehdr.evid=evid;
	fh.filehdr.slat=olat;
	fh.filehdr.slon=olon;
	fh.filehdr.sdepth=odepth;
	fh.filehdr.stime=otime;
	strncpy(fh.filehdr.gridname,gridname.c_str(),16);
}
/* Removes members of d that have no data inside time aligned window range */
auto_ptr<ThreeComponentEnsemble> clean_gather(
	auto_ptr<ThreeComponentEnsemble> d,
		double tstart, double tend)
{
	vector<ThreeComponentSeismogram>::iterator dptr;
	for(dptr=d->member.begin();dptr!=d->member.end();++dptr)
	{
		//DEGUG
		/*
		cout << "clean_gather sta="<<dptr->get_string("sta")
			<< " t0="<< (dptr->t0) <<endl;
		*/
		double t0d=dptr->t0;
		double ted=dptr->endtime();
		/* Test for data outside stack window and delete
 		them.  dptr assignment construct is used because
		stl erase method returns iterator of next element
		after the delete or the end.  Proper way to 
		do this with an stl container. */
		if((t0d>tend)||(ted<tstart))
		{
			if(SEISPP_verbose) cerr << "clean_gather:  "
					<< "deleting seismogram for sta="
					<< dptr->get_string("sta")
					<<endl;
			dptr=d->member.erase(dptr);
		}
	}
	return(d);
}

	


#ifdef MATLABDEBUG
MatlabProcessor mp(stdout);
#endif

bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
    Dbptr db;
    int i,j;
    char *pfin=NULL;
    int sift=0;
    char subset_string[128];
    Pf *pf;
    Pf *pfnext;
    char *ensemble_tag;

    // This is the input data ensemble
    ThreeComponentEnsemble *din;
    // Tbl tag used in pf to define depth-dependent apeture.
    // frozen here as this constant but passed as a ariable to the
    // appropriate constructor below
    string aperture_tag="depth_dependent_aperture";
    string mdlist_tag="copy_metadata_list";

    ios::sync_with_stdio();
    /* Initialize the error log and write a version notice */
    //elog_init (argc, argv);

    /* usual cracking of command line */
    if(argc < 2) usage();
    string dbname_in(argv[1]);
    int rank(0);
    int np(1);

    for(i=2;i<argc;++i)
    {
	string sarg(argv[i]);
	if(sarg=="-V")
            usage();
	else if(sarg=="-v")
        {
            Verbose=true;
            SEISPP::SEISPP_verbose=true;
        }
	else if(sarg=="-pf")
        {
            ++i;
            if(i>=argc) usage();
            pfin = argv[i];
        }
        else if(sarg=="-rank")
        {
            ++i;
            if(i>=argc) usage();
            rank=atoi(argv[i]);
	    if(rank<0) 
	    {
		cerr << "Illegal value for -r argument ="
			<< rank<<endl
			<< "Must be nonnegative integer"<<endl;
		usage();
	    }
        }
	else if(sarg=="-np")
	{
	    ++i;
	    if(i>=argc) usage();
	    np=atoi(argv[i]);
	    if(np<=0) 
	    {
		cerr << "Illegal value for -np argument ="
			<< np<<endl
			<< "Must be positive integer"<<endl;
		usage();
            }
	}
        else
            usage();
    }
	/* sanity checks on manual parallel variables */
	if( (np>0) || (rank>=0) )
	{
		if( (np<0) || (rank<0) )
		{
			cerr << "Illegal argument combination.  "
				<< "-rank and -np must both be set if either is set"
				<< endl;
			usage();
		}
		else if(rank>(np-1))
		{
			cerr << "Illegal argument combination.  "
				<< "Value for -rank must be >= -np"<<endl;
			usage();
		}
	}
	else
	{
		/* single processor mode set if we end up here.*/
		np=1;
		rank=0;
	}
    /* this sets defaults */
    if(pfin == NULL) pfin = strdup("pwstack");

    i = pfread(pfin,&pf);
    if(i != 0) die(1,(char *)"Pfread error\n");
    try
    {
	Metadata control(pf);
        // This builds the grid of plane wave components
        RectangularSlownessGrid ugrid(pf,"Slowness_Grid_Definition");
        // control parameters on stack process
        double ts,te;
	ts=control.get_double("stack_time_start");
	te=control.get_double("stack_time_end");
	int stack_count_cutoff=control.get_int("stack_count_cutoff");
	double aperture_taper_length=control.get_double("aperture_taper_length");
	double centroid_cutoff=control.get_double("centroid_cutoff");
        // the data are windowed around arrivals to this interval
        // normally should be ts:te with sufficient allowance for moveout
        double tsfull, tefull;                    // normally longer than ts and te to allow
        tsfull = control.get_double("data_time_window_start");
        tefull = control.get_double("data_time_window_end");
        TimeWindow data_window(tsfull,tefull);
        string dir = control.get_string("waveform_directory");
        if(dir.size()<=0)
            dir=strdup("./pwstack");
        if(makedir(const_cast<char *>(dir.c_str())))
	{
	    cerr << "Cannot create directory ="<<dir<<endl;
            exit(-1);
	}
	string dfilebase=control.get_string("output_data_file_base_name");
	string fielddir=control.get_string("field_file_directory");
	string fieldnamebase=control.get_string("field_base_name");
        //
        // station_mdl defines database attributes copied to
        // metadata space of data object.  ensemble_mdl are
        // globals copied to the metadata area for the entire ensemble
        //
        MetadataList station_mdl=pfget_mdlist(pf,"station_metadata");
        MetadataList ensemble_mdl=pfget_mdlist(pf,"ensemble_metadata");
        AttributeMap InputAM("css3.0");
        bool use_fresnel_aperture=control.get_bool("use_fresnel_aperture");
        DepthDependentAperture aperture;
        if(use_fresnel_aperture)
        {
            cout << "Using Fresnel zone aperture widths for stacks"<<endl;
            double fvs,fvp,fcm,fperiod,fdt;
            int fnt;
            fvs=control.get_double("fresnel_vs");
            fvp=control.get_double("fresnel_vp");
            fcm=control.get_double("fresnel_cutoff_multiplier");
            fperiod=control.get_double("fresnel_period");
            fdt=control.get_double("fresnel_lag_time_sampling_interval");
            fnt=control.get_int("fresnel_number_lag_samples");
            cout << "Fresnel zone parameters:  Vs="<<fvs
                << ", Vp="<<fvp
                << ", period="<<fperiod
                << ", cutoff multiplier="<<fcm
                << "Aperture parameters will be specified by "<<fnt
                << " points sampled at intervals of " <<fdt
                << " for total depth of "<< fdt*static_cast<double>(fnt)
                <<endl;
            aperture=DepthDependentAperture(fvs,fvp,fperiod,fdt,fnt,
                    fcm,true);
        }
        else 
        {
            aperture=DepthDependentAperture(pf,aperture_tag);
        }
	/* These will throw an exception if not defined.  
	We use a parameter to turn these off if desired.
	Not very elegant, but functional. */
        TopMute mute(pf,string("Data_Top_Mute"));
	bool enable_data_mute=control.get_bool("enable_data_mute");
	if(enable_data_mute)
		mute.enabled=true;
	else
		mute.enabled=false;
        TopMute stackmute(pf,string("Stack_Top_Mute"));
	bool enable_stack_mute=control.get_bool("enable_stack_mute");
	if(enable_stack_mute)
		stackmute.enabled=true;
	else
		stackmute.enabled=false;
        /* new parameters April 2007 for coherence attribute calculator*/
        double dtcoh,cohwinlen;
        dtcoh=control.get_double("coherence_sample_interval");
        cohwinlen=control.get_double("coherence_average_length");
        /* This database open must create a doubly grouped view to
        // correctly define a three component ensemble grouping
        // normally this will be done by a hidden dbprocess list
        // stored in the master pf directory
        */
        DatascopeHandle dbh(dbname_in,true);
	string dbviewmode(control.get_string("database_view_mode"));
	if(dbviewmode=="dbprocess")
        	dbh=DatascopeHandle(dbh,pf,string("dbprocess_commands"));
	else if(dbviewmode=="use_wfdisc")
	{
		dbh.lookup("arrival");
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
cout << "Catalog view size="<<dbh.number_tuples()<<endl;
		DatascopeHandle ljhandle(dbh);
		ljhandle.lookup("wfprocess");
		ljhandle.natural_join("sclink");
		ljhandle.natural_join("evlink");
cout << "Left join table size="<<ljhandle.number_tuples()<<endl;
		list<string> jk,sjk;
		jk.push_back("evid");
		jk.push_back("sta");
		dbh.join(ljhandle,jk,jk);
cout << "Working table size="<<dbh.number_tuples()<<endl;
                sjk.push_back("sta");
		dbh.join(string("site"),sjk,sjk);
cout << "After site join size="<<dbh.number_tuples()<<endl;
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
	cout << "Processing begins on database " 
		<<  dbname_in << endl
		<<"Number of rows in working database view== "<<dbh.number_tuples() <<endl;
        list<string> group_keys;
        group_keys.push_back("evid");
        dbh.group(group_keys);
        dbh.rewind();
	cout << "This run will process data from "<<dbh.number_tuples()<<" events."<<endl;

        // We need to load the primary GCLgrid that defines the
        // location of pseudostation points.  It is assumed we
        // will build a site and sitechan table in the output database
        // in that define these virtual stations in a different
        // application.  i.e. dbho will eventually need a site table
        // produced separately to be useful.  This program should not
        // need this information though.
        //
        string stagridname=control.get_string("pseudostation_grid_name");
        GCLgrid stagrid(dbh,stagridname);
        cout << "Using pseudostation grid of size "<<stagrid.n1<<" X " <<stagrid.n2<<endl;

	/* This field derived from stagrid is used to hold stack fold
	for each pseudostation grid point */
	GCLscalarfield fold(stagrid);
	fold.zero();  // best to guarantee initialization

	char ssbuf[256];
        int rec;
        //DEBUG
        /*
        MatlabProcessor mp(stdout);
        */
        for(rec=0,dbh.rewind();rec<dbh.number_tuples();++rec,++dbh)
        {
	    if(rec%np != rank)continue;
            int iret;
            int evid;
            double olat,olon,odepth,otime;
            // ensemble is read once for entire grid in this
            // version of the program.  This assumes newer
            // methods like that under development by Fan
            // will prove better than pseudostation method
            //

            din = new ThreeComponentEnsemble(
                dynamic_cast<DatabaseHandle&>(dbh),
                station_mdl, ensemble_mdl,InputAM);
            //DEBUG
#ifdef MATLABDEBUG
            string chans[3]={"x1","x2","x3"};
            mp.load(*din,chans);
            mp.process(string("wigb(x1);figure;wigb(x2);figure;wigb(x3);"));
            mp.run_interactive();
#endif
            auto_ptr<ThreeComponentEnsemble>
                ensemble=ArrivalTimeReference(*din,"arrival.time",
                data_window);
            // Release this potentially large memory area
            delete din;
	    if(SEISPP_verbose) cout << "Ensemble for evid="
			<<  ensemble->get_int("evid")
			<< " has "<<ensemble->member.size()<<" seismograms"
			<<endl;
            /* Throws out data without overlaping times*/
	    int original_ensemble_size=ensemble->member.size();
	    ensemble=clean_gather(ensemble,ts,te);
	    int clean_ensemble_size=ensemble->member.size();
	    if(clean_ensemble_size!=original_ensemble_size)
	    {
		cerr << "Warning:  potential data problem"<<endl
			<< "clean_gather procedure cleared "
			<<(clean_ensemble_size-original_ensemble_size)
			<<" seismograms with time range outside "
			<<"stack window"<<endl
			<<"Number of seismograms remaining="
			<<ensemble->member.size()<<endl;
	    }
            //DEBUG
#ifdef MATLABDEBUG
            mp.load(*ensemble,chans);
            mp.process(string("figure; wigb(x1)"));
            mp.process(string("figure; wigb(x2)"));
            mp.run_interactive();
#endif
            // this should probably be in a try block, but we need to
            // extract it here or we extract it many times later.
            evid=ensemble->get_int("evid");
            olat=ensemble->get_double("origin.lat");
            olon=ensemble->get_double("origin.lon");
            // Database has these in degrees, but we need them in radians here.
            olat=rad(olat);  olon=rad(olon);
            odepth=ensemble->get_double("origin.depth");
            otime=ensemble->get_double("origin.time");
	    /* A way to assure this is cleared.  May not be necessary */
	    ssbuf[0]='\0';
	    stringstream ss(ssbuf);
	    ss << dir << "/" << dfilebase << "_" << evid;
	    string dfilebase=ss.str();
	    string dfile=dfilebase+DataFileExtension;
	    string coh3cf=dfilebase+Coh3CExtension;
	    string cohf=dfilebase+CohExtension;
	    PwmigFileHandle dfh(dfile,false,false);
	    load_file_globals(dfh,evid,olat,olon,odepth,otime,stagridname);
	    PwmigFileHandle coh3cfh(coh3cf,false,false);
	    load_file_globals(coh3cfh,evid,olat,olon,odepth,otime,stagridname);
	    PwmigFileHandle cohfh(cohf,false,true);
	    load_file_globals(cohfh,evid,olat,olon,odepth,otime,stagridname);
            double lat0,lon0,elev0;
	    cout << "Beginning processing for event id = "<<evid<<endl;
            for(i=0;i<stagrid.n1;++i)
                for(j=0;j<stagrid.n2;++j)
            {
                lat0=stagrid.lat(i,j);
                lon0=stagrid.lon(i,j);
                elev0=stagrid.depth(i,j);
                elev0=-elev0;                     // z is positive down, elev is positive up
                // ux0 and uy0 are the incident wavefield's
                // slowness vector.  Stacks are made relative to this
                // and the data are aligned using P wave arrival times
                try
                {
                    //
                    // this is a backdoor method to pass this information
                    // to the main processing program called just below.
                    // perhaps bad form, but it is logical as these
                    // are metadata related to these data by any measure.
                    ensemble->put("ix1",i);
                    ensemble->put("ix2",j);
                    ensemble->put("lat0",lat0);
                    ensemble->put("lon0",lon0);
                    ensemble->put("elev0",elev0);
                    ensemble->put("gridname",stagridname);
                    Hypocenter hypo(olat,olon,odepth,otime,
                        string("tttaup"),string("iasp91"));
                    SlownessVector slow=hypo.pslow(lat0,lon0,elev0);
                    ensemble->put("ux0",slow.ux);
                    ensemble->put("uy0",slow.uy);

                    iret=pwstack_ensemble(*ensemble,
                        ugrid,
                        mute,
                        stackmute,
			stack_count_cutoff,
                        ts,
                        te,
                        aperture,
			aperture_taper_length,
			centroid_cutoff,
                        dtcoh,
                        cohwinlen,
                        ensemble_mdl,
                        dfh,
			coh3cfh,
			cohfh);
		    if(iret<=0)
		    {
			fold.val[i][j]=0.0;
		    }
		    else
		    {
			fold.val[i][j]=static_cast<double>(iret);
		    }
		    if(SEISPP_verbose)
                    {
                        if(iret>0)
			    cout << "Grid point ("<<i<<", "<<j
				<<") has stack fold="<<(int)(fold.val[i][j])
				<<endl;
                        else
                        {
                            cout << "Grid point ("<<i<<", "<<j
                                <<") was not processed:  ";;
                            switch(iret)
                            {
                            case (-1):
			    case (0):
                                cout << "stack count below threshold."<<endl;
                                break;
                            case (-2):
                                cout << "centroid of stations outside cutoff"<<endl;
                                break;
                            case (-3):
                                cout << "sum of weights at zero lag is "
                                    << "below threshold (probably 0)"
                                    <<endl;
                                break;
                            default:
                                cout << endl
                                    << "ERROR.  Unknown return code returned by "
                                    << "pwstack_ensemble ="<<iret<<endl
                                    << "This is a coding error that needs to be fixed"
                                    << "Check the source code"<<endl;
                                exit(-1);;
                            }
                        }
                    }
                } catch (SeisppError& serr)
                {
                    serr.log_error();
                    cerr << "Ensemble " << rec << " data skipped" << endl;
                    cerr << "Pseudostation grid point indices (i,j)="
                        << "("<<i<<","<<j<<")"<<endl;
                }
            }
	    char fsbuf[64];
	    sprintf(fsbuf,"%s_%d",fieldnamebase.c_str(),evid);
	    string fieldname(fsbuf);
            /* This uses the new file based save metod */
            try {
                fold.save(fieldname,fielddir);
            }catch  (exception& e)
            {
                cerr << "Error saving fold data to file "<<fieldname
                    << " in directory "<<fielddir<<endl
                    << "Bundering on as this is not critical data."
                    <<endl;
            }
        }
    } catch (SeisppError& err)
    {
        err.log_error();
    }
    // The GCLgrid library db routines throw simple int exceptions
    // This should only be entered if the GCLgrid constructor fails.
    catch (exception& stdexc)
    {
        cerr << stdexc.what()<<endl;
    }
}
