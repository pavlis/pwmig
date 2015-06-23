BIN=RFeventstacker
PF=RFeventstacker.pf
DATADIR=schemas/css3.0.ext
DATA=stackstats

cxxflags=-g
# Use the following for Solaris
ldlibs= -lpwmig -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid -lseispp
# Following is required to work with g++ 3.2.3
#ldflags=-shared -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid
SUBDIR=/contrib


include $(ANTELOPEMAKE) 
OBJS=RFeventstacker.o  resample.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
