#include "../Defs/packet.h"

component FlowGen : public TypeII{
	public: 
		outport void out(PACKET& request);		
		outport void control_out(PACKET& p);
		inport inline void in(PACKET& response);

		Timer <trigger_t> interFlowRequest;
		Timer <trigger_t> FlowDuration;
		inport inline void timer_interFlowRequest(trigger_t& t);
		inport inline void timer_FlowDuration(trigger_t& t);

		FlowGen(); 
		virtual ~FlowGen(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int SourceNumber;
		int interFlowRequestTime;
		int FlowDurationTime;
};

FlowGen::FlowGen(){

	connect interFlowRequest.to_component,timer_interFlowRequest;
	connect FlowDuration.to_component,timer_FlowDuration;
}

void FlowGen :: Setup(const char *name, int id){

	SourceNumber = id;
};

void FlowGen :: Start(){
	interFlowRequestTime = 10;	
	FlowDurationTime = 5;
	interFlowRequest.Set(0);//interFlowRequestTime);
	
};

void FlowGen :: Stop(){
};

void FlowGen :: timer_interFlowRequest(trigger_t &){

	PACKET request;
	request.source = SourceNumber;
	request.destination = 0;
	request.type = 300;		//Flow Request
	//printf("\n\nFlowGen -> Send FlowRequest packet from %d at %f",request.source, SimTime());
	out(request);

};

void FlowGen :: timer_FlowDuration(trigger_t &){

	PACKET stoppacketGen;
	PACKET notifyend;
	
	//Send stop to the packet generator

	stoppacketGen.begin = STOP;
	interFlowRequest.Set(interFlowRequestTime + SimTime());
	control_out(stoppacketGen);	
	
	//Send delete flow to the CAC

	notifyend.source = SourceNumber;
	notifyend.destination = 0;
	notifyend.type = 400;
	out(notifyend);
};

void FlowGen :: in(PACKET &response)
{
	//Received response

	PACKET startpacketGen;

	if (response.destination == SourceNumber)
		//printf("\nFlowGen -> Received FlowResponse packet from %d at %f",response.source, SimTime());

	if(response.status == ACCEPT){

		startpacketGen.begin = START;
		FlowDuration.Set(FlowDurationTime + SimTime());
		control_out(startpacketGen);

	}
	else 		//If the flow could not be accepted start the inter flow timer once again

		interFlowRequest.Set(interFlowRequestTime + SimTime());
};

