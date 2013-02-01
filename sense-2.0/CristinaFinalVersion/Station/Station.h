#include "../FlowGen/FlowGen.h"
#include "../Source/Source.h"
#include "../Mobility/Mobility.h"

component Station : public TypeII{
	public: 
		Source source;
		FlowGen flowgen;
		Mobility mobility;

		inport inline void flowGenIn(PACKET& p);	//to flowGen: ack from CAC
		outport void flowGenOut(PACKET& p);		//from flowGen: request to CAC
		outport void sourceOut(PACKET& p);		//from source: generated packets		

		Station();
		virtual ~Station(){}; 
		void Setup(const char *, int);
		

	private:
		int StationNumber;
};

Station::Station(){	
	connect flowGenIn,flowgen.in;
	connect flowgen.control_out,source.in;
	connect flowgen.out,flowGenOut;
	connect source.out,sourceOut;
	connect mobility.control_out, flowgen.control_in;
}

void Station :: Setup(const char *name, int id){
	StationNumber = id;
	source.Setup("source",id);
	flowgen.Setup("FlowGen",id);
	mobility.Setup("Mobility",id);
};


