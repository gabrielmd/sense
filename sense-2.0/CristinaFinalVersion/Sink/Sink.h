#include "../Defs/packet.h"

component Sink : public TypeII
{
	public: 
		inport inline void in(PACKET& p);

		void Setup(const char *, int);

	private:
		int SinkNumber;
		int print_sink_debugging_points;

};

void Sink :: Setup(const char *name, int id)
{
	SinkNumber = id;
	print_sink_debugging_points=0;

};

void Sink :: in(PACKET& packet){

	if(packet.type==DATA && packet.current_bridge==station_info[SinkNumber].AttachedBridge){
		if(print_sink_debugging_points) printf("\t\t\t\tSink %d -> Received packet\n \t\t\t\t\t seqNum: %d from %d to %d, num_hops: %d (at %f)\n\n",SinkNumber,packet.seqNum,packet.source,packet.destination,packet.num_hops,SimTime());
		//if(PRINT_CAC)	printf("\n\tSink %d -> Received packet from %d to %d at %f\n",SinkNumber,packet.source,packet.destination, SimTime());
		if(packet.destination==SinkNumber) received_packets[packet.source][packet.destination]++;
	}
};
