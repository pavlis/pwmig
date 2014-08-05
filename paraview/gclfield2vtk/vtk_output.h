#if !defined(_vtk_output_h_)
#define _vtk_output_h_


class vtkStructuredGrid; 	// forward reference

/*!/ \brief Convert a GCLscalarfield3d object to a VTK Structured Grid written to disk.

 \param g The grid object to be written.  Polymorphism assumed as their are converters
       for both vector and scalar objects.
 \param filename Filename to write to.
 \param use_xml Boolean flag to determine if the new XML VTK format should be used. (default = false)
 \param swap_xz Swap X/Z values to fit a "z out" coordinate arrangement. (default = true)

*/
void	output_gcl3d_to_vtksg(GCLgrid3d &g, const string filename, vector<string> tags, 
        bool use_xml = false,bool use_binary=true);

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

#endif
