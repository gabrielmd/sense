CC=g++
CCFLAGS=-Wall -O6
MAKE=
COMPCXX=../../bin/cxx
OBJS = mm1  mm1-comp mm1-server mmm1

all:  $(OBJS) 

mm1.cxx: mm1.cc 
	$(COMPCXX) mm1.cc
mm1: mm1.cxx
	$(CC) $(CCFLAGS) -o mm1 mm1.cxx

mm1-comp.cxx: mm1-comp.cc 
	$(COMPCXX) mm1-comp.cc
mm1-comp: mm1-comp.cxx
	$(CC) $(CCFLAGS) -o mm1-comp mm1-comp.cxx

mm1-server.cxx: mm1-server.cc 
	$(COMPCXX) mm1-server.cc
mm1-server: mm1-server.cxx
	$(CC) $(CCFLAGS) -o mm1-server mm1-server.cxx

mmm1.cxx: mmm1.cc 
	$(COMPCXX) mmm1.cc
mmm1: mmm1.cxx
	$(CC) $(CCFLAGS) -o mmm1 mmm1.cxx

clean:
	rm $(OBJS) *.exe *.cxx compcxx_*.h


