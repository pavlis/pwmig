BIN=migsimulation
PF=migsimulation.pf
# F77LIBS required by layered model simulator 
ldlibs=-lseispp -lsynthetic -ltrvltm -lbrttutil $(TRLIBS) $(DBLIBS)  \
       $(F77LIBS) \
               -lperf -lgclgrid -lpwmig -lseispp

include $(ANTELOPEMAKE) 
CXXFLAGS += -g

OBJS=migsimulation.o 
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
