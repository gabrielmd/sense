#include "../Defs/packet.h"

component CAC : public TypeII
{
	public: 
		inport inline void in(PACKET& request);
		outport void out(PACKET& response);

		CAC(){}; 
		virtual ~CAC(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int CACNumber;	

};

void CAC :: Setup(const char *name, int id)
{
	CACNumber = id;

	BlockedNewFlows[CACNumber] = 0;
	BlockedHandoffFlows[CACNumber] = 0;
	AcceptedNewFlows[CACNumber] = 0;
	AcceptedHandoffFlows[CACNumber] = 0;

	TotalFlowRequests[CACNumber] = 0;
	TotalHandoffRequests[CACNumber] = 0;

};

void CAC :: Start(){

	cac_info[CACNumber].CACId = CACNumber;	
	cac_info[CACNumber].ActiveFlows = 0;
	//cac_info[CACNumber].numAdjacentCells = 0;		//For the moment
	cac_info[CACNumber].BW = CellBW;
	cac_info[CACNumber].N = (int)(CellBW / FlowRate);	//2e6 / 64e3 = 31 flows could be accepted

	//Initialize ActiveFlow_INFO Structure
	for(int i = 0; i < MAX_STATIONS; i++){
		cac_info[CACNumber].active_flows_info[i].FlagActive = 0;
		cac_info[CACNumber].active_flows_info[i].TimeOfActivation = -1;
		cac_info[CACNumber].active_flows_info[i].SourceId = -1;
		cac_info[CACNumber].active_flows_info[i].AT = -1;
		cac_info[CACNumber].active_flows_info[i].AR = -1;
	}
};

void CAC :: Stop(){

float BlockingNewProbability;
float BlockingHandoffProbability;
int AttachedStations = 0;

//Print Statistics

if(PRINT_CAC==1){

	printf("\nSTATISTICS CAC id %d\n", CACNumber);

	printf("\n\tRequests Received \t %d\n", TotalFlowRequests[CACNumber]);
	printf("\n\tHandoffs Received \t %d\n", TotalHandoffRequests[CACNumber]);

	printf("\n\tAccepted New Flows \t %d\n", AcceptedNewFlows[CACNumber]);
	printf("\n\tAccepted Handoff Flows \t %d\n", AcceptedHandoffFlows[CACNumber]);

	printf("\n\tBlocked New Flows \t %d\n", BlockedNewFlows[CACNumber]);
	printf("\n\tBlocked Handoff Flows \t %d\n", BlockedHandoffFlows[CACNumber]);

	if(BlockedNewFlows[CACNumber] == 0)
		BlockingNewProbability = 0;
	else
		BlockingNewProbability = (float)BlockedNewFlows[CACNumber]/(float)TotalFlowRequests[CACNumber];

	if(BlockedHandoffFlows[CACNumber] == 0)
		BlockingHandoffProbability = 0;
	else
		BlockingHandoffProbability = (float)BlockedHandoffFlows[CACNumber]/(float)TotalHandoffRequests[CACNumber];

	printf("\n\tNew Flows Block. Prob.\t %f\n", BlockingNewProbability);	
	printf("\n\tHandoff Block. Prob.\t %f\n", BlockingHandoffProbability);	

}
	for (int i=0; i<n_stations; i++){

		if(station_info[i].AttachedBridge == CACNumber)
			AttachedStations++;
	}
	printf("\t %d", AttachedStations);		

};

void CAC :: in(PACKET &request)
{

PACKET response;

	if (request.destination == CACNumber){

		if(request.type == 300){			//Flow Request
			response.source = CACNumber;
			response.destination = request.source;
			response.current_bridge = CACNumber;
		
			if (cac_info[CACNumber].ActiveFlows < cac_info[CACNumber].N){

				response.status = ACCEPT;
				cac_info[CACNumber].ActiveFlows++;

				//Update ActiveFlows_INFO structure

				cac_info[CACNumber].active_flows_info[request.source].FlagActive = 1;
				cac_info[CACNumber].active_flows_info[request.source].TimeOfActivation = SimTime();
				cac_info[CACNumber].active_flows_info[request.source].SourceId = request.source;
				cac_info[CACNumber].active_flows_info[request.source].AT = FlowAT;
				cac_info[CACNumber].active_flows_info[request.source].AR = FlowAR;
			}
			else
				response.status = REJECT;

			out(response);
		}
		if (request.type == 400){			//Flow Delete
			cac_info[CACNumber].ActiveFlows--;

				//Update ActiveFlows_INFO structure

				cac_info[CACNumber].active_flows_info[request.source].FlagActive = 0;
				cac_info[CACNumber].active_flows_info[request.source].TimeOfActivation = -1;
				cac_info[CACNumber].active_flows_info[request.source].SourceId = -1;
				cac_info[CACNumber].active_flows_info[request.source].AT = -1;
				cac_info[CACNumber].active_flows_info[request.source].AR = -1;

		}
	}
};

