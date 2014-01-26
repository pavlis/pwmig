#include <stdint.h>
#include "SEGY2002FileHandle.h"
typedef struct SegyReel{
        int32_t            kjob, kline, kreel;
        int16_t           kntr, knaux, sr, kfldsr, knsamp, 
				kfsamp, dsfc, kmfold, ksort;
        /* Above were in old SEGY.  Added in 2002 standard */
        int16_t kvsum,ksweepstart,ksweepend,ksweeptimelen;
        int16_t ksweepcode,ksweepchan,ksweeptaperst,ksweeptaperend;
        /* These are actually boolean parameters handled as int 1 or 2 */
        int16_t taper_type,correlated,gainrecovered;
        /* switch parameters for different options - see standard */
        int16_t ampmeth,measure_system,polarity,vibpolarity;
        unsigned char unused1[240];
        int16_t segyrev;
        int16_t fixed_length_flag;
        int16_t number_extended_text_headers;
	unsigned char unused2[94];
} SegyReel;
/* Initializer for SegyReel struct.  Could be class constructor, but
   since the struct is so rigid better done this way. */
void Initialize_BinaryHeader(SegyReel *h)
{
    h->kjob=0;
    h->kline=0;
    h->kreel=0;
    h->kntr=0;
    h->knaux=0;
    h->sr=0;
    h->kfldsr=0;
    h->knsamp=0;
    h->kfsamp=0;
    h->dsfc=5;
    h->kmfold=0;
    /* default for this program is cmp equivalents */
    h->ksort=9;
    h->kvsum=1;
    h->ksweepstart=0;
    h->ksweepend=0;
    h->ksweeptimelen=0;
    h->ksweepcode=0;
    h->ksweepchan=0;
    h->ksweeptaperst=0;
    h->ksweeptaperend=0;
    h->taper_type=0;
    h->correlated=1;
    h->gainrecovered=1;
    h->ampmeth=1;
    h->measure_system=1;
    h->polarity=2;
    h->vibpolarity=0;
    h->segyrev=1;
    h->fixed_length_flag=0;
    h->number_extended_text_headers=0;
    int i;
    const int nu1(240),nu2(94);
    for(i=0;i<nu1;++i)
        h->unused1[i]='\0';
    for(i=0;i<nu2;++i)
        h->unused2[i]='\0';
}
/* Common error logging routine. */
void log_error(SeisppError& serr,const char *p,const char *def)
{
    const string base_error("SEGY2002FileHandle::WriteFileHeaders:  ");
    cerr << p << " parameter not defined.  Actual error message thrown was:"<<endl;
    serr.log_error();
    cerr << "Defaulted to "<<def<<endl;
}


/* Private method writes file (reel) headers */
void SEGY2002FileHandle::WriteFileHeaders(Metadata& md)
{
    const string base_error("SEGY2002FileHandle::WriteFileHeaders:  ");
    char textheader[3200];
    /* For now we set textheader to all blanks characters */
    for(int i=0;i<3200;++i) textheader[i]=' ';
    /* Define and initialize binary (file) header */
    SegyReel BinaryHeader;
    Initialize_BinaryHeader(&BinaryHeader);
    try {
        BinaryHeader.kjob=md.get_long("job_id_number");
    }catch(SeisppError& serr)
    {
        log_error(serr,"job_id_number","1");
        BinaryHeader.kjob=1;
    };
    try {
        BinaryHeader.kline=md.get_long("line_number");
    }catch(SeisppError& serr)
    {
        log_error(serr,"line_number","1");
        BinaryHeader.kline=1;
    };
    // This one is always set to 1 here 
    BinaryHeader.kreel=1;
    try {
        BinaryHeader.kntr=md.get_int("number_of_traces_per_ensemble");
    }catch(SeisppError& serr)
   {
        log_error(serr,"number_of_traces_per_ensemble","1");
        BinaryHeader.kntr=1;
   };

    try {
        BinaryHeader.knaux=md.get_int("number_of_aux_traces_per_ensemble");
    }catch(SeisppError& serr)
   {
        log_error(serr,"number_of_aux_traces_per_ensemble","0");
        BinaryHeader.knaux=0;
   };
    try {
        BinaryHeader.sr=md.get_int("sample_interval");
    }catch(SeisppError& serr)
   {
        log_error(serr,"sample_interval","1000");
        BinaryHeader.sr=1000;
   };
    try {
        BinaryHeader.kfldsr=md.get_int("field_sample_interval");
    }catch(SeisppError& serr)
   {
       // Silently set to sr if missing
        BinaryHeader.kfldsr=BinaryHeader.sr;
   };
   /* This one must be required */
    try {
        BinaryHeader.knsamp=md.get_int("number_samples");
    }catch(SeisppError& serr)
   {
       cerr << "SEGY2002FileHandle::WriterFileHeaders:  " 
           << "Missing required parameter number_samples"<<endl
           <<"Fatal error"<<endl;
       exit(-1);
   };
    try {
        BinaryHeader.kfsamp=md.get_int("field_number_samples");
    }catch(SeisppError& serr)
   {
        // Silently make equal to knsamp if not defined
        BinaryHeader.kfsamp=BinaryHeader.knsamp;
   };
   /* This is frozen to 32 bit ieee floats */
   BinaryHeader.dsfc=5;
    try {
        BinaryHeader.kmfold=md.get_int("fold");
    }catch(SeisppError& serr)
   {
        log_error(serr,"fold","1");
        BinaryHeader.kmfold=1;
   };
    try {
        BinaryHeader.ksort=md.get_int("segy_sort_order_code");
    }catch(SeisppError& serr)
   {
       /* Standard says 2 is "CDP Ensemble" */
        log_error(serr,"segy_sort_order_code","2");
        BinaryHeader.ksort=2;
   };
    /* Intentionally ignoring (defaulting):  sweep variables, taper_type
       correlated, and gainrecovered. */
    try {
        string amplitude_method=md.get_string("amplitude_method");
        if(amplitude_method=="AGC")
            BinaryHeader.ampmeth=3;
        else if (amplitude_method=="spherical_divergence")
            BinaryHeader.ampmeth=2;
        else if(amplitude_method=="none")
            BinaryHeader.ampmeth=1;
        else
            BinaryHeader.ampmeth=4;
    }catch(SeisppError& serr)
    {
        cerr << base_error<<" amplitude_method parameter not defined"<<endl
            << "Defaulted to none"<<endl;
        BinaryHeader.ampmeth=1;
    }
    try{
        string meas=md.get_string("measurement_units");
        if(meas=="feet") 
            BinaryHeader.measure_system=2;
        else
            BinaryHeader.measure_system=1;
    }catch(SeisppError& serr)
    {
        cerr << base_error<<" measurement_units parameter not defined"<<endl
            << "Defaulted to meters"<<endl;
        BinaryHeader.measure_system=1;
    }
    try {
        string polarity=md.get_string("polarity");
        if(polarity=="reversed")
            BinaryHeader.polarity=1;
        else 
            BinaryHeader.polarity=2;
    }catch(SeisppError& serr)
    {
        cerr << base_error<<" polarity parameter not defined"<<endl
            << "Defaulted to normal (segy 2)"<<endl;
        BinaryHeader.measure_system=2;
    }
    /* Intentionally ignore vibpolarity */
    /* Weirdness of the standard requires this in hex 
     This is for 2002 standard set as 1.0 */
    BinaryHeader.segyrev=0x10;  
    /* Booleans always work in this context so do not need a try block */
    bool fl=md.get_bool("using_variable_length_records");
    if(fl)
        BinaryHeader.fixed_length_flag=0;
    else
        BinaryHeader.fixed_length_flag=1;

    int iret;
    iret=GenericFileHandle::put(const_cast<char *>(textheader),3200);
    iret=GenericFileHandle::put(reinterpret_cast<const char *>(&BinaryHeader),400);

}
SEGY2002FileHandle::SEGY2002FileHandle(string fname,
        list<string> tmdlist, Pf *pf,
        bool read_only) : GenericFileHandle(fname,string("SEGY2002"),read_only)
{
    const string base_error("SEGY2002FileHandle constructor:  ");
    try {
        Metadata md(pf);
        /* We define these variables we need to send tot he set_required 
           protected method */
        string amname=md.get_string("AttributeCrossReferenceMapName");
        string xrefstr=pftbl2string(pf,amname.c_str());
        AttributeCrossReference xref(xrefstr);
        /* These are left empty because planning only to implement
           a writer.  Change if implementing a reader */
        list<string> orderkeys,ensmdlist;
        if(read_only)
            throw SeisppError(base_error 
                    + "Attempt to open in read mode.  Only a writer.");
        /* Hard code some arguments to set_required less elegant, but
           this is not intenteded to be the most elegant code ever. */
        this->set_required(xref,orderkeys,ensmdlist,tmdlist,
            string("nsamp"),string("dt"),1000000.0,true,false);

        /* Make sure file output stream is at 0 offset */
        this->rewind();
        WriteFileHeaders(md);
    }catch(...){throw;};
}
        


