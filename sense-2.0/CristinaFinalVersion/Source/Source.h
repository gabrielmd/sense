#include "../Defs/packet.h"

component Source : public TypeII{
	public: 
		outport void out(PACKET& p);

		Timer <trigger_t> interarrival;
		inport inline void timer_interarrival(trigger_t& t);
		inport inline void in(PACKET& p);

		Source(); 
		virtual ~Source(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

		int seqNum;

		int print_source_debugging_points;

	private:
		int SourceNumber;
		int InterarrivalTime;
};

Source::Source(){
	connect interarrival.to_component,timer_interarrival;
}

void Source :: Setup(const char *name, int id){
	SourceNumber = id;
	seqNum=0;
	srand(time(0));
	print_source_debugging_points=0;
};

void Source :: Start(){
	InterarrivalTime = 1;	
};

void Source :: Stop(){
};

void Source :: timer_interarrival(trigger_t &){

	//CBR

	PACKET packet;
	packet.source = SourceNumber;

	if(n_stations == 1)
		packet.destination =SourceNumber;
	else{

		packet.destination = rand()%n_stations;
		while(packet.destination==SourceNumber){
			packet.destination = rand()%n_stations; //(SourceNumber +1) % 2;
		}
	}

	packet.type = 100;
	packet.flowId = 0;
	packet.seqNum = seqNum++; 
	packet.num_hops=0;
	packet.current_bridge=station_info[SourceNumber].AttachedBridge;

	if(print_source_debugging_points) printf("Source %d -> Send packet\n \t seqNum: %d from %d to %d at %f\n\n",SourceNumber,packet.seqNum,packet.source,packet.destination, SimTime());
	if(PRINT_CAC)	printf("\n\tSource %d -> Send packet from %d to %d at %f\n",SourceNumber,packet.source,packet.destination, SimTime());
	interarrival.Set(InterarrivalTime + SimTime());
	generated_packets[packet.source][packet.destination]++;
	out(packet);
};

void Source :: in(PACKET &packet)
{
	if(packet.begin == START){
		interarrival.Set(SimTime());
		if(print_source_debugging_points) printf("Source %d -> Received START packet from FlowGen at %f\n",SourceNumber, SimTime());
		seqNum=0;
	}
	if(packet.begin == STOP){
		interarrival.Cancel();
		if(print_source_debugging_points) printf("Source -> Received STOP packet from FlowGen at %f\n", SimTime());
	}
};

