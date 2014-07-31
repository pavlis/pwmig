BIN=vtk_gcl_converter

MAN1=vtk_gcl_converter.1
PF=vtk_gcl_converter.pf
ldlibs=-lm -lgeocoords -lseispp -lgclgrid -lperf $(DBLIBS) 

include $(ANTELOPEMAKE)
OBJS=vtk_gcl_converter.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
