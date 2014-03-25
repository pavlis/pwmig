INCLUDE=tttaup.h tttaup.i
LIB = libtttaup.a
DLIB=$(LIB:.a=$(DSUFFIX))

MAN3= ptime.3 ptime.3f slowness.3 tttaup.3
REQUIRE_DATA = taup_ttimes
DATADIR = tables

ldlibs=-lstock -lcoords -lposix4 -lm 

include $(DSAPMAKE)
DIRS=

OBJS=f2ctaup.o tt_taup.o ptime.o ptime_.o slowness.o tttaup.o
$(LIB) : $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT)
	$(RANLIB) $@

        
$(DLIB) : $(OBJS)
	$(DLD) $(DLDFLAGS) -L$(DSAP)/lib -lcoords -lstock -lposix4 -lm -o $@ $(LORDER) $(OBJS) $(TSORT)


# $Id: Makefile,v 1.2 1998/05/12 16:45:59 danq Exp $ 
