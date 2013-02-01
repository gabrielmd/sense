#include "../Defs/packet.h"

#define HELLO_TIME 2
#define PRE_BACKUP_TIMER 5
#define PRE_FORWARDING_TIMER 5

component STP : public TypeII{
	public: 
		int bridge_id;			//current bridge id
		int root_id;			//assumed root id
		int root_path_cost;		//lower cost to rhe root

		inport inline void in_bpdu(PACKET& p, int ip);		//incoming bpdus from bridging module
		outport void out_bpdu(PACKET& p, int op);		//outgoing bpdus to bridging module

		inport inline void send_bpdu_hello_time(trigger_t& t);	//function to be called when timer hello_time expires
		Timer<trigger_t> hello_time;				//timer hello_time, only for root (2 sec)

		inport inline void prebackup_timer_expiration(trigger_t& t, int p);	//function to be called when timer pre_backup expires. p->port
		MultiTimer<trigger_t> prebackup_timer;					//pre_backup timer

		inport inline void preforwarding_timer_expiration(trigger_t& t, int p);	//function to be called when timer pre_forwarding expires. p->port
		MultiTimer<trigger_t> preforwarding_timer;				//preforwarding timer

		STP(){}; 
		virtual ~STP(){}; 
		void Setup(const char *, int );
		void Start();
		void Stop();

		bool updates_info(PACKET& p, int p);	//check if the incoming bpdu updates information
		void send_bpdu(int op);			//sends a bpdu through output port op
		void reconfigure_bridge();		//reconfigures the ports of the bridge

		void print_bridge_info();		//prints the bridge information
		void print_port_info(int pid);		//prints the port information
		int print_STP_debugging_points;
};

/////////////////////////////////////////////////
//////////////////////SETUP//////////////////////
/////////////////////////////////////////////////
void STP :: Setup(const char *name, int bid){
//Setup function called after creating the component. initialize vars

	connect hello_time.to_component,send_bpdu_hello_time;
	connect preforwarding_timer.to_component,preforwarding_timer_expiration;
	connect prebackup_timer.to_component,prebackup_timer_expiration;

	bridge_id=bid;		//bridge id (prio? + mac)
	root_id=bid;		//it assumes the root is itself
	root_path_cost=0;	//since the rrot is itself, the cost to the root si 0

	for(int i=0;i<N_PORTS;i++){			//at the beginning, all ports are designated
		port[bridge_id][i].des_port_id=i;
		port[bridge_id][i].des_bridge_id=bid;
		port[bridge_id][i].des_root_id=bid;
		port[bridge_id][i].des_root_path_cost=0;
		port[bridge_id][i].state=PRE_FORWARDING;
		port[bridge_id][i].role=DESIGNATED_PORT;
		preforwarding_timer.Set(0 + PRE_FORWARDING_TIMER, i);
		//cout << "preforwarding_timer: " << preforwarding_timer.GetTime(i) << "\n";
	}

	hello_time.Set(0.0001);	//set the hello_message timer (right at the beginning)
	print_STP_debugging_points=0;
}

/////////////////////////////////////////////////
//////////////////////START//////////////////////
/////////////////////////////////////////////////
void STP :: Start(){
//Start function called automatically at the beginning of the simulation
}

/////////////////////////////////////////////////
//////////////////////STOP///////////////////////
/////////////////////////////////////////////////
void STP :: Stop(){
//Stop function called automatically at the end of the simulation
}

/////////////////////////////////////////////////
/////////////////////IN_BPDU/////////////////////
/////////////////////////////////////////////////
void STP :: in_bpdu(PACKET &bpdu, int in_port){
//called when a bpdu arrives from the bridging module

	if(bpdu.type!=BPDU)
		if(PRINT_STP==1)	cout << "STP module: a non-bpdu packet has been received!!!\n\n";
	else if(print_STP_debugging_points)
		if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") has received a BPDU in port " << in_port << " (at " << SimTime() << " sec)\n\tBRIDGE_ID: " << bpdu.bridge_id << "\tROOT_ID: " << bpdu.root_id << "\tRPC: " << bpdu.root_path_cost << "\tPORT_ID: " << bpdu.port_id << "\n\n";

	if(updates_info(bpdu, in_port)){	//if the new information is better
		
		//store received information in bridge database
		root_id=bpdu.root_id;
		root_path_cost=bpdu.root_path_cost + LINK_COST;

		//store received information in port database
		port[bridge_id][in_port].des_port_id=bpdu.port_id;
		port[bridge_id][in_port].des_bridge_id=bpdu.bridge_id;
		port[bridge_id][in_port].des_root_id=bpdu.root_id;
		port[bridge_id][in_port].des_root_path_cost=bpdu.root_path_cost;

		reconfigure_bridge();

		//send bpdu to all designated
		for(int n=0;n<N_PORTS;n++){
			if(port[bridge_id][n].role==DESIGNATED_PORT)
				send_bpdu(n);
		}
		
	}
	else{	//if the new information is worse
		if(print_STP_debugging_points) cout << "STP(" << bridge_id << ") has received a non-updating BPDU in port " << in_port << " (at " << SimTime() << " sec)\n\n";
		if(port[bridge_id][in_port].role==DESIGNATED_PORT)
			send_bpdu(in_port); //if designated, send bpdu to that port in order to warn the other side bridge that its info is wrong
	}
	
};

/////////////////////////////////////////////////
///////////////////RECONFIGURE///////////////////
/////////////////////////////////////////////////
void STP :: reconfigure_bridge(){
//It reconfigures the bridge ports: select root and designated ports

	//select new root port
	int root_port_tmp=0;
	int min_rpc=1000000;
	for(int i=0;i<N_PORTS;i++){
		if(port[bridge_id][i].des_root_path_cost<min_rpc && port[bridge_id][i].des_root_id==root_id){ //ADD CODE IN CASE OF TIE!!! now it takes the first one...
			root_port_tmp=i;
			min_rpc=port[bridge_id][i].des_root_path_cost;
		}
	}
	port[bridge_id][root_port_tmp].role=ROOT_PORT;
	root_path_cost=min_rpc + LINK_COST;

	//from others, select designated port(s)
	for(int i=0;i<N_PORTS;i++){
		if(i!=root_port_tmp){

			if(port[bridge_id][i].des_root_id != root_id){		//not brand new information
				port[bridge_id][i].role=DESIGNATED_PORT;
			}
			else{
				if(port[bridge_id][i].des_root_path_cost > root_path_cost){
					port[bridge_id][i].role=DESIGNATED_PORT;
				}
				else{
					if(port[bridge_id][i].des_root_path_cost == root_path_cost && port[bridge_id][i].des_bridge_id > bridge_id){
						port[bridge_id][i].role=DESIGNATED_PORT;
					}
					else{
						if(port[bridge_id][i].des_root_path_cost  == root_path_cost && port[bridge_id][i].des_bridge_id == bridge_id && port[bridge_id][i].des_port_id > i){
						}
						else{
							if(port[bridge_id][i].des_root_path_cost  == root_path_cost && port[bridge_id][i].des_bridge_id == bridge_id && port[bridge_id][i].des_port_id == i) // tot igual, soc jo el designated
								port[bridge_id][i].role=DESIGNATED_PORT;
							else
								port[bridge_id][i].role=NON_DESIGNATED_PORT;
						}
					}
				}
			}
			if(port[bridge_id][i].role==DESIGNATED_PORT){	//if designated, save bridge information
				port[bridge_id][i].des_port_id=i;
				port[bridge_id][i].des_bridge_id=bridge_id;
				port[bridge_id][i].des_root_id=root_id;
				port[bridge_id][i].des_root_path_cost=root_path_cost;
			}
		}
	}
	
	//port state selection
	for(int i=0;i<N_PORTS;i++){
		if(port[bridge_id][i].role==ROOT_PORT || port[bridge_id][i].role==DESIGNATED_PORT){ //to forwarding
			switch (port[bridge_id][i].state){
				case FORWARDING: 	break;
				case PRE_FORWARDING: 	break;
				case BACKUP: 		port[bridge_id][i].state=PRE_FORWARDING; preforwarding_timer.Set(SimTime() + PRE_FORWARDING_TIMER, i); break;
				case PRE_BACKUP: 	port[bridge_id][i].state=FORWARDING; prebackup_timer.Cancel(i); break;
				default:		if(PRINT_STP==1)	cout << "State ERROR in reconfigure_bridge()\n";
			}
		}

		if(port[bridge_id][i].role==NON_DESIGNATED_PORT){ //to backup
			switch (port[bridge_id][i].state){
				case FORWARDING: 	port[bridge_id][i].state=PRE_BACKUP; prebackup_timer.Set(SimTime() + PRE_BACKUP_TIMER, i); break;
				case PRE_FORWARDING: 	port[bridge_id][i].state=BACKUP; preforwarding_timer.Cancel(i); break;
				case BACKUP: 		break;
				case PRE_BACKUP: 	break;
				default:		if(PRINT_STP==1)	cout << "State ERROR in reconfigure_bridge()\n";
			}
		}

	}
	
 	if(print_STP_debugging_points) print_bridge_info();
}

/////////////////////////////////////////////////
///////////////////UPDATES_INFO//////////////////
/////////////////////////////////////////////////
bool STP :: updates_info(PACKET& bpdu, int p){
//compares the info from the received bpdu and that one from the current segment (port database)

	if(bpdu.root_id < port[bridge_id][p].des_root_id){
		return true;
	}
	else{
		if(bpdu.root_id == port[bridge_id][p].des_root_id && bpdu.root_path_cost < port[bridge_id][p].des_root_path_cost){
			return true;
		}
		else{
			if(bpdu.root_id == port[bridge_id][p].des_root_id && (bpdu.root_path_cost) == port[bridge_id][p].des_root_path_cost && bpdu.bridge_id < port[bridge_id][p].des_bridge_id){
				return true;	
			}
			else{
				if(bpdu.root_id == port[bridge_id][p].des_root_id && (bpdu.root_path_cost) == port[bridge_id][p].des_root_path_cost && bpdu.bridge_id == port[bridge_id][p].des_bridge_id && bpdu.port_id < port[bridge_id][p].des_port_id){
					return true;
				}
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////
/////////////////////SEND_BPDU///////////////////
/////////////////////////////////////////////////
void STP :: send_bpdu(int op){
//sends a bpdu through the port op

	PACKET bpdu;					//create the bpdu to send
	bpdu.type=BPDU;
	bpdu.bridge_id=bridge_id;
	bpdu.root_id=root_id;
	bpdu.root_path_cost=root_path_cost;
	bpdu.port_id=op;

	bpdu.source=-1;
	bpdu.destination=-1;
	

	if(print_STP_debugging_points) cout << "STP(" << bridge_id << ") sends a BPDU through port " << op <<" (at " << SimTime() <<" sec)\n\tBRIDGE_ID: " << bpdu.bridge_id << "\tROOT_ID: " << bpdu.root_id << "\tRPC: " << bpdu.root_path_cost << "\tPORT_ID: " << bpdu.port_id << "\n\n";
	out_bpdu(bpdu,op);
}

////////////////////////////////////////////////////////////
/////////////////////SEND_BPDU_HELLO_TIME///////////////////
////////////////////////////////////////////////////////////
void STP :: send_bpdu_hello_time(trigger_t& t){
//called when hello_timer expires (2 sec). sends bpdu to designated ports

	if(bridge_id==root_id){		//if i'm the root
		//send bpdu through DESIGNATED ports
		for(int n=0;n<N_PORTS;n++){
			if(port[bridge_id][n].role==DESIGNATED_PORT)
				send_bpdu(n);
		}
	}
	hello_time.Set(SimTime() + HELLO_TIME);
}

////////////////////////////////////////////////////////////
///////////////PRE_FORWARDING_TIMER_EXPIRATION//////////////
////////////////////////////////////////////////////////////
void STP :: preforwarding_timer_expiration(trigger_t& t, int p){
//called when preforwarding timer expires. p contains the port associated to this timer
	port[bridge_id][p].state=FORWARDING;

	if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_forwarding timer expiration (at " << SimTime() << " sec)\n\n";

	if(print_STP_debugging_points){
		if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_forwarding timer expiration (at " << SimTime() << " sec)\n\n";
		if(PRINT_STP==1)	print_bridge_info();
	}
}

////////////////////////////////////////////////////////////
/////////////////PRE_BACKUP_TIMER_EXPIRATION////////////////
////////////////////////////////////////////////////////////
void STP :: prebackup_timer_expiration(trigger_t& t, int p){
//called when prebackup timer expires. p contains the port associated to this timer
	port[bridge_id][p].state=BACKUP;

	if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_backup timer expiration (at " << SimTime() << " sec)\n\n";

	if(print_STP_debugging_points){
		cout << "STP(" << bridge_id << ") at port " << p << ": pre_backup timer expiration (at " << SimTime() << " sec)\n\n";
		print_bridge_info();
	}
}

/////////////////////////////////////////////////
/////////////////PRINT_BRIDGE_INFO///////////////
/////////////////////////////////////////////////
void STP :: print_bridge_info(){
//prints bridge information (bridge_id, root_id, RPC) and port information

	cout << "***************************************************\n";
	cout << "BRIDGE_ID:\t" << bridge_id << "\nROOT_ID:\t" << root_id << "\nROOT_PATH_COST:\t" << root_path_cost << "\n\n";
	for(int i=0;i<N_PORTS;i++)
		print_port_info(i);
	cout << "***************************************************\n\n";
}

/////////////////////////////////////////////////
//////////////////PRINT_PORT_INFO////////////////
/////////////////////////////////////////////////
void STP :: print_port_info(int pid){
//print port information (des_port, des_root, des_bridge, des_cost, state, role)

	string state_string,role_string;
	switch (port[bridge_id][pid].state){
		case 100: state_string="FORWARDING"; break;
		case 200: state_string="PRE_FORWARDING"; break;
		case 300: state_string="BACKUP"; break;
		case 400: state_string="PRE_BACKUP"; break;
	}

	switch (port[bridge_id][pid].role){
		case 100: role_string="ROOT_PORT"; break;
		case 200: role_string="DESIGNATED_PORT"; break;
		case 300: role_string="NON_DESIGNATED_PORT"; break;
	}

	cout << "\tPORT ID " << pid << "\n\t*********\n\tDES_PORT_ID:\t" << port[bridge_id][pid].des_port_id << "\n\tDES_BRIDGE_ID:\t" << port[bridge_id][pid].des_bridge_id << " \n\tDES_ROOT_ID:\t" << port[bridge_id][pid].des_root_id << "\n\tDES_RPC:\t" << port[bridge_id][pid].des_root_path_cost << "\n\tSTATE:\t\t" << state_string << "\n\tROLE:\t\t" << role_string << "\n\n";
}
