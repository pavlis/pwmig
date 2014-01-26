BIN=pwstack
PF=pwstack.pf
DATADIR=schemas/css3.0.ext
DATA=gclgdisk gclfield

#MAN1=pwstack.1
cflags=-g
# Use the following for Solaris
ldlibs=-lpwmig -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid -lseispp
# Following is required to work with g++ 3.2.3
#ldflags=-shared -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid


include $(ANTELOPEMAKE) 

OBJS=pwstack.o \
	apsubs.o \
	aperture.o \
	pwstack_process.o \
	compute_stack_coherence.o
$(BIN) : $(OBJS)
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
