#include "../Defs/packet.h"

component FlowGen : public TypeII{
	public: 
		outport void out(PACKET& request);		
		outport void control_out(PACKET& p);
		inport inline void in(PACKET& response);
		inport inline void control_in(int& OldCell, int& newCell);

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
	interFlowRequest.Set(5);//interFlowRequestTime);
	
};

void FlowGen :: Stop(){
};

void FlowGen :: timer_interFlowRequest(trigger_t &){

	PACKET request;

	TotalFlowRequests[station_info[SourceNumber].AttachedBridge]++;
	
	request.source = SourceNumber;
	request.destination = station_info[SourceNumber].AttachedBridge;
	request.type = 300;		//Flow Request
	if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowRequest packet from %d to %d at %f",SourceNumber, request.source, request.destination, SimTime());
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
	notifyend.destination = station_info[SourceNumber].AttachedBridge;
	notifyend.type = 400;
	if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowDelete packet from %d to %d at %f",SourceNumber, notifyend.source, notifyend.destination, SimTime());
	out(notifyend);

};

void FlowGen :: in(PACKET &response)
{

	//Received response

	PACKET startpacketGen;

	if (response.destination == SourceNumber && response.current_bridge == station_info[SourceNumber].AttachedBridge){
			
		if(response.status == ACCEPT){

			startpacketGen.begin = START;
			if(!FlowDuration.Active()){		//This means that is not a handoff request
				FlowDuration.Set(FlowDurationTime + SimTime());
				AcceptedNewFlows[station_info[SourceNumber].AttachedBridge]++;

			}
			else
				AcceptedHandoffFlows[station_info[SourceNumber].AttachedBridge]++;
			if(PRINT_CAC==1)	printf("\nFlowGen %d -> Received FlowResponse packet from %d at %f with status ACCEPT\n", response.destination, response.source, SimTime());

			control_out(startpacketGen);


		}
		else{		//If the flow could not be accepted start the inter flow timer once again

			if(FlowDuration.Active()){		//This means that is a handoff request
					FlowDuration.Cancel();	
					BlockedHandoffFlows[station_info[SourceNumber].AttachedBridge]++;
			}
			else
				BlockedNewFlows[station_info[SourceNumber].AttachedBridge]++;

			interFlowRequest.Set(interFlowRequestTime + SimTime());
			if(PRINT_CAC==1)	printf("\nFlowGen %d -> Received FlowResponse packet from %d at %f with status REJECT\n", response.destination,response.source, SimTime());
			
		}
	}
};
void FlowGen :: control_in(int &OldCell, int &NewCell){

//Change cell -> Send delete to current CAC and stop packet generator, send flowrequest_handoff to new CAC


	PACKET stoppacketGen;
	PACKET notifyend;
	PACKET request;

	if(FlowDuration.Active()){			//Only if the flow is active

		//Send stop to the packet generator

		stoppacketGen.begin = STOP;
		control_out(stoppacketGen);	
		
		//Send delete flow to the CAC

		notifyend.source = SourceNumber;
		notifyend.destination = OldCell;
		notifyend.type = 400;
		if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowDelete packet from %d to %d at %f",SourceNumber, notifyend.source, notifyend.destination, SimTime());
		out(notifyend);

		if(station_info[SourceNumber].AttachedBridge != -1){

			//Send request to the new CAC

			TotalHandoffRequests[station_info[SourceNumber].AttachedBridge]++;

			request.source = SourceNumber;
			request.destination = station_info[SourceNumber].AttachedBridge;
			request.type = 300;		//Flow Request
			if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowRequest Handoff packet from %d to %d at %f",SourceNumber, request.source, request.destination, SimTime());
			out(request);
		}
		else
			FlowDuration.Cancel();
		
	}
	if(station_info[SourceNumber].AttachedBridge == -1 && interFlowRequest.Active()){
		interFlowRequest.Cancel();
	}
};
