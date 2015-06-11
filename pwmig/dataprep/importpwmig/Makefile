BIN=importpwmig
MAN1=importpwmig.1
ldlibs=-lseispp -lgclgrid $(DBLIBS) -lperf
SUBDIR=/contrib

include $(ANTELOPEMAKE)
OBJS=importpwmig.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
