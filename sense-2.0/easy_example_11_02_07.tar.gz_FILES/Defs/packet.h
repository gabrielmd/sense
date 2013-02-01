#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	//General
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		//100->data;200->bpdu(STP); 300->(FlowRequest);400->(FlowDelete) 
	int bit_length;		//packet length in bits

	//For FlowResponse
	int status; 		//Accept or reject flow

	//For Flowgenerator/Source communication
	int begin;		//Start or stop packet generator

	//STP
	int bridge_id;		//transmitting bridge id
	int root_id;		//assumed root id
	int root_path_cost;	//lower cost to rhe root
	int port_id;		//transmiting port id
	double age;		//since the root created information?
	double MAX_AGE;		//so as to disseminate
};


struct PORT_INFO{
	//STP
	int des_port_id;	//designated port id
	int des_bridge_id;	//designated bridge id
	int des_root_id;	//assumed root id by designated bridge on that segment
	int des_root_path_cost;	//cost of designated bridge
	int state;		//current state of the port: 100->forwarding, 200->pre-forwarding, 300->backup, 400->prebackup
	int role;		//current role of the port: 100->ROOT, 200->designated, 300->non_designated
};

struct CAC_INFO{
	
	int CACId;		//Identification Number of this CAC
	int ActiveFlows;	//Active Flows in this cell
	int AdjacentCells;	//Adjacent cells of this cac -> Useful in order to compute number of expected handoff calls
	double BW;			//BW of this cell
	int N; 			//Max number of flows that could be accepted per cell

};

CAC_INFO cac_info[1];

#endif 

