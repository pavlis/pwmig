//@{
// Computes a GCLgrid defined field (raygrid) for an incident P wavefield.  
// The result is travel times from a given hypocenter to points inside a GCLgrid3d
// object.  The times are computed from a 1d refernce model to the base of the model
// and then using approximate ray tracing from the base up.  
// Actual algorithm computes times to the surface and then subtracts path integral times
// to get 3d travel times at points in the grid.  The grid geometry is defined by
// incident wave rays traced with a 1d reference model.  That is, they are like the
// S wavefield raygrid used in the main program of pwmig.
// 
// @param pstagrid parent pseudostation grid used to build this grid.
// @param border_pad is defines additional padding used to extend the pseudostation
//     grid on which the data are defined.  This is necessary (essential really) because
//     in pwmig this grid is used for all plane wave components while the S grids are
//     different for each plane wave component.  This should be made large enough that
//     it can contain all scattering points connected to S rays from the pseudostationh
//     grid.  
// @param UP3d P wave slowness defined with a 3d field object
// @param vp1d P wave 1d reference model.  Used to trace traces
// @param u0 inident wave slowness
// @param h hypocenter of source that created wavefield being imaged
// @param zmax ray trace parameters passed directly to BuildGCLraygrid
// @param tmax ray trace parameters passed directly to BuildGCLraygrid
// @param dt ray trace parameters passed directly to BuildGCLraygrid
//
//@returns auto_ptr object containing the travel time field
//@throws catches all exceptions and passes them along.  Several functions
//      called in this procedure could throw several possible exceptions I don't
//      know well which is the reason for the catch all syntax.
//
//@}
auto_ptr<GCLscalarfield3d> ComputeIncidentWaveRaygrid(GCLgrid& pstagrid,
				int border_pad,
					GCLscalarfield& UP3d,
						VelocityModel_1d vp1d,
							SlownessVector u0,
								Hypocenter h,
									double zmax,
									double tmax, 
									double dt)
{
	int i,j,k;
	// First set up the new expanded grid.  The new grid has border_pad cells
	// added on each side of the parent grid (pstagrid).  
	int n1new, n2new;
	int i0new, j0new;
	n1new = pstagrid.n1 + 2*border_pad;
	n2new = pstagrid.n2 + 2*border_pad;
	i0new = pstagrid.i0 + border_pad;
	j0new = pstagrid.j0 + border_pad;
	try {
		// this assumes the pstagrid is a "regular" gclgrid created with this same constructor.
		// Otherwise there may be small disconnect, although it should not really matter
		// provided dx1_nome and dx2_nom approximate the parent well.
		GCLgrid ng2d(n1new, n2new, string("ng2d"),
				pstagrid.lat0,pstagrid.lon0,pstagrid.r0,
				pstagrid.azimuth_y, pstagrid.dx1_nom, pstagrid.dx2_nom, i0new, j0new);
		// Use ng2d to compute a 1d reference model travel time grid
		// Note we explicitly do no include elevation in this calculation assuming this
		// will be handled with statics at another point
		GCLscalarfield Tp0(ng2d);
		for(i=0;i<Tp0.n1;++i)
		{
			for(j=0;j<Tp0.n2;++j)
			{
				Tp0.val[i][j]=h.ptime(Tp0.lat(i,j),Tp0.lon(i,j),0.0);
			}
		}
		auto_ptr<GCLgrid3d> IncidentRaygrid(Build_GCLraygrid(false, ng2d, u0,h,Vp1d,zmax,tmax,dt));
		auto_ptr<GCLscalarfield3d> Tp(new GCLscalarfield3d(*IncidentRaygrid));
		int kk, kend=Tp->n3 - 1;
		for(i=0;i<Tp->n1;++i)
		{
			for(j=0;j<Tp->n2;++j)
			{
				auto_ptr<dmatrix> path(extract_gridline(IncidentRaygrid,
							i,j,0,3,true);
				// this is the pwmig function using 1d if path is outside 3d model
				vector<double>Ptimes=compute_gridtime(UP3d,i,j,Vp1d,*IncidentRaygrid,path);
				for(k=0,kk=kend;k<kend;++k,--kk)
					Tp->val[i][j][k] = Tp0[i][j] - times(kk);
				path.release();
			}
		}
		return Tp;
	} catch (...) throw;
}
