#include "JsonStyleMetadata.h"
namespace SEISPP {
using namespace std;
using namespace SEISPP;

/* This is translating the type from jsoncpp to
	JsonStyleInputType defined above.
	*/
JsonStyleInputType jsonmd_type(Value token)
{
    int tp=token.type();
    switch(tp)
    {
    	case 0:
    		return(JSMDNULL);
    	case 1:
    		return(JSMDINT);
    	case 2:
    		return(JSMDUINT);
    	case 3:
    		return(JSMDREAL);
    	case 4:
    		return(JSMDSTR);
    	case 5:
    		return(JSMDBOOL);
    	case 6:
    		return(JSMDARR);
    	case 7:
    		return(JSMDOBJ);
    	default:
    		throw JsonStyleMetadataError("Unexpected type from JSON parser.");
    }
    return(JSMDNULL);
}
JsonStyleMetadata jsonread(string fname)
{
    try {
        JsonStyleMetadata result(fname);
        return(result);
    }
    catch(...){throw;};
}
JsonStyleMetadata::JsonStyleMetadata(string fname)
{
	try {
		ifstream doc(fname.c_str(), ifstream::binary);
		Json::Value root;   // will contains the root value after parsing
		Json::Reader reader;
		bool parsingSuccessful = reader.parse(doc, root, false);
		if (!parsingSuccessful)
			throw JsonStyleMetadataError("Failed to parse the JSON file.");
		JsonStyleMetadata all_the_stuff(root);
		this->mreal=all_the_stuff.mreal;
		this->mint=all_the_stuff.mint;
		this->mbool=all_the_stuff.mbool;
		this->mstring=all_the_stuff.mstring;
		this->marray=all_the_stuff.marray;
		this->mbranch=all_the_stuff.mbranch;
	}catch(...){throw;};
}
JsonStyleMetadata::JsonStyleMetadata(Value root)
{
	try {
		if(root.size()<=0)
			throw JsonStyleMetadataError("The input JSON document is empty.");
		for(Json::ValueIterator itr = root.begin(); itr != root.end(); itr++)
		{
			JsonStyleInputType jt=jsonmd_type(*itr);
			switch(jt)
			{
				case JSMDINT:
					this->put(itr.key().asString(),long((*itr).asLargestInt()));
					break;
				case JSMDUINT:
					if((*itr).asLargestUInt()<=LONG_MAX)
						this->put(itr.key().asString(),long((*itr).asLargestUInt()));
					else
						this->put(itr.key().asString(),double((*itr).asLargestUInt()));
					break;
				case JSMDREAL:
					this->put(itr.key().asString(),double((*itr).asDouble()));
					break;
				case JSMDSTR:
					this->put(itr.key().asString(),string((*itr).asString()));
					break;
				case JSMDBOOL:
					this->put(itr.key().asString(),bool((*itr).asBool()));
					break;
				default:
					if(jt==JSMDOBJ)
					{
						JsonStyleMetadata child(*itr);
						mbranch.insert(pair<string,JsonStyleMetadata>(itr.key().asString(),child));;
					}
					else if(jt==JSMDARR)
					{
						list<Value> subarray;
						for(Json::ValueIterator itr2 = (*itr).begin(); itr2 != (*itr).end(); itr2++)
							subarray.push_back(*itr2);
						marray.insert(pair<string,list<Value> >(itr.key().asString(),subarray));;
					}
					else
						throw JsonStyleMetadataError(itr.key().asString()+" is defined as null.");
			}
		}
	}catch(...){throw;};
}
JsonStyleMetadata::JsonStyleMetadata(const JsonStyleMetadata& parent)
    : Metadata(parent)
{
    marray=parent.marray;
    mbranch=parent.mbranch;
}
list<Value> JsonStyleMetadata::get_array(const string key)
{
    map<string,list<Value> >::iterator iptr;
    iptr=marray.find(key);
    if(iptr==marray.end()) throw JsonStyleMetadataError(
            "get_array failed trying to find data for key="+key);
    return(marray[key]);
}
JsonStyleMetadata JsonStyleMetadata::get_branch(const string key)
{
    map<string,JsonStyleMetadata>::iterator iptr;
    iptr=mbranch.find(key);
    if(iptr==mbranch.end()) throw JsonStyleMetadataError(
            "get_branch failed trying to find data for key="+key);
    return(mbranch[key]);
}
list<string> JsonStyleMetadata::branch_keys()
{
    map<string,JsonStyleMetadata>::iterator iptr;
    list<string> result;
    for(iptr=mbranch.begin();iptr!=mbranch.end();++iptr)
        result.push_back((*iptr).first);
    return(result);
}
list<string> JsonStyleMetadata::array_keys()
{
    map<string,list<Value> >::iterator iptr;
    list<string> result;
    for(iptr=marray.begin();iptr!=marray.end();++iptr)
        result.push_back((*iptr).first);
    return(result);
}
JsonStyleMetadata& JsonStyleMetadata::operator=(const JsonStyleMetadata& parent)
{
    if(this!=&parent)
    {
        /* These are protected members of Metadata.  There is a trick to
           dynamic_cast operator= in some situations but could not 
           reproduce it and make it work here. Probably because
           Metadata has no virtual members */
        mreal=parent.mreal;
        mint=parent.mint;
        mbool=parent.mbool;
        mstring=parent.mstring;
        marray=parent.marray;
	mbranch=parent.mbranch;
    }
    return(*this);
}
MetadataList get_mdlist(SEISPP::JsonStyleMetadata& m, const string tag)
{
	try {
		MetadataList mdl;
		Metadata_typedef mdt;
		string mdtype;
		JsonStyleMetadata t=m.get_branch(tag);
		MetadataList tl=t.keys();
		list<Metadata_typedef>::iterator tptr;
		for(tptr=tl.begin();tptr!=tl.end();++tptr)
		{
			mdt.tag = (*tptr).tag;
			mdtype=t.get_string((*tptr).tag);
			if(mdtype=="real" || mdtype=="REAL")
				mdt.mdt = MDreal;
			else if(mdtype=="int" || mdtype=="INT" || mdtype=="integer")
				mdt.mdt = MDint;
			else if(mdtype=="string" || mdtype=="STRING")
				mdt.mdt = MDstring;
			else if(mdtype=="boolean" || mdtype=="BOOLEAN")
				mdt.mdt = MDboolean;
			else
			{
				cerr << "get_mdlist:  Warning type keyword = "
				    << mdtype << " not recognized."<<endl
				    << mdt.tag <<" attribute in input list ignored"
				    <<endl;
				continue;
			}

			mdl.push_back(mdt);
		}
		return(mdl);
	}catch(...){throw;};
}

} // End SEISPP Namespace declaration 
