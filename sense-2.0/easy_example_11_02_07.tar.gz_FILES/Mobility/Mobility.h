#include "../Defs/packet.h"

component Mobility : public TypeII{

	public: 
		Mobility();
		virtual ~Mobility(){}; 
		void Setup(const char *, int);
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

};


void Mobility :: timer_ComputeNewPosition(trigger_t &){

int change = rand() % 101;

	printf("change: %d\n", change);

	if(change < ProbHandoff){	//Change Cell

		printf("Change Cell\n");

	}
	else
		printf("Continue in the same cell\n");
		
	ComputeNewPosition.Set(Tobs + SimTime());
};
