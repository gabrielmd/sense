
component Stats : public TypeII{
	public: 
		Stats(){};
		virtual ~Stats(){};
		
		void print_station_stats(int n);			//prints station statistics such as generated and received packets (end-to-end)
		void print_bridge_port_stats(int n_b, int n_p);
		void print_global_stats();
		void init_stats();
};

void Stats :: print_global_stats(){
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF RECEIVED PACKETS IN BRIDGING MODULES:\t\t%d\n",total_bridging_received_data_packets);
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF FORWARDED PACKETS IN BRIDGING MODULES:\t\t%d\n",total_bridging_forwarded_data_packets);
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF BROADCASTED PACKETS IN BRIDGING MODULES:\t%d\n\n",total_bridging_broadcasted_data_packets);
}

void Stats :: print_station_stats(int n_stations){
	for(int so=0;so<n_stations;so++){
		if(PRINT_STP==1)	printf("Station %d\n",so);
		for(int si=0;si<n_stations;si++){
			if(PRINT_STP==1)	printf("\tto station %d:\t %d generated packets\n",si,generated_packets[so][si]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
	
	for(int so=0;so<n_stations;so++){
		if(PRINT_STP==1)	printf("Station %d\n",so);
		for(int si=0;si<n_stations;si++){
			if(PRINT_STP==1)	printf("\tto station %d:\t %d received packets\n",si,received_packets[so][si]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
}

void Stats :: print_bridge_port_stats(int n_bridges, int n_ports){
	for(int b=0;b<n_bridges;b++){
		if(PRINT_STP==1)	printf("Bridge %d\n",b);
		for(int p=0;p<n_ports;p++){
			if(PRINT_STP==1)	printf("\tport %d\n",p);
			if(PRINT_STP==1)	printf("\t\tIN:\t\tdata:\t%d\tbpdu:\t%d\n",received_data_packets[b][p],received_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\tOUT:\t\tdata:\t%d\tbpdu:\t%d\n",forwarded_data_packets[b][p],forwarded_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  LOST BACKUP:\tdata:\t%d\tbpdu:\t%d\n",backup_lost_data_packets[b][p],backup_lost_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  LOST QUEUE:\tdata:\t%d\tbpdu:\t%d\n",queue_lost_data_packets[b][p],queue_lost_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  UTILIZATION:\tdata:\t%0.2f\tbpdu:\t%0.2f\n",utilization_data_packets[b][p],utilization_bpdu_packets[b][p]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
}

void Stats :: init_stats(){
	total_bridging_received_data_packets=0;
	total_bridging_forwarded_data_packets=0;
	total_bridging_broadcasted_data_packets=0;
	
	for(int i=0;i<MAX_STATIONS;i++){
		for(int j=0;j<MAX_STATIONS;j++){
			generated_packets[i][j]=0;
			received_packets[i][j]=0;
		}
	}

	for(int i=0;i<MAX_STATIONS;i++){
		for(int j=0;j<MAX_PORTS;j++){
			received_data_packets[i][j]=0;
			received_bpdu_packets[i][j]=0;
			forwarded_data_packets[i][j]=0;
			forwarded_bpdu_packets[i][j]=0;

			backup_lost_data_packets[i][j]=0;
			backup_lost_bpdu_packets[i][j]=0;

			queue_lost_data_packets[i][j]=0;
			queue_lost_bpdu_packets[i][j]=0;

			utilization_data_packets[i][j]=0.0;
			utilization_bpdu_packets[i][j]=0.0;
		}
	}
}
