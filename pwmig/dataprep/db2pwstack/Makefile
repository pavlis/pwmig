BIN=db2pwstack
PF=db2pwstack.pf
MAN1=db2pwstack.1
ldlibs=-lpwmig -lseispp -lgclgrid -ltrvltm -lpfstream -lbrttutil $(TRLIBS) $(DBLIBS) -lperf -lgclgrid -lseispp
include $(ANTELOPEMAKE)  	
OBJS=db2pwstack.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
