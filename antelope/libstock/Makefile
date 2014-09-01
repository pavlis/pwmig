LIB=libstock.a
INCLUDE=stock.h
INCLUDE= arrays.h\
OBJS=datafile.o
  strings.o \
  wd.o
include $(PWMIGMAKE)
$(LIB) : $(OBJS)
	ar rv $@ $(OBJS)
