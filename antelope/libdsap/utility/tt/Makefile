
LIB=libtt.a
INCLUDE=tt.h
MAN3=tt.3

include $(DSAPMAKE)
DIRS=

OBJS=tt.o
$(LIB) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT)
	$(RANLIB) $@

$(DLIB) : $(OBJS)
	$(DLD) $(DLDFLAGS) -o $(DLIB) $(LORDER) $(OBJS) $(TSORT)

# $Id: Makefile,v 1.1.1.1 1997/04/12 04:19:08 danq Exp $ 
