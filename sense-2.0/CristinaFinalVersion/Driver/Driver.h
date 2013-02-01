#include "../Defs/packet.h"

component Driver : public TypeII
{
	public: 
		inport inline void in0(PACKET& p);	//we get a packet and the output port
		inport inline void in1(PACKET& p);	//we get a packet and the output port
		inport inline void in2(PACKET& p);	//we get a packet and the output port
		inport inline void in3(PACKET& p);	//we get a packet and the output port
		inport inline void in4(PACKET& p);	//we get a packet and the output port
		inport inline void in5(PACKET& p);	//we get a packet and the output port
		inport inline void in6(PACKET& p);	//we get a packet and the output port

		outport void out0(PACKET& p);
		outport void out1(PACKET& p);
		outport void out2(PACKET& p);
		outport void out3(PACKET& p);
		outport void out4(PACKET& p);
		outport void out5(PACKET& p);
		outport void out6(PACKET& p);

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
void Driver :: in2(PACKET &packet){
	out2(packet);
};
void Driver :: in3(PACKET &packet){
	out3(packet);
};
void Driver :: in4(PACKET &packet){
	out4(packet);
};
void Driver :: in5(PACKET &packet){
	out5(packet);
};
void Driver :: in6(PACKET &packet){
	out6(packet);
};
