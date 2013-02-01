#include "../Defs/packet.h"

component Bridging : public TypeII
{
	public: 

		int bridge_id;
		vector<MAC_ENTRY> mac_table; //int mac_destination; int outport; double age;

		inport inline void in(PACKET& p, int ip);

		outport[] void out(PACKET& p);

		inport inline void in_bpdu(PACKET& p, int op);	//STP sends a BPDU to be sent through port op
		outport void out_bpdu(PACKET& p, int op);

		int search_destination_MAC_table(int d);
		void add_destination_MAC_table(int s, int p);

		Bridging(); 
		virtual ~Bridging(){}; 
		void Setup(const char *, int );
		void print_mac_table();		//prints the port information

		int print_bridging_debugging_points;
	private:
};

Bridging :: Bridging(){
	out.SetSize(MAX_PORTS);
};

void Bridging :: Setup(const char *name, int bid){
	bridge_id=bid;
	print_bridging_debugging_points=0;
}

int Bridging :: search_destination_MAC_table(int d){ //returns out port (-1 if not found)
	int i=0;
	int found=0;
	int op=-1;
	int v_size=mac_table.size();
	while(!found && i<v_size){
		if((mac_table.at(i)).mac_destination==d){
			op=(mac_table.at(i)).outport;
			found=1;
		}
		i++; 
	}
	return(op);	
}

void Bridging :: add_destination_MAC_table(int s, int p){
	int i=0;
	int found=0;
	int v_size=mac_table.size();
	while(!found && i<v_size){ 				//check all entries
		if((mac_table.at(i)).mac_destination==s){
			vector<MAC_ENTRY>::iterator elem;
			elem=mac_table.begin() + i;
			mac_table.erase(elem);	//remove it
			MAC_ENTRY temp_m_e;
			temp_m_e.mac_destination=s;
			temp_m_e.outport=p;
			temp_m_e.age=1000;
			mac_table.push_back(temp_m_e);		//and store the new value
			found=1;
		}
		i++; 
	}
	if(found==0){						//if not found
		MAC_ENTRY temp_m_e;				//store new value
		temp_m_e.mac_destination=s;
		temp_m_e.outport=p;
		temp_m_e.age=1000;
		mac_table.push_back(temp_m_e);
	}
}

void Bridging :: in(PACKET &packet,int ip){ //ip->inputport , bid->

	if(packet.type==DATA){		//if data packet received
		packet.num_hops++;
		received_data_packets[bridge_id][ip]++;
		total_bridging_received_data_packets++;
		
		//update MAC table with packet origin
		add_destination_MAC_table(packet.source,ip);
		
		//if destination is in the MAC table
		int op=search_destination_MAC_table(packet.destination);
		if(op!=-1){
			total_bridging_forwarded_data_packets++;
			if(port[bridge_id][op].state==FORWARDING){  //does other state forward packets?
				out[op](packet);
				forwarded_data_packets[bridge_id][op]++;
			}
			else{
				backup_lost_data_packets[bridge_id][op]++;
			}
		}
		else{ //if it is not
			total_bridging_broadcasted_data_packets++;
			//broadcast to all other ports if forwarding
			if(print_bridging_debugging_points) printf("Bridging %d broadcasts packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,ip,SimTime());
			for(int i=0;i<N_PORTS;i++){
				if(i!=ip){
					if(port[bridge_id][i].state==FORWARDING){  //does other state forward packets?
						out[i](packet);
						forwarded_data_packets[bridge_id][i]++;
					}
					else{
						backup_lost_data_packets[bridge_id][i]++;
					}
				}
			}
		}
	}
	
	if(packet.type==BPDU){		//if bpdu packet received
		received_bpdu_packets[bridge_id][ip]++;
		out_bpdu(packet,ip);
	}
};

void Bridging :: in_bpdu(PACKET &packet, int op){

	if(packet.type==BPDU){		//it should always be a bpdu since this port is connected to STP module
		forwarded_bpdu_packets[bridge_id][op]++;
		out[op](packet);
	}
	else
		cout << "Bridging module: a data packet received through bpdu inport!!!\n\n";
};

void Bridging :: print_mac_table(){
	cout << "BRIDGE_ID:\t" << bridge_id << "\n";
	if(mac_table.empty())	cout << "\tEMPTY\n";
	for(int i=0; i<(signed int)mac_table.size();i++)
		cout << "\tMAC:\t" << mac_table[i].mac_destination << "\tOUTPORT:\t" << mac_table[i].outport << "\n";
	cout << "\n";
}
