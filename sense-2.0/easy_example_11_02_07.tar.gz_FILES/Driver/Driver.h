#include "../Defs/packet.h"

component Driver : public TypeII
{
	public: 
		inport inline void in0(PACKET& p);	//we get a packet and the output port
		inport inline void in1(PACKET& p);	//we get a packet and the output port
		//inport inline void in2(PACKET& p);	//we get a packet and the output port
		//inport inline void in3(PACKET& p);	//we get a packet and the output port

		outport void out0(PACKET& p);
		outport void out1(PACKET& p);

		Driver(){}; 
		virtual ~Driver(){}; 
		void Setup(const char *, int ){};

	private:
};

void Driver :: in0(PACKET &packet){
	out0(packet);
};
void Driver :: in1(PACKET &packet){
	out1(packet);
};
