#include "../Fifo/Fifo.h"
#include "../Server/Server.h"
#include "../Defs/packet.h"

component Scheduler : public TypeII{
	public: 
		inport inline void in(PACKET& p);			//input port (packet from previous module)
		outport void out(PACKET& p);				//output port (packet to next module)
		inport inline void to_out(PACKET& p);		//'virtual' incoming port that 'connects' to the real out port
														//this last is created because directly connecting server.out to
														//out gives error. then, we connect server.out to this to_out,
														//and in the function associated to this to_out, i send the packet
														//to the real put port
		
		Fifo fifo;
		Server server;

		Scheduler(); 
		virtual ~Scheduler(){}; 
		void Setup(const char *, int);

	private:
		int SchedulerNumber;
};

Scheduler::Scheduler(){
	connect in,fifo.in;
	connect fifo.out,server.in;
	connect server.next,fifo.next;
	connect server.out,to_out;						//as explained before, we connect the server.out to to_out
}

void Scheduler :: to_out(PACKET& packet){			//and this to_out forward the packet to the real out port
	//printf("scheduler %d sends at %f\n",SchedulerNumber,SimTime());
	out(packet);
}

void Scheduler :: in(PACKET& packet){									//this is the function associated to the incoming 'in' port
	//printf("scheduler %d receives at %f\n",SchedulerNumber,SimTime());	//in here i just forward what i receive to the fifo.in port
	fifo.in(packet);
}

void Scheduler :: Setup(const char *name, int id){
	SchedulerNumber = id;
	
	fifo.Setup("fifo",id);	
	server.Setup("server",id);
};