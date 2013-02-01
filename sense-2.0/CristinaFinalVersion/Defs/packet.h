#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)

//bridge
#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1

//type of packet
#define DATA 100
#define BPDU 200

//port role(STP)
#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300

//port state(STP)
#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3

//////////////
//GENERAL/////
//////////////

int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	//General
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		//100->data;200->bpdu(STP)
	int bit_length;		//packet length in bits
	int num_hops;		//packet length in bits
	int current_bridge;	//bridge where the source is currently located

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

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	//array of bridges containing array of ports - port database. it contains info about the believed designated bridge, and the state and role information

struct MAC_ENTRY{
	//BRIDGE
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		//if the flow is active (1) or not (0) 
	float TimeOfActivation;	//Time the flow was activated
	int SourceId;		//Station Id of this flow
	float AT;		//AT for the Scheduler
	float AR;		//AR for the Scheduler
};

struct CAC_INFO{
	
	int CACId;			//Identification Number of this CAC
	int ActiveFlows;		//Active Flows in this cell
	int numAdjacentCells;		//Number of adjacent cells of this cac(cell)
	int AdjacentCells[MAX_BRIDGES];	//Adjacent cells of this cac -> Useful in order to compute number of expected handoff calls
	double BW;			//BW of this cell
	int N; 				//Max number of flows that could be accepted per cell
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];

//////////////
//STATISTICS//
//////////////

//general results creation-reception pf packets
int generated_packets[MAX_STATIONS][MAX_STATIONS];		//number of generated packets from station to station (station - source)
int received_packets[MAX_STATIONS][MAX_STATIONS];		//number of received packets from station to station (station - sink)

//link information at packet level
int received_data_packets[MAX_BRIDGES][MAX_PORTS];		//number of received data packets in each port of each bridge (bridge - bridging)
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		//number of received bpdu packets in each port of each bridge (bridge - bridging)
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		//number of forwarded data packets in each port of each bridge (bridge - bridging)
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		//number of forwrded bpdu packets in each port of each bridge (bridge - bridging)
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;

//lost packets due to backup-blocked port
int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	//lost data packets due to backup state (bridge- bridging)
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	//lost bpdu packets due to backup state (bridge- bridging)

//lost packets in scheduler queues
int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	//lost data packets due to full queue (bridge - scheduler - queue)
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	//lost bpdu packets due to full queue (bridge - scheduler - queue)
//there is also an scheduler in tehe ouptut ports of the stations, but there shouls be no losses there... it's not wise...

//time while a link is serving, to compute utilization and overheads
double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; //time while the link/server is used/serving a data packet
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; //time while the link/server is used/serving a bpdu packet

//queue performance
double queue_waiting_time;								//decide what kind of array to use, how many queues per port (cesar)
double queue_number_enqueued;							//decide what kind of array to use, how many queues per port (cesar)

//CAC Statistics
int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;
//...

#endif 

