BIN=pwmig
DIRS=
PF=pwmig.pf

#MAN1=pwmig.1
ldlibs=-lpwmig -lseispp -ltrvltm -lgclgrid $(TRLIBS) $(DBLIBS) -lperf -lm

include $(ANTELOPEMAKE) 
OBJS= mdutils.o \
pwmig.o \
ray_transformation_operator.o \
raygrid.o
# include dependencies
mdutils.cc : pwmig.h
pwmig.cc : pwmig.h
ray_transformation_operator.cc : pwmig.h
raygrid.cc : pwmig.h
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
