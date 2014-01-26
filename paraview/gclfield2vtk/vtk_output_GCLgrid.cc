#include <iostream>
#include <fstream>
#include <string>
#include "gclgrid.h"
using namespace std;
int vtk_output_GCLgrid(GCLgrid& g,string fname)
{
	int icell,i,j;
	ofstream out;
	out.open(fname.c_str(),ios::out);
	// Header block
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
	// Now the data section.  Eventually we should allow cast to 
	// scalar field but for now we hard code elev.  There is an off
	// by one thing here as the polygons are defined by 4 vertices.
	// Here I use a simple (incorrect) algorithm and use the lower
	// left corner elevation without worrying about misregistration by
	// a half grid cell.  These will only determine color anyway.
	//out << "CELL_DATA " << icell << endl
		out << "POINT_DATA "<< (g.n1)*(g.n2) <<endl
		<< "SCALARS scalars double"<<endl
		<< "LOOKUP_TABLE default"<<endl;

	for(j=0;j<g.n2;++j)
		for(i=0;i<g.n1;++i)
		{
			out <<	-g.depth(i,j) <<endl;
		}
	
	out.close();
	return(icell);
}
