#if !defined(_vtk_output_h_)
#define _vtk_output_h_


class vtkStructuredGrid; 	// forward reference

// @{
// Convert a GCLscalarfield3d object to a VTK Structured Grid written to disk.
//
// @param g The grid object to convert from.
// @param filename Filename to write to.
// @param use_xml Boolean flag to determine if the new XML VTK format should be used. (default = false)
// @param swap_xz Swap X/Z values to fit a "z out" coordinate arrangement. (default = true)
//
// @}
void	output_gcl3d_to_vtksg(GCLscalarfield3d &g, const string filename, bool use_xml = false,
						 bool use_binary=true);

// @{
// Convert a GCLscalarafield3d object to a vtkStructuredGrid object
//
// @param g The grid object to convert from.
// @}
vtkStructuredGrid *convert_gcl3d_to_vtksg(GCLscalarfield3d &g);

#endif
