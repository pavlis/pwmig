#include <vector>
#include <string>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include "gclgrid.h"

#if !defined(_vtk_output_h_)
#define _vtk_output_h_


class vtkStructuredGrid; 	// forward reference

/*! \brief Convert a GCLscalarafield3d object to a vtkStructuredGrid object

\param g The grid object to convert from.
\param tags is assumed to contain a single string component that is the name assigned
   to the scalars field of the output.
*/
vtkStructuredGrid *convert_gcl3d_to_vtksg(GCLscalarfield3d &g,vector<string> tags);
/*! \brief Convert a GCLvectorafield3d object to a vtkStructuredGrid object

\param g The grid object to convert from.
\param tags is a vector of strings that tag the attributes in each component of the
   vector field that is to be converted.  The size of this vector should match the
   nv attribute of the vector field.   This is not a fatal error but will generate 
   a warning if the sizes do that match.
*/
vtkStructuredGrid *convert_gcl3d_to_vtksg(GCLvectorfield3d &g, vector<string> tags);
template <class Tgrid> vtkStructuredGrid *convert_gcl3d_to_vtksg(Tgrid& g,
        vector<string> tags)
{
}

/*!/ \brief Convert data stored in a GCLgrid3d structured to and write to vtk file.

This template defines the core functionality of the the gclfield2vtk program.   
It takes a field object as input, converts it to VTKs structured grid object,
and then calls the VTK write method to save teh file.   This is a template
because VTK stores scalars and vectors in coherent data structures defined
inte vtkStructuredGrid object.   At present Tgrid should be either 
GCLscalarfield3d or GCLvectorfield3d.  

 \param g The grid object to be written. 
 \param filename Filename to write to.
 \param tags are component names assigned to each component of the field.  Vector should
    be of length 1 for scalars to avoid warnings about length mismatch.
 \param use_xml Boolean flag to determine if the new XML VTK format should be used. (default = false)
 \param swap_xz Swap X/Z values to fit a "z out" coordinate arrangement. (default = true)

*/
template <class Tgrid> void output_gcl3d_to_vtksg(Tgrid& g, const string filename,
        vector<string> tags, bool use_xml=true, bool use_binary=false)
{
	
	vtkStructuredGrid *pGrid = 0; 	

	pGrid = convert_gcl3d_to_vtksg(g,tags);
	if(use_xml)
	{
		vtkXMLStructuredGridWriter *pWriter
			= vtkXMLStructuredGridWriter::New();
		pWriter->SetFileName(filename.c_str());
		pWriter->SetInput(pGrid);
		pWriter->Write();
		pWriter->Delete();
	}
	else
	{
		vtkStructuredGridWriter *pWriter
			= vtkStructuredGridWriter::New();
		pWriter->SetFileName(filename.c_str());
		pWriter->SetFileTypeToBinary();
		pWriter->SetInput(pGrid);
		pWriter->Write();
		pWriter->Delete();
	}
	pGrid->Delete();
}


#endif
