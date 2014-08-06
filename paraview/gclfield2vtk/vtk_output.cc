#include <iostream>
#include "vtk_output.h"
#include "gclgrid.h"

using namespace std;

/* g is the grid to be converted.   

   tag is a string vector whose first component is uses as the name field for the
   scalars field in the output data file.   It is a vector to allow polymorphic
   implementaiton of the writer later in this file.  Only the first element of the
   tag vector is accessed by this procedure.*/
vtkStructuredGrid*	convert_gcl3d_to_vtksg(GCLscalarfield3d &g, 
        string name, vector<string> tag)
{

	vtkStructuredGrid	*pGrid 			= vtkStructuredGrid::New();
	vtkPoints		*pPoints		= vtkPoints::New();
	vtkDoubleArray		*pData			= vtkDoubleArray::New();

	pGrid->SetDimensions(g.n1, g.n2, g.n3);

	pData->SetNumberOfTuples(g.n1 * g.n2 * g.n3);
	pData->SetNumberOfComponents(1);
        /* this may not be necessary */
        pData->SetComponentName(0,tag[0].c_str());
        pData->SetName(name.c_str());

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
/* g is the vectofield data to be converted.  
   tags is the a vector of strings to set as the component names in the vtk array.   
   length of tags array must match g.nv.  */
vtkStructuredGrid*	convert_gcl3d_to_vtksg(GCLvectorfield3d &g,string name,
        vector<string> tags)
{

	vtkStructuredGrid	*pGrid 			= vtkStructuredGrid::New();
	vtkPoints		*pPoints		= vtkPoints::New();
	vtkDoubleArray		*pData			= vtkDoubleArray::New();

	pGrid->SetDimensions(g.n1, g.n2, g.n3);

	pData->SetNumberOfComponents(g.nv);
	pData->SetNumberOfTuples(g.n1 * g.n2 * g.n3);
        /* This name gets set as the "Scalars_" field that prepends component
           names */
        pData->SetName(name.c_str());
	int k,kk,l;
        string null_name("component");
        if( (g.nv) != (tags.size()) )
            cerr << "Warning (convert_gcl3d_to_vtksg):  Mismatch in vector field component names"
                                                        <<endl
                << "Field be converted has "<<g.nv<<" components"<<endl
                << "Received a vector of strings of length "<<tags.size()<<endl
                << "Any undefined names will get the tag = "<<null_name<<endl;
        for(k=0;k<g.nv;++k)
        {
            if(k<tags.size())
            {
                pData->SetComponentName(k,tags[k].c_str());
            }
            else
            {
                pData->SetComponentName(k,null_name.c_str());
            }
        }

	for(k=g.n3-1,kk=0;k>=0;k--,kk++)
	{
		for(int j = 0; j < g.n2; j++)
		{
			for(int i = 0; i < g.n1; i++)
			{
				pPoints->InsertNextPoint(g.x1[i][j][k], 
					g.x2[i][j][k], g.x3[i][j][k]);

                                for(l=0;l<g.nv;++l)
                                    pData->SetComponent((kk*g.n1*g.n2)+(j*g.n1)+i,
                                            l,g.val[i][j][k][l]);
				
			}
		}
	}

	pGrid->SetPoints(pPoints);
	pGrid->GetPointData()->SetScalars(pData);

	pPoints->Delete();
	pData->Delete();

	return(pGrid);
}
