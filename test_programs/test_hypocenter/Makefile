BIN=test_hypocenter
cxxflags=-g -DNO_ANTELOPE
ldlibs=-lseispp -lttpp -lgclgrid -lperf -lstock -lcoords -lm
include $(PWMIGMAKE)
OBJS=test_hypocenter.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
