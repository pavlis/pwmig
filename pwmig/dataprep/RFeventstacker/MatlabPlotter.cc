#include <sstream>
#include "MatlabPlotter.h"
namespace SEISPP 
{
using namespace std;
using namespace SEISPP;

MatlabPlotter::MatlabPlotter() : MatlabProcessor()
{
	for(int k=0;k<3;++k) fn3ce[k]=k+1;
	fntse=4;
	fnts=5;
	fntcs=6;

}
void MatlabPlotter::wigbplot(ThreeComponentEnsemble& d,string chans[3],bool plot_image)
{
    try {
	char buf[256];
	ostringstream ss(buf);
	string command;
	this->load(d,chans);
	ss.clear();
	ss<<"figure("<<fn3ce[0]<<");"<<endl;
	command=ss.str();
	this->process(command);
	command="clf;wigb("+chans[0]+string(");");
	this->process(command);
	if(plot_image)
	{
		command="imagesc("+chans[0]+string(");");
		this->process(command);
	}
	command="title '"+chans[0]+"'";
	this->process(command);

	ss.clear();
	ss<<"figure("<<fn3ce[1]<<");"<<endl;
	command=ss.str();
	this->process(command);
	command="clf;wigb("+chans[1]+string(");");
	this->process(command);
	if(plot_image)
	{
		command="imagesc("+chans[1]+string(");");
		this->process(command);
	}
	command="title '"+chans[1]+"'";
	this->process(command);

	ss.clear();
	ss<<"figure("<<fn3ce[2]<<");"<<endl;
	command=ss.str();
	this->process(command);
	command="clf;wigb("+chans[2]+string(");");
	this->process(command);
	if(plot_image)
	{
		command="imagesc("+chans[2]+string(");");
		this->process(command);
	}
	command="title '"+chans[2]+"'";
	this->process(command);
	
    }
    catch(...){throw;};
}
void MatlabPlotter::wigbplot(TimeSeriesEnsemble& d,bool plot_image)
{
    try {
	this->load(d,string("d"));
	char buf[256];
	ostringstream ss(buf);
	string command;
	ss.clear();
	ss<<"figure("<<fntse<<");"<<endl;
	command=ss.str();
	this->process(command);
	command="wigb(d);";
	this->process(command);
	if(plot_image)
	{
		command="imagesc(d);";
		this->process(command);
	}
	command="title 'TimeSeriesEnsemble data'";
	this->process(command);
    }catch(...){throw;};
}
void MatlabPlotter::plot(ThreeComponentSeismogram& d)
{
	try {
		this->load(d,string("d"));
		char buf[256];
		ostringstream ss(buf);
		string command;
		ss.clear();
		ss<<"figure("<<fntcs<<");"<<endl;
		command=ss.str();
		this->process(command);
		command=string("subplot(3,1,1), plot(d(1,:));")
			+ string("subplot(3,1,2),plot(d(2,:));")
			+ string("subplot(3,1,3),plot(d(3,:));");
		this->process(command);
		command="title 'ThreeComponentSeismogram data';";
		this->process(command);
	}catch(...){throw;};
}
void MatlabPlotter::plot(TimeSeries& d)
{
	try {
		this->load(d,string("d"));
		char  buf[256];
		ostringstream ss(buf);
		string command;
		ss.clear();
		ss<<"figure("<<fntcs<<");"<<endl;
		command=ss.str();
		this->process(command);
		command="plot(d);";
		this->process(command);
		command="title 'TimeSeries data';";
		this->process(command);
	}catch(...){throw;};
}
} // End SEISPP namespace declaration
