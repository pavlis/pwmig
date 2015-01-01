LIB=libstock.a
INCLUDE=stock.h \
        elog.h \
        swapbytes_pwmig.h
OBJS=datafile.o \
     elog.o \
     epoch.o \
     h2e.o \
     now.o \
     str2epoch.o \
    strings.o \
    swap.o \
    time.o \
    wd.o
cxxflags = -g
include $(PWMIGMAKE)
$(LIB) : $(OBJS)
	ar rv $@ $(OBJS)
