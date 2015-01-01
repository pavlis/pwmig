INCLUDE=Hypocenter.h TauPCalculator.h TravelTimeCalculator.h 
LIB = libttpp.a
#DLIB=$(LIB:.a=$(DSUFFIX))

#REQUIRE_DATA = taup_ttimes
#DATADIR = tables

ldlibs=-lstock -lcoords -lseispp
cxxflags=-DNO_ANTELOPE -g

include $(PWMIGMAKE)
DIRS=

OBJS=TauPCalculator.o \
  Hypocenter.o \
  delta.o \
  f2ctaup.o 
$(LIB) : $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT)
	$(RANLIB) $@

        
#$(DLIB) : $(OBJS)
#	$(DLD) $(DLDFLAGS) -L$(PWMIG)/lib -lcoords -lstock -lposix4 -lm -o $@ $(LORDER) $(OBJS) $(TSORT)

