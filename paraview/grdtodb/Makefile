BIN=grdtodb

#MAN1=pwmig.1
#cflags=-g
ldlibs=-lseispp -ltrvltm -lgclgrid $(TRLIBS) $(DBLIBS) -lperf -lm

SUBDIR=/contrib

include $(ANTELOPEMAKE) 
OBJS= grdtodb.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
