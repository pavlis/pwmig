#include <vtkStructuredGrid.h>
#include <vtkStructuredGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <string>
#include <iostream>
#include "gclgrid.h"

using namespace std;

vtkStructuredGrid*	convert_gcl3d_to_vtksg(GCLscalarfield3d &g)
{

	vtkStructuredGrid	*pGrid 			= vtkStructuredGrid::New();
	vtkPoints		*pPoints		= vtkPoints::New();
	vtkDoubleArray		*pData			= vtkDoubleArray::New();

	pGrid->SetDimensions(g.n1, g.n2, g.n3);

	pData->SetNumberOfTuples(g.n1 * g.n2 * g.n3);
	pData->SetNumberOfComponents(1);

	int kk;
	for(int k=g.n3-1,kk=0;k>=0;k--,kk++)
	{
		for(int j = 0; j < g.n2; j++)
		{
			for(int i = 0; i < g.n1; i++)
			{
				pPoints->InsertNextPoint(g.x1[i][j][k], 
					g.x2[i][j][k], g.x3[i][j][k]);

				pData->SetValue((kk*g.n1*g.n2)+(j*g.n1)+i,
					g.val[i][j][k]);
				
			}
		}
	}

	pGrid->SetPoints(pPoints);
	pGrid->GetPointData()->SetScalars(pData);

	pPoints->Delete();
	pData->Delete();

	return(pGrid);
}


void output_gcl3d_to_vtksg(GCLscalarfield3d &g, const string filename,
		bool use_xml = false, bool use_binary=true)
{
	
	vtkStructuredGrid *pGrid = 0; 	

	pGrid = convert_gcl3d_to_vtksg(g);
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



