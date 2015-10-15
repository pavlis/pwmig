#include <iostream>
#include <fstream>
#include <string>
#include "gclgrid.h"
using namespace std;
int vtk_output_points(GCLgrid& g,ofstream& out)
{
    try{
        int i,j;
	out << "# vtk DataFile Version 2.0"<<endl
		<< g.name <<endl
		<< "ASCII" <<endl
		<< "DATASET POLYDATA"<<endl
		<< "POINTS " << (g.n1)*(g.n2) <<" float"<<endl;
	for(j=0;j<g.n2;++j)
		for(i=0;i<g.n1;++i)
		{
			out << g.x1[i][j]
				<< " "
				<< g.x2[i][j]
				<< " "
				<< g.x3[i][j] 
				<< endl;
		}
    }catch(...){throw;};
}
int vtk_output_GCLgrid(GCLgrid& g,string fname,string scalars_tag)
{
	int icell,i,j;
	ofstream out;
	out.open(fname.c_str(),ios::out);
        try{
            vtk_output_points(g,out);
        }catch(...)
        {
            cerr << "vtk_output_GCLgrid failed writing point data.  Fatal, exiting."
                <<endl;
            exit(-1);
        }
	// Documentation says number of polygons and then
	// "sizeof the cell list".  Since polygons here are 
	// all squares this to reduce to 5*number polygons
	out << "POLYGONS " << (g.n2 - 1)*(g.n1 - 1) 
		<< " " << (g.n2 - 1)*(g.n1 - 1)*5 <<endl;

	for(j=0,icell=0;j<g.n2-1;++j)
		for(i=0;i<g.n1-1;++i,++icell)
		{
			out << "4 "
				<< j*g.n1 + i << " "
				<< j*g.n1 + i + 1<< " "
				<< (j+1)*g.n1 + i + 1<< " "
				<< (j+1)*g.n1 + i << endl;
		}
	// Now the data section.  Eventually we should allow cast to 
	// scalar field but for now we hard code elev.  There is an off
	// by one thing here as the polygons are defined by 4 vertices.
	// Here I use a simple (incorrect) algorithm and use the lower
	// left corner elevation without worrying about misregistration by
	// a half grid cell.  These will only determine color anyway.
	//out << "CELL_DATA " << icell << endl
		out << "POINT_DATA "<< (g.n1)*(g.n2) <<endl
		<< "SCALARS "<<scalars_tag <<" double"<<endl
		<< "LOOKUP_TABLE default"<<endl;

	for(j=0;j<g.n2;++j)
		for(i=0;i<g.n1;++i)
		{
                    /* this is both inefficient and not elegant.   
                       The idea is to reverse the sign of the depth
                       of a point when elevation is desired.  If the
                       tag is anything else the assumption is depth*/
                    if(scalars_tag=="Elevation")
			out <<	-g.depth(i,j) <<endl;
                    else
                        out << g.depth(i,j)<<endl;
		}
	
	out.close();
	return(icell);
}
/* This is a (painfully) parallel routine for 2D scalar fields.   A far
   more elegant solution for this is to use polymorphism, but I took the 
   easy way out and copied the above and made minor modifications for field data.
   */
int vtk_output_GCLgrid(GCLscalarfield& g,string fname,string scalars_tag)
{
	int icell,i,j;
	ofstream out;
	out.open(fname.c_str(),ios::out);
        try{
            vtk_output_points(g,out);
        }catch(...)
        {
            cerr << "vtk_output_GCLgrid failed writing point data.  Fatal, exiting."
                <<endl;
            exit(-1);
        }
	// Documenation says number of polygons and then
	// "sizeof the cell list".  Since polygons here are 
	// all squares this seems to reduce to 5*number polygons
	out << "POLYGONS " << (g.n2 - 1)*(g.n1 - 1) 
		<< " " << (g.n2 - 1)*(g.n1 - 1)*5 <<endl;

	for(j=0,icell=0;j<g.n2-1;++j)
		for(i=0;i<g.n1-1;++i,++icell)
		{
			out << "4 "
				<< j*g.n1 + i << " "
				<< j*g.n1 + i + 1<< " "
				<< (j+1)*g.n1 + i + 1<< " "
				<< (j+1)*g.n1 + i << endl;
		}
	// Now the data section.  
		out << "POINT_DATA "<< (g.n1)*(g.n2) <<endl
		<< "SCALARS "<<scalars_tag <<" double"<<endl
		<< "LOOKUP_TABLE default"<<endl;

	for(j=0;j<g.n2;++j)
		for(i=0;i<g.n1;++i)
		{
			out <<	g.val[i][j]<<endl;
		}
	
	out.close();
	return(icell);
}
