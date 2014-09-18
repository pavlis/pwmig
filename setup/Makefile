all : buildenv setup
setup : setup.csh
	cp setup.csh $(PWMIG)
buildenv : pwmigmake subdirs
	if [ ! -d $(PWMIG)/bin ] ; then \
		mkdir $(PWMIG)/bin ; \
	fi ;
	cp subdirs $(PWMIG)/bin
	if [ ! -d $(PWMIG)/include ] ; then \
		mkdir $(PWMIG)/include ; \
	fi ;
	cp pwmigmake $(PWMIG)/include
	if [ ! -d $(PWMIG)/static ] ; then \
		mkdir $(PWMIG)/static ; \
	fi ;
	if [ ! -d $(PWMIG)/lib ] ; then \
		mkdir $(PWMIG)/lib ; \
	fi ;
	if [ ! -d $(PWMIG)/man ] ; then \
		mkdir $(PWMIG)/man ; \
		mkdir $(PWMIG)/man/man3 ; \
	fi ;
	if [ ! -d $(PWMIG)/antelope_src ] ; then \
		mkdir $(PWMIG)/antelope_src ; \
		mkdir $(PWMIG)/antelope_src/libgclgrid ; \
		mkdir $(PWMIG)/antelope_src/libseispp ; \
		mkdir $(PWMIG)/antelope_src/libperf ; \
                cp directory_makefile $(PWMIG)/antelope_src/Makefile ;\
	fi ;
