BIN=horizonstat
#MAN1=		# man pages for commands
ldflags=-lseispp -lgclgrid $(TRLIBS) $(DBLIBS) -lperf -lseispp -lm

include $(ANTELOPEMAKE) 
OBJS=horizonstat.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
