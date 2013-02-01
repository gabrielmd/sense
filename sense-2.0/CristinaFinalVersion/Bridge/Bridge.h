#include "../Scheduler/Scheduler.h"
#include "../Bridging/Bridging.h"
#include "../Driver/Driver.h"
#include "../CAC/CAC.h"
#include "../STP/stp.h"

#include <vector>

#include <string>
#include <iostream>
using namespace std;

component Bridge : public TypeII{
	public: 
		int bridge_id;		//current bridge id

		Bridging bridging;	
		Driver driver;
		CAC cac;
		Scheduler[] scheduler;
		STP stp;

		inport inline void in0(PACKET& p);
		inport inline void in1(PACKET& p);
		inport inline void in2(PACKET& p);
		inport inline void in3(PACKET& p);
		inport inline void in4(PACKET& p);
		inport inline void in5(PACKET& p);
		inport inline void in6(PACKET& p);

		outport void out0(PACKET& p);
		outport void out1(PACKET& p);
		outport void out2(PACKET& p);
		outport void out3(PACKET& p);
		outport void out4(PACKET& p);
		outport void out5(PACKET& p);
		outport void out6(PACKET& p);

		inport inline void to_out0(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out1(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out2(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out3(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out4(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out5(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out6(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port

		inport inline void cac_in(PACKET& p);		//to the CAC, from the flow generator	
		outport void cac_out(PACKET& p);		//from the CAC, to the flowgenerator
	
		Bridge(){};
		virtual ~Bridge(){}; 
		void Setup(const char *, int);

		int print_bridge_debugging_points;
};

void Bridge :: Setup(const char *name, int bid){

	bridge_id=bid;

	bridging.Setup("bridging",bid);

	stp.Setup("STP",bid);

	scheduler.SetSize(MAX_PORTS);
	for(int i=0;i<MAX_PORTS;i++){
		scheduler[i].Setup("scheduler",bid,i);
	}
	cac.Setup("cac",bid);

	connect bridging.out[0],scheduler[0].in;
	connect bridging.out[1],scheduler[1].in;
	connect bridging.out[2],scheduler[2].in;
	connect bridging.out[3],scheduler[3].in;
	connect bridging.out[4],scheduler[4].in;
	connect bridging.out[5],scheduler[5].in;
	connect bridging.out[6],scheduler[6].in;

	connect scheduler[0].out,driver.in0;
	connect scheduler[1].out,driver.in1;
	connect scheduler[2].out,driver.in2;
	connect scheduler[3].out,driver.in3;
	connect scheduler[4].out,driver.in4;
	connect scheduler[5].out,driver.in5;
	connect scheduler[6].out,driver.in6;

	connect driver.out0,to_out0;
	connect driver.out1,to_out1;
	connect driver.out2,to_out2;
	connect driver.out3,to_out3;
	connect driver.out4,to_out4;
	connect driver.out5,to_out5;
	connect driver.out6,to_out6;

	connect bridging.out_bpdu,stp.in_bpdu;
	connect stp.out_bpdu, bridging.in_bpdu;

	connect cac.out,cac_out;
	connect cac_in,cac.in;

	print_bridge_debugging_points=0;
};

//////////////////////
//IN PORTS FUNCTIONS//
//////////////////////

void Bridge :: in0(PACKET& packet){
	if(packet.current_bridge==bridge_id){
		if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,0,SimTime());
		if(PRINT_CAC == 1) 	printf("\tBridge %d -> Received packet from source %d to destination %d \n",bridge_id,packet.source,packet.destination);
		bridging.in(packet,0);
	}
}

void Bridge :: in1(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,1,SimTime());
	bridging.in(packet,1);
}

void Bridge :: in2(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,2,SimTime());
	bridging.in(packet,2);
}

void Bridge :: in3(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,3,SimTime());
	bridging.in(packet,3);
}

void Bridge :: in4(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,4,SimTime());
	bridging.in(packet,4);
}

void Bridge :: in5(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,5,SimTime());
	bridging.in(packet,5);
}

void Bridge :: in6(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,6,SimTime());
	bridging.in(packet,6);
}

//////////////////////////
//TO OUT PORTS FUNCTIONS//
//////////////////////////

void Bridge :: to_out0(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	packet.current_bridge=bridge_id; //only for MySink
	out0(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,0,SimTime());
}

void Bridge :: to_out1(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out1(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,1,SimTime());
}

void Bridge :: to_out2(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out2(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,2,SimTime());
}

void Bridge :: to_out3(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out3(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,3,SimTime());
}

void Bridge :: to_out4(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out4(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,4,SimTime());
}

void Bridge :: to_out5(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out5(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,5,SimTime());
}

void Bridge :: to_out6(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out6(packet);
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,6,SimTime());
}
