#include "../Defs/packet.h"

component Bridging : public TypeII
{
	public: 
		inport inline void in(PACKET& p);	//we get a packet and the output port

		outport[] void out(PACKET& p);

		Bridging(); 
		virtual ~Bridging(){}; 
		void Setup(const char *, int ){};

	private:
};

Bridging :: Bridging(){
	out.SetSize(2);
};

void Bridging :: in(PACKET &packet){
	out[0](packet);
	out[1](packet);
};
