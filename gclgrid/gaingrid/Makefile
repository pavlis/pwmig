BIN=gaingrid
cflags=-g
ldlibs=-lseispp -ltrvltm -lgclgrid $(TRLIBS) $(DBLIBS) -lperf -lm -lseispp

include $(ANTELOPEMAKE) 
OBJS=gaingrid.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
