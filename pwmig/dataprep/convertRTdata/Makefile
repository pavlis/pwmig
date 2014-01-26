BIN=convertRTdata
INCLUDE=
PF=convertRTdata.pf
#MAN1=		# man pages for commands

#cflags=-g -DSUNPERF
cxxflags=-g 
DBX=-g
ldlibs= -lseispp -lgclgrid -ltrvltm $(DBLIBS) $(TRLIBS) -lperf -lm

include $(ANTELOPEMAKE)
DIRS=

OBJS=convertRTdata.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
