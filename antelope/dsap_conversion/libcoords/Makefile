LIB=libcoords.a
INCLUDE=coords.h
OBJS=dist.o \
     dcarsph.o \
     dsphcar.o \
     dr3add.o \
     dr3cros.o \
     dr3dot.o \
     dr3mag.o \
     dr3mxv.o \
     dr3norm.o \
     dr3sub.o \
     dr3sxv.o \
     rotate.o
include $(PWMIGMAKE)
$(LIB) : $(OBJS)
	ar rv $@ $(OBJS)
