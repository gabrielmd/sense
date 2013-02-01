#include "../Scheduler/Scheduler.h"
#include "../Bridging/Bridging.h"
#include "../Driver/Driver.h"
#include "../CAC/CAC.h"

component Bridge : public TypeII{
	public: 
		inport inline void in(PACKET& p);		//array of input ports
		outport[] void out(PACKET& p);			//array of output ports
		inport inline void to_out0(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		inport inline void to_out1(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
		//inport inline void to_out2(PACKET& p);	//'virtual' incoming port that 'connects' to the real out port
		//inport inline void to_out3(PACKET& p);	//'virtual' incoming port that 'connects' to the real out port
		inport inline void cac_in(PACKET& p);		//to the CAC, from the flow generator	
		outport void cac_out(PACKET& p);		//from the CAC, to the flowgenerator
		inport inline void to_cac_out(PACKET& p);	//virtual port

		Bridging bridging;	
		Scheduler[] scheduler;
		Driver driver;
		CAC cac;

		Bridge();
		virtual ~Bridge(){}; 
		void Setup(const char *, int);

	private:
		int BridgeNumber;
};

Bridge::Bridge(){
	out.SetSize(2);
	scheduler.SetSize(2);
	
	connect in,bridging.in;
	connect bridging.out[0],scheduler[0].in;
	connect bridging.out[1],scheduler[1].in;
	connect scheduler[0].out,driver.in0;
	connect scheduler[1].out,driver.in1;
	connect driver.out0,to_out0;
	connect driver.out1,to_out1;

	connect cac.out,to_cac_out;
	connect cac_in,cac.in;
}

void Bridge :: to_out0(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out[0](packet);
}

void Bridge :: to_out1(PACKET& packet){			//and this to_out forwards the packet to the real out port
	//printf("bridge %d sends at %f\n",BridgeNumber,SimTime());
	out[1](packet);
}

void Bridge :: to_cac_out(PACKET& packet){
	cac_out(packet);
}

void Bridge :: in(PACKET& packet){
	//printf("bridge %d receives at %f\n",BridgeNumber,SimTime());
	scheduler[0].in(packet);
	scheduler[1].in(packet);
}

void Bridge :: cac_in(PACKET& packet){
	cac.in(packet);
}

void Bridge :: Setup(const char *name, int id){
	BridgeNumber = id;

	scheduler[0].Setup("scheduler",id);
	scheduler[1].Setup("scheduler",id);

	cac.Setup("cac",id);
};