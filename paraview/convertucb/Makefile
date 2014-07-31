BIN=convertucb
cflags=-g 
ldlibs=-lgclgrid -lseispp $(DBLIBS) -lperf

include $(ANTELOPEMAKE) 
OBJS=	convertucb.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
