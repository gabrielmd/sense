#include "../Defs/packet.h"

component Empty : public TypeII{ //this is an empty node with only an input where to conenct all those not used ports
	public: 
		inport inline void in(PACKET& p);
		//outport void out(PACKET& p);
};

void Empty :: in(PACKET& p){}