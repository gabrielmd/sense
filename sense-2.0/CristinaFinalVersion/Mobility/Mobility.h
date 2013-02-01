#include "../Defs/packet.h"

component Mobility : public TypeII{

	public: 
		Mobility();
		virtual ~Mobility(){}; 
		void Setup(const char *, int);
		outport void control_out(int& OldCell, int& NewCell);
		Timer <trigger_t> ComputeNewPosition;
		inport inline void timer_ComputeNewPosition(trigger_t& t);


	private:
		int StationNumber;
};

Mobility::Mobility(){		
	connect ComputeNewPosition.to_component,timer_ComputeNewPosition;
}

void Mobility :: Setup(const char *name, int id){

	StationNumber = id;
	ComputeNewPosition.Set(Tobs);		
	station_info[StationNumber].AttachedBridge = 0;

};


void Mobility :: timer_ComputeNewPosition(trigger_t &){

int change = rand() % 101;
int OldCell;
int NewCell;
int Out = 0;

	OldCell = station_info[StationNumber].AttachedBridge;

	if((ALLOW_OUT == 1) && (OldCell > 6))		//If we allow to exit the topology and we are in a border cell
		Out = 1;

	if(change < ProbHandoff){			//Change Cell

		NewCell = rand()%(cac_info[OldCell].numAdjacentCells+Out);

		while(NewCell==station_info[StationNumber].AttachedBridge){
			NewCell = rand()%(cac_info[OldCell].numAdjacentCells+Out); 
		}
		station_info[StationNumber].AttachedBridge = cac_info[OldCell].AdjacentCells[NewCell];
		
			if(station_info[StationNumber].AttachedBridge == -1){
				if(PRINT_CAC == 1) printf("\n\t\t--Station %d - Out of the topology--\n", StationNumber);
				StationsOutOfTopology++;
				if(StationsOutOfTopology == n_stations)
					printf("\n\t\t--All stations are out of the topology at time %f--\n", SimTime());
			}
			else
				if(PRINT_CAC == 1) printf("\n\t\t--Station %d - Change Cell from %d to %d--\n", StationNumber, OldCell, station_info[StationNumber].AttachedBridge);
		
	control_out(OldCell, station_info[StationNumber].AttachedBridge);
		
	}
	if(station_info[StationNumber].AttachedBridge != -1)	
		ComputeNewPosition.Set(Tobs + SimTime());
};
