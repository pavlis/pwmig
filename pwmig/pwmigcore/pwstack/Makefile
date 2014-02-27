BIN=pwstack
PF=pwstack.pf
DATADIR=schemas/css3.0.ext
DATA=gclgdisk gclfield
INCLUDE=pwstack_reader.h

#MAN1=pwstack.1
cflags=-g
# Use the following for Solaris
ldlibs=-lpwmig -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid -lseispp
# Following is required to work with g++ 3.2.3
#ldflags=-shared -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid


include $(ANTELOPEMAKE) 

pwstack_reader.cc : pwstack_reader.h
OBJS=pwstack.o \
	apsubs.o \
	aperture.o \
	pwstack_process.o \
        pwstack_reader.o \
	compute_stack_coherence.o
$(BIN) : $(OBJS)
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
