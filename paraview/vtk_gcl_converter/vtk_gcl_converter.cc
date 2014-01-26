#include <iostream>
#include <vector>
#include <list>
#include "seispp.h"
#include "gclgrid.h"
#include "RegionalCoordinates.h"
class BoundingBox
{
public:
    BoundingBox(double x1l,double x1h,double x2l,double x2h,
            double x3l,double x3h);
    bool inside(Cartesian_point cp);
private:
    double x1low,x1high;
    double x2low,x2high;
    double x3low,x3high;
};
BoundingBox::BoundingBox(double x1l, double x1h,
        double x2l,double x2h,double x3l,double x3h)
{
    x1low=x1l;
    x1high=x1h;
    x2low=x2l;
    x2high=x2h;
    x3low=x3l;
    x3high=x3h;
}
bool BoundingBox::inside(Cartesian_point cp)
{
	if(cp.x1>x1high) return(false);
	if(cp.x1<x1low) return(false);
	if(cp.x2>x2high) return(false);
	if(cp.x2<x2low) return(false);
	if(cp.x3>x3high) return(false);
	if(cp.x3<x3low) return(false);
	return(true);
}

using namespace std;
using namespace SEISPP;
enum ObjectType{POINTS,LINES,POLYGONS};
void usage()
{
	cerr << "vtk_gcl_converter [ [-lines|polygons] -2d -noz -clip -pf pffile] < infile "<<endl
		<< "Converted data written to stdout.  Default format points"<<endl;
	exit(-1);
}

bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
        ObjectType objtype(POINTS);
	bool noz(false);
	bool clip(false);
	int i;
        string pffile("vtk_gcl_converter");
	for(i=1;i<argc;++i)
	{
		string testarg(argv[i]);
		if(testarg=="-lines")
			objtype=LINES;
                else if(testarg=="-polygons")
                        objtype=POLYGONS;
		else if(testarg=="-noz")
			noz=true;
		else if(testarg=="-clip")
			clip=true;
		else if(testarg=="-pf")
                {
                    ++i;
                    if(i>=argc) usage();
                    pffile=string(argv[i]);
                }
		else
			usage();
	}
	try {
                Pf *pf;
                if(pfread(const_cast<char *>(pffile.c_str()),&pf))
                {
                    cerr << "pfread failed on file="<<pffile<<endl
                        << "Cannot continue"<<endl;
                    exit(-1);
                }
                Metadata control(pf);
                double olat,olon,oradius,azimuth_y;
                olat=control.get_double("origin_latitude");
                olon=control.get_double("origin_longitude");
                oradius=control.get_double("origin_radius");
                azimuth_y=control.get_double("azimuth_y_axis");
                olat=rad(olat);
                olon=rad(olon);
                azimuth_y=rad(azimuth_y);
                RegionalCoordinates coords(olat,olon,oradius,azimuth_y);
                /* Construct a bounding box from parameter file data */
                double x1l=control.get_double("x1low");
                double x1h=control.get_double("x1high");
                double x2l=control.get_double("x2low");
                double x2h=control.get_double("x2high");
                double x3l=control.get_double("x3low");
                double x3h=control.get_double("x3high");
                BoundingBox bb(x1l,x1h,x2l,x2h,x3l,x3h);
		vector<Cartesian_point> cplist;
		Geographic_point gp;
		Cartesian_point cp;
		double lat,lon,depth,radius;
		/* First write common data for either points or lines for file head */
		cout << "# vtk DataFile Version 2.0"<<endl
                  << argv[0] << " conversion"  <<endl
                  << "ASCII" <<endl
                  << "DATASET POLYDATA"<<endl;
		typedef list<int> LineLinks;
		LineLinks thissegment;
		list<LineLinks> segments;
		char line[256];
		int i=0;
                switch(objtype)
                {
                case LINES:
                case POLYGONS:
			while(!cin.eof())
			{
				cin.getline(line,256);
				if(cin.eof())
				{
					/*this is an odd logic construct, but
					we make this look like a > line to force
					the last segment to be pushed to the segments
					list.  We make sure the size isn't zero */
					if(thissegment.size()>1)
					{
						line[0]='>';
						line[1]='\0';
					}
				}
				if(line[0]=='#') continue;
				if(line[0]=='>')
				{
					if(thissegment.size()>1)
					{
						segments.push_back(thissegment);
						thissegment.clear();
					}
					else if(thissegment.size()==1)
					{
						/* Handle special case possible when clipping.
						This is if a line wanders in and out of the bounding
						box it is possible to end up with a stray point we
						need to clear like this. */
						thissegment.clear();
					}
				}
				else
				{
					if(noz)
					{
						sscanf(line,"%lf%lf",&lon,&lat);
						depth=0.0;  // inefficient but clearer
					}
					else
					{
						sscanf(line,"%lf%lf%lf",&lon,&lat,&depth);
					}
					gp.lat=rad(lat);
					gp.lon=rad(lon);
					gp.r=r0_ellipse(gp.lat)-depth;
                                        cp=coords.cartesian(gp);
					if(clip)
					{
						if(bb.inside(cp))
						{
							cplist.push_back(cp);
							thissegment.push_back(i);
							++i;
						}
						else
						{
							/* truncate segment if it
							wanders outside box. 
							drop if only one point */

							if(thissegment.size()>1)
							   segments.push_back(thissegment);
							thissegment.clear();
						}
					}
					else
					{
						cplist.push_back(cp);
						thissegment.push_back(i);
						++i;
					}
				}
			} 
                        break;
                case POINTS:
                default:
		    while(cin.getline(line,256))
		    {
		    	if(noz)
			{
				sscanf(line,"%lf%lf",&lon,&lat);
				depth=0.0;
			}
			else
			{
				sscanf(line,"%lf%lf%lf",&lon,&lat,&depth);
			}
			gp.lat=rad(lat);
			gp.lon=rad(lon);
			gp.r=r0_ellipse(gp.lat)-depth;
                        cp=coords.cartesian(gp);
			if(clip)
			{
				if(bb.inside(cp))
					cplist.push_back(cp);
			}
			else
				cplist.push_back(cp);
                    }
		}

		cout << "POINTS " << cplist.size() << " float"<<endl;
		vector<Cartesian_point>::iterator cpptr;
		for(cpptr=cplist.begin();cpptr!=cplist.end();++cpptr)
		{
			cout << cpptr->x1 << " "
				<< cpptr->x2<<" "
				<< cpptr->x3<<" "<<endl;
		}
		int sizepar;
		list<LineLinks>::iterator llptr;
		LineLinks::iterator lineptr;
                switch(objtype)
		{
                case POLYGONS:
                case LINES:
                default:
			/* For this stupid format we have to count the total number
			of items in segments and number of points for the "size" parameter
			of the LINES keyword line. */
			sizepar=0;
			for(llptr=segments.begin();llptr!=segments.end();++llptr)
			{
				for(lineptr=llptr->begin();lineptr!=llptr->end();++lineptr)
					++sizepar;
			}
			sizepar+=segments.size();
                        switch(objtype)
                        {
                            case POLYGONS:
                                cout << "POLYGONS ";
                                break;
                            case LINES:
                            default:
			        cout <<  "LINES "; 
                        }
                        cout << segments.size() << " " <<sizepar<<endl;
			for(llptr=segments.begin();llptr!=segments.end();++llptr)
			{
				cout << llptr->size() << " ";
				for(lineptr=llptr->begin();lineptr!=llptr->end();++lineptr)
				{
					cout << *lineptr<<" ";
				}
				cout << endl;
			}
		}
	} catch (SeisppError& serr)
	{
		serr.log_error();
		exit(-1);
	}
}
