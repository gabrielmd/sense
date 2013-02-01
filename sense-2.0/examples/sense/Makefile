CC=g++
CCFLAGS=-Wall -g
MAKE=
COMPCXX=../../bin/cxx
OBJS = sim_flooding sim_aodv sim_dsr sim_nullmac sim_80211

all:  $(OBJS) 

sim_flooding.cxx: sim_routing.cc 
	$(COMPCXX) -Dnet_component=Flooding -Dnet_struct=Flooding_Struct sim_routing.cc
	mv sim_routing.cxx sim_flooding.cxx
sim_flooding:	sim_flooding.cxx
	$(CC) $(CCFLAGS) -o sim_flooding sim_flooding.cxx

sim_aodv.cxx: sim_routing.cc 
	$(COMPCXX) -Dnet_component=AODVI -Dnet_struct=AODVI_Struct sim_routing.cc
	mv sim_routing.cxx sim_aodv.cxx
sim_aodv: sim_aodv.cxx
	$(CC) $(CCFLAGS) -o sim_aodv sim_aodv.cxx

sim_dsr.cxx: sim_routing.cc 
	$(COMPCXX) -Dnet_component=DSRI -Dnet_struct=DSRI_Struct sim_routing.cc
	mv sim_routing.cxx sim_dsr.cxx
sim_dsr: sim_dsr.cxx
	$(CC) $(CCFLAGS) -o sim_dsr sim_dsr.cxx

sim_80211.cxx: sim_80211.cc 
	$(COMPCXX) sim_80211.cc
sim_80211: sim_80211.cxx
	$(CC) $(CCFLAGS) -o sim_80211 sim_80211.cxx

sim_nullmac.cxx: sim_80211.cc 
	$(COMPCXX) -Dmac_component=NullMAC -Dmac_struct=NullMAC_Struct sim_80211.cc
	mv sim_80211.cxx sim_nullmac.cxx
sim_nullmac: sim_nullmac.cxx
	$(CC) $(CCFLAGS) -o sim_nullmac sim_nullmac.cxx

clean:
	rm -f $(OBJS) *.exe *.exe.stackdump *.cxx compcxx_*.h

test: sim_flooding sim_aodv sim_dsr sim_80211 sim_nullmac
	./sim_flooding
	./sim_aodv
	./sim_dsr
	./sim_80211
	./sim_nullmac

