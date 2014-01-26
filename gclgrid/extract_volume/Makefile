BIN=extract_volume
PF=extract_volume.pf

#MAN1=extract_volume.1
#cflags=-O
ldlibs=-lseispp -ltrvltm -lgclgrid -lseispp $(TRLIBS) $(DBLIBS) -lperf -lwffil -lbrttutil -lm

include $(ANTELOPEMAKE) 
OBJS= extract_volume.o pfutils.o SEGY2002FileHandle.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
