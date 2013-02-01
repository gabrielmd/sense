#include "../Defs/packet.h"

component Sink : public TypeII
{
	public: 
		inport inline void in(PACKET& p);

		void Setup(const char *, int);

	private:
		int SinkNumber;

};

void Sink :: Setup(const char *name, int id)
{
	SinkNumber = id;

};

void Sink :: in(PACKET& packet){
	printf("\t\t\t\tSink %d -> Received packet\n \t\t\t\t\t seqNum: %d from %d to %d at %f\n\n",SinkNumber,packet.seqNum,packet.source,packet.destination, SimTime());
};
