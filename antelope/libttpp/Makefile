INCLUDE=Hypocenter.h TauPCalculator.h TravelTimeCalculator.h tttaup.i
LIB = libttpp.a
DLIB=$(LIB:.a=$(DSUFFIX))

#REQUIRE_DATA = taup_ttimes
#DATADIR = tables

ldlibs=-lstock -lcoords -lseispp

include $(PWMIGMAKE)
DIRS=

OBJS=TauPCalculator.o \
  Hypocenter.o \
  delta.o \
  subs.o 
$(LIB) : $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT)
	$(RANLIB) $@

        
#$(DLIB) : $(OBJS)
#	$(DLD) $(DLDFLAGS) -L$(PWMIG)/lib -lcoords -lstock -lposix4 -lm -o $@ $(LORDER) $(OBJS) $(TSORT)

