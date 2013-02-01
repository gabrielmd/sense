#include "../FlowGen/FlowGen.h"
#include "../Source/Source.h"
#include "../Mobility/Mobility.h"

component Station : public TypeII{
	public: 
		inport inline void flowGenIn(PACKET& p);	//to flowGen: ack from CAC
		outport void flowGenOut(PACKET& p);		//from flowGen: request to CAC
		outport void sourceOut(PACKET& p);		//from source: generated packets
		inport inline void to_flowGenOut(PACKET& p);	//virtual ports
		inport inline void to_sourceOut(PACKET& p);	//virtual ports

		Source source;
		FlowGen flowgen;
		Mobility mobility;

		Station();
		virtual ~Station(){}; 
		void Setup(const char *, int);

	private:
		int StationNumber;
};

Station::Station(){	
	connect flowgen.out,to_flowGenOut;
	connect source.out,to_sourceOut;
	connect flowGenIn,flowgen.in;
	connect flowgen.control_out,source.in;
}

void Station :: flowGenIn(PACKET& packet){
	flowgen.in(packet);
}

void Station :: to_flowGenOut(PACKET& packet){
	flowGenOut(packet);
}

void Station :: to_sourceOut(PACKET& packet){
	sourceOut(packet);
}

void Station :: Setup(const char *name, int id){
	StationNumber = id;
	source.Setup("source",id);
	flowgen.Setup("FlowGen",id);
	mobility.Setup("Mobility",id);
};
