#include "../Fifo/Fifo.h"
#include "../Server/Server.h"
#include "../Defs/packet.h"

component Scheduler : public TypeII{
	public: 
		Fifo fifo;
		Server server;

		inport inline void in(PACKET& p);			//input port (packet from previous module)
		outport void out(PACKET& p);				//output port (packet to next module)
		
		Scheduler(); 
		virtual ~Scheduler(){}; 
		void Setup(const char *, int, int);

	private:
		int bridge_id;
		int SchedulerNumber;
		int print_scheduler_debugging_points;
};

Scheduler::Scheduler(){
	//connect in,fifo.in;
	connect fifo.out,server.in;
	connect server.next,fifo.next;
	connect server.out,out;
}

void Scheduler :: Setup(const char *name, int bid, int sid){
	bridge_id=bid;
	SchedulerNumber = sid;
	print_scheduler_debugging_points=0;
	
	fifo.Setup("fifo",bid,sid);	
	server.Setup("server",bid);
};

void Scheduler :: in(PACKET& packet){
	if(print_scheduler_debugging_points && packet.type==DATA) printf("Scheduler %d in bridge %d receives packet type %d from source %d to destination %d (%f sec)\n\n",SchedulerNumber,bridge_id,packet.type,packet.source,packet.destination,SimTime());
	fifo.in(packet);
}