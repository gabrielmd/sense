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
	packet.destination = 1;
	packet.flowId = 0;
	packet.seqNum = seqNum++; 
	printf("Source %d -> Send packet\n \t seqNum: %d from %d to %d at %f\n\n",SourceNumber,packet.seqNum,packet.source,packet.destination, SimTime());
	interarrival.Set(InterarrivalTime + SimTime());
	out(packet);
};

void Source :: in(PACKET &packet)
{
	if(packet.begin == START){
		interarrival.Set(SimTime());
		printf("Source %d -> Received START packet from FlowGen at %f\n\n",SourceNumber, SimTime());
		seqNum=0;
	}
	if(packet.begin == STOP){
		interarrival.Cancel();
		printf("Source -> Received STOP packet from FlowGen at %f\n\n", SimTime());
	}
};

