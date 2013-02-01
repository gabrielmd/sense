CC=g++
CCFLAGS=-Wall -g
OBJS= hello-incl hello-system hello-funct print-int print-array
COMPCXX=../../bin/cxx

all:	$(OBJS)

hello-incl: hello-incl.cxx 
	$(CC) $(CCFLAGS) -o hello-incl hello-incl.cxx

hello-incl.cxx: hello-incl.h hello-incl.cc 
	$(COMPCXX) hello-incl.cc

hello-system: hello-system.cxx 
	$(CC) $(CCFLAGS) -o hello-system hello-system.cxx

hello-system.cxx: hello-system.cc 
	$(COMPCXX) hello-system.cc

hello-funct: hello-funct.cxx 
	$(CC) $(CCFLAGS) -o hello-funct hello-funct.cxx

hello-funct.cxx: hello-funct.cc 
	$(COMPCXX) hello-funct.cc

print-int.cxx: print-int.cc
	$(COMPCXX) print-int.cc

print-int: print-int.cxx
	$(CC) $(CCFLAGS) -o print-int print-int.cxx

print-array.cxx: print-array.cc 
	$(COMPCXX) print-array.cc

print-array: print-array.cxx
	$(CC) $(CCFLAGS) -o print-array print-array.cxx

clean:
	rm $(OBJS) *.exe *.cxx compcxx_*.h


