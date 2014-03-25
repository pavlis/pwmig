INCLUDE = \
	arrays.h \
	elog.h \
	pf.h \
	rb.h \
	dsap_regex.h \
	stock.h \
	tmpl.h 


DATA = wd.cf
LIB = libstock.a 
DLIB=$(LIB:.a=$(DSUFFIX))

PF=pfcheck.pf
OPTIMIZE= \
	arr.c \
	pf_.c \
	pffree.c \
	pfget.c \
	pflex.c \
	pfput.c \
	pfresolve.c \
	pfwrite.c \
	quicksort.c \
	rb_delete.c \
	rb_free.c \
	rb_init.c \
	rb_insert.c \
	rb_level_traverse.c \
	rb_lshow.c \
	rb_search.c \
	rb_show.c \
	rb_traverse.c \
	rb_vcg.c \
	stbl.c \
	tbl.c 

MAN3= 	abspath.3 \
	align.3 \
	arr.3 \
	ask.3 \
	banner.3 \
	bits.3 \
	compress.3 \
	datafile.3 \
	datafile.3f \
	elog.3 \
	error_handling.3f \
	expand_env.3 \
	fdkey.3 \
	fill_random.3 \
	fixup_env.3 \
	fs2cs.3 \
	gethdir.3 \
	hexdump.3 \
	dsap_glob.3 \
	htond.3 \
	hook.3 \
	isort.3 \
	isort.3f \
	makedir.3 \
	patsub.3 \
	pf.3 \
	pf_.3f \
	quicksort.3 \
	dsap_regex.3 \
	run.3 \
	shellsort.3 \
	shellsort.3f \
	signal_info.3 \
	ssearch.3 \
	ssearch.3f \
	stbl.3 \
	strings.3 \
	tbl.3 \
	tbl.3f \
	wd.3 \
	xlat.3 \
	zopen.3 

include $(DSAPMAKE)
DIRS=

OBJS= \
	align.o \
	arr.o \
	ask.o \
	banner.o \
	bits.o \
	datafile.o \
	datafile_.o \
	elog.o \
	error_.o \
	expand_env.o \
	fdkey.o \
	fill_random.o \
	fixup_env.o \
	fs2cs.o \
	dsap_glob.o \
	gz_open.o \
	hexdump.o \
	hook.o \
	htond.o \
	ignoreSIGPIPE.o \
	isort.o \
	isort_.o \
	newdata.o \
	patsub.o \
	pffree.o \
	pfget.o \
	pflex.o \
	pfput.o \
	pfresolve.o \
	pfwrite.o \
	pf_.o \
	quicksort.o \
	rb_delete.o \
        rb_free.o \
        rb_init.o \
        rb_insert.o \
        rb_level_traverse.o \
        rb_lshow.o \
        rb_search.o \
        rb_show.o \
        rb_traverse.o \
	dsap_regex.o \
	run.o \
	shellsort.o \
	shellsort_.o \
	ssearch.o \
	ssearch_.o \
	stbl.o \
	strings.o \
	tbl.o \
	tbl_.o \
	tmpl.o \
	wd.o \
	xlat.o \
	zopen.o 

$(LIB) : $(INCLUDE) $(OBJS)
	-rm -f $(LIB)
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT)
	$(RANLIB) $@
                
$(DLIB) : $(OBJS)
	$(DLD) $(DLDFLAGS) -o $@ $(LORDER) $(OBJS) $(TSORT)
 
pflex.c : pflex.l
	$(LEX) -t $*.l > $@   
	fix_yacclex PF $@

 
pfresolve.c : pfresolve.l
	$(LEX) -t $*.l > $@   
	fix_yacclex PFR $@

# $Id: Makefile,v 1.12 1998/02/16 21:26:12 danq Exp $ 
