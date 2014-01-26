BIN=extract_section
PF=extract_section.pf

MAN1=extract_section.1
#cflags=-O
ldlibs=-lseispp -ltrvltm -lgclgrid -lseispp $(TRLIBS) $(DBLIBS) -lperf -lm

include $(ANTELOPEMAKE) 
OBJS= extract_section.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
