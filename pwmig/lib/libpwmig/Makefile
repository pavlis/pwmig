LIB=libpwmig.a
INCLUDE=PwmigFileHandle.h PPWDeconProcessor.h
#MAN3=		# man pages for library routines
include $(ANTELOPEMAKE)
#CXXFLAGS+=-D_FILE_OFFSET_BITS=64
#CXXFLAGS+=-D_LARGEFILE_SOURCE
#CXXFLAGS+=-D_LARGEFILE64_SOURCE
#  needed for debug
CXXFLAGS+=-I/sw/include
# for valgrid use only
CXXFLAGS+=-O0

OBJS=PwmigFileHandle.o PPWDeconProcessor.o
PPWDeconProcessor.o :PPWDeconProcessor.h
$(LIB) : $(OBJS)
	$(RM) $@
	$(CXXAR) $(CXXARFLAGS) $@ $(OBJS)  
	$(RANLIB) $@
