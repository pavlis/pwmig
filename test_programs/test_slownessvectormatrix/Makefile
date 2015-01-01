BIN=test_svm
cxxflags=-g -DNO_ANTELOPE
ldlibs=-lseispp -lpwmig
include $(PWMIGMAKE)
OBJS=test_svm.o
$(BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
