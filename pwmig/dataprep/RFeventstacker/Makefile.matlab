BIN=RFeventstacker
PF=RFeventstacker.pf
DATADIR=schemas/css3.0.ext
DATA=stackstats
INCLUDE=MatlabPlotter.h

#RPATH="-Wl,--rpath-link,$TMW_ROOT/extern/lib/$Arch,--rpath-link,$TMW_ROOT/bin/$Arch"
ldlibs= -lpwmig -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil \
  $(MATLABLIB)  -lmatlabprocessor -leng -lmx -lut -lmat \
  $(TRLIBS) $(DBLIBS) -lperf -lgclgrid -lseispp
# Following is required to work with g++ 3.2.3
#ldflags=-shared -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid


include $(ANTELOPEMAKE) 
include $(ANTELOPEMAKE).local

CXXFLAGS += -g $(MATLABINCLUDE) -DMATLABPLOTTING

OBJS=RFeventstacker.o MatlabPlotter.o
MatlabPlotter.o : MatlabPlotter.h
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
