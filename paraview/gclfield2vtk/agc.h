#ifndef _SEISPP_AGC_H_
#define _SEISPP_AGC_H_
#include <vector>
namespace SEISPP
{
/*! Apply an agc operator to a vector of data.

Automatic Gain Control (agc) is a commonly used in seismic processing. 
AGC works by computing a moving window average rms and scaling all data
in the vector to have a uniform amplitude in time gates of that length.

This is a general template to implement agc on a vector of data of 
any numerical type.  In general, however, it should only be trusted to
work correctly on float or double vectors since the algorithm is 
intrinsically floating point.

This routine will silently do nothing returning a copy of the original
vector if iwagc is greater than the length of the data vector or if iwagc<=0.

This procedure was modified from a function called do_agc in the Seismic
Unix program sugain.  do_agc was plain C and used standard arrays.  The
author modified this to use the STL library.  This will be somewhat slower
than the old SU program because it returns a copy of data rather than 
manipulate the data vector directly.  

\param data is the input vector of data
\param iwagc is the agc window length in samples
\author Gary L. Pavlis
*/

template <class T> 
	vector<T>  agc(vector<T> data, int iwagc)
{
	int nt=data.size();
	if( (nt<iwagc) || (iwagc<=0) ) return(data);
	vector<T> agcdata;
	int i;
	T val, sum, rms;
	int nwin;
	agcdata.reserve(nt);
	T tempval;
	
        /* compute initial window for first datum */
        sum = 0.0;
        for (i = 0; i < iwagc+1; ++i) {
                val = data[i];
                sum += val*val;
        }
        nwin = 2*iwagc+1;
        rms = sum/nwin;
	/* This was original SU code for this using C arrays. Replaced
	with STL vector push_back manipulation */
        //agcdata[0] = (rms <= 0.0) ? 0.0 : data[0]/sqrt(rms);
        tempval = (rms <= 0.0) ? 0.0 : data[0]/sqrt(rms);
	agcdata.push_back(tempval);

        /* ramping on */
        for (i = 1; i <= iwagc; ++i) {
                val = data[i+iwagc];
                sum += val*val;
                ++nwin;
                rms = sum/nwin;
                tempval = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
		agcdata.push_back(tempval);
        }

        /* middle range -- full rms window */
        for (i = iwagc + 1; i <= nt-1-iwagc; ++i) {
                val = data[i+iwagc];
                sum += val*val;
                val = data[i-iwagc];
                sum -= val*val; /* rounding could make sum negative! */
                rms = sum/nwin;
                tempval = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
		agcdata.push_back(tempval);
        }

        /* ramping off */
        for (i = nt - iwagc; i <= nt-1; ++i) {
                val = data[i-iwagc];
                sum -= val*val; /* rounding could make sum negative! */
                --nwin;
                rms = sum/nwin;
                tempval = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
		agcdata.push_back(tempval);
        }

	/* Return modified data */
	return(agcdata);
}


} // end SEISPP namespace encapsulation
#endif
