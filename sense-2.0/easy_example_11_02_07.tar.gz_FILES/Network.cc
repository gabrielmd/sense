#include "../common/cost.h"

#include "Station/Station.h"
#include "Sink/Sink.h"
#include "Bridge/Bridge.h"

component Network : public CostSimEng
{
	private: 
		int n_stations;
		int n_bridges;
		int n_sinks;

		Station[] station;
		Bridge[] bridge;
		Sink[] sink;

	public:
		Network();
		void Setup(const char *);
		void Stop(){};
		void Start(){};
};

Network::Network(){							//initialize component Network variables
	n_stations = 1;
	n_sinks = 4;
	n_bridges = 3;
	
	station.SetSize(n_stations);
	bridge.SetSize(n_bridges);
	sink.SetSize(n_sinks);

}

void Network :: Setup(const char *name){	//set up variables (internal and external)

	printf("Start Network Creation\n");

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

	//connecting the modules
	connect station[0].flowGenOut,bridge[0].cac_in;
	connect bridge[0].cac_out,station[0].flowGenIn;

	connect station[0].sourceOut,bridge[0].in;
	connect bridge[0].out[0],bridge[1].in;
	connect bridge[0].out[1],bridge[2].in;
	connect bridge[1].out[0],sink[0].in;
	connect bridge[1].out[1],sink[1].in;
	connect bridge[2].out[0],sink[2].in;
	connect bridge[2].out[1],sink[3].in;
		
	printf("End Network Setup.\n");
	
};

int main(int argc, char* argv[]){

	Network network;
	network.Seed=1;
	network.StopTime=10;
	network.Setup("Network");
	printf("Start!\n\n");
	network.Run();
	return(0);
};
