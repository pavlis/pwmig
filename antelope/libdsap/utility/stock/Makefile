LIB=libstock.a
INCLUDE= arrays.h\
 arrays.h \
 deviants.h \
 deviants_include.h \
 dsap_regex.h \
 elog.h \
 rb.h \
 stock.h \
 stocktime.h
OBJS=arr.o \
  elog.o \
  hook.o \
  ssearch.o \
  rb_delete.o \
  rb_free.o \
  rb_init.o \
  rb_insert.o \
  rb_level_traverse.o \
  rb_lshow.o \
  rb_search.o \
  rb_show.o \
  rb_traverse.o \
  rb_vcg.o \
  strings.o \
  tbl.o \
  wd.o
include $(PWMIGMAKE)
$(LIB) : $(OBJS)
	ar rv $@ $(OBJS)
