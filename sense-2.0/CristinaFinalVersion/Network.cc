#include "../common/cost.h"

#include "Station/Station.h"
#include "Sink/Sink.h"
#include "Bridge/Bridge.h"
#include "Stats/stats.h"
#include "Empty/empty.h"

component Network : public CostSimEng
{
		Station[] station;
		Bridge[] bridge;
		Sink[] sink;
		Stats stats;
		Empty empty;

	public:
		Network();
		void Setup(const char *);
		void Stop();
		void Start();

		inport inline void start_stats_collection(trigger_t& t);	//port associated to the servce timer
		Timer<trigger_t> start_stats;			//timer to start collecting statistic
};

Network::Network(){					
	//initialize component Network variable
	n_stations = 100;
	n_sinks = n_stations;
	n_bridges = 19;
	
	station.SetSize(n_stations);
	bridge.SetSize(n_bridges);
	sink.SetSize(n_sinks);

	connect start_stats.to_component,start_stats_collection;
}

void Network :: Start(){
	printf("\nStart!\n\n");
	start_stats.Set(9); //time to start collecting stats: 9 sec (after all ports into forwarding)
}

void Network :: Stop(){
	printf("\nEnd of simulation!\n\n");
	stats.print_global_stats();
	stats.print_station_stats(n_stations);
	stats.print_bridge_port_stats(n_bridges,N_PORTS);
}

void Network :: start_stats_collection(trigger_t& t){
	stats.init_stats();
}

void Network :: Setup(const char *name){	//set up variables (internal and external)

	printf("\nStart Network Creation\n\n");

	StationsOutOfTopology = 0;

	//sources
	for(int n=0; n < n_stations; n++){
		station[n].Setup("station", n);
	}

	//Bridges
	for(int n=0; n < n_bridges; n++){
		bridge[n].Setup("bridge",n);	
	}
	
	//sinks
	for(int n=0; n < n_sinks; n++){
		sink[n].Setup("sink",n);
	}

	//initialize adjacent cells information

	for(int i=0; i < n_bridges; i++){
		for(int j=0; j < n_bridges; j++)
			 cac_info[i].AdjacentCells[j] = -1;

	}

	//6 bridges(4 ports) amb una estacio i una sink cadascun

	//bridge <-> bridge
	connect bridge[0].out1,bridge[1].in1;	//bridge 0 <-> bridge 1
	connect bridge[1].out1,bridge[0].in1;

	connect bridge[0].out2,bridge[2].in1;	//bridge 0 <-> bridge 2
	connect bridge[2].out1,bridge[0].in2;

	connect bridge[0].out3,bridge[3].in1;	//bridge 0 <-> bridge 3
	connect bridge[3].out1,bridge[0].in3;

	connect bridge[0].out4,bridge[4].in1;	//bridge 0 <-> bridge 4
	connect bridge[4].out1,bridge[0].in4;

	connect bridge[0].out5,bridge[5].in1;	//bridge 0 <-> bridge 6
	connect bridge[5].out1,bridge[0].in5;

	connect bridge[0].out6,bridge[6].in1;	//bridge 0 <-> bridge 1
	connect bridge[6].out1,bridge[0].in6;	

	for(int i=0; i<=5; i++){ 	//First Cell
		 cac_info[0].AdjacentCells[i] = i+1;
		 cac_info[i+1].AdjacentCells[0] = 0;		

	}

	for(int b=0; b<6; b++){	//Connections between first ring bridges

 		connect bridge[b+1].out2,bridge[((b+1)%6)+1].in3;
		connect bridge[((b+1)%6)+1].out3,bridge[b+1].in2;

		 cac_info[b+1].AdjacentCells[1] = ((b+1)%6)+1;
		 cac_info[((b+1)%6)+1].AdjacentCells[2] = b+1;

	}

	for(int b=7; b<18; b++){	//Connections between second ring bridges

 		connect bridge[b].out2,bridge[b+1].in3;
		connect bridge[b+1].out3,bridge[b].in2;

		cac_info[b].AdjacentCells[1] = b+1;
		cac_info[b+1].AdjacentCells[2] = b;

	}

	connect bridge[18].out2,bridge[7].in3;
	connect bridge[7].out3,bridge[8].in2;		

	cac_info[18].AdjacentCells[1] = 7;
	cac_info[7].AdjacentCells[2] = 18;

	int count_b=8;
	for(int b=2; b<=6; b++){	//Connections between second and third rings

 		connect bridge[b].out4,bridge[count_b].in4;
		connect bridge[count_b].out4,bridge[b].in4;

		cac_info[b].AdjacentCells[3] = count_b;
		cac_info[count_b].AdjacentCells[3] = b;

		count_b++;
 		connect bridge[b].out5,bridge[count_b].in1;
		connect bridge[count_b].out1,bridge[b].in5;

		cac_info[b].AdjacentCells[4] = count_b;
		cac_info[count_b].AdjacentCells[0] = b;

		count_b++;
 		connect bridge[b].out6,bridge[count_b].in1;
		connect bridge[count_b].out1,bridge[b].in6;

		cac_info[b].AdjacentCells[5] = count_b;
		cac_info[count_b].AdjacentCells[0] = b;

	}
	connect bridge[1].out4,bridge[18].in4;
	connect bridge[18].out4,bridge[1].in4;

	cac_info[1].AdjacentCells[3] = 18;
	cac_info[18].AdjacentCells[3] = 1;

	connect bridge[1].out5,bridge[7].in1;
	connect bridge[7].out1,bridge[0].in5;

	cac_info[1].AdjacentCells[4] = 7;
	cac_info[7].AdjacentCells[0] = 1;

	connect bridge[1].out6,bridge[8].in1;
	connect bridge[8].out1,bridge[1].in6;

	cac_info[1].AdjacentCells[5] = 8;
	cac_info[8].AdjacentCells[0] = 1;

	for(int i=0; i <=6; i++)
		cac_info[i].numAdjacentCells = 6;
	for(int i=7; i <=18; i++)
		if (i%2 == 0) //Par
			cac_info[i].numAdjacentCells = 4;
		else
			cac_info[i].numAdjacentCells = 3;

	//Print adjacent cells information

	if(PRINT_CAC == 1){
		for(int i=0; i < n_bridges; i++){
			for(int j=0; j < cac_info[i].numAdjacentCells; j++)
				printf("Cell %d adjacent %d\n", i, cac_info[i].AdjacentCells[j]);
			printf("\n");

		}
	}

	//bridge <-> station (source) - always in bridge port 0
	for(int s=0; s<n_stations ; s++){
		for(int b=0; b<n_bridges ; b++){
			connect station[s].sourceOut,bridge[b].in0;
			connect bridge[b].out0,sink[s].in;
		}
	}

	//bridge <-> station (CAC)
	for(int s=0; s<n_stations ; s++){
		for(int b=0; b<n_bridges ; b++){
			connect station[s].flowGenOut,bridge[b].cac_in;
			connect bridge[b].cac_out,station[s].flowGenIn;
		}
	}

	//all ports connected to empty (always highest port id's)
	for(int i=7; i<n_bridges; i++){ //all bridges except bridge 0 (root, center of star)
		connect bridge[i].out5,empty.in;
		connect bridge[i].out6,empty.in;
	}
	connect bridge[7].out4,empty.in;
	connect bridge[9].out4,empty.in;
	connect bridge[11].out4,empty.in;
	connect bridge[13].out4,empty.in;
	connect bridge[15].out4,empty.in;
	connect bridge[17].out4,empty.in;

	
	printf("End Network Setup\n\n");
	
};

int main(int argc, char* argv[]){

	Network network;
	network.Seed=1;
	network.StopTime=100000;
	network.Setup("Network");
	network.Run();
	return(0);
};
