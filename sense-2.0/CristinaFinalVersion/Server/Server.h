#include "../Defs/packet.h"

component Server : public TypeII{
  public:
    Server();
	virtual ~Server() {}
	void Setup(const char *, int);

    double service_time;							// the average service time

    inport inline void in(PACKET& p);				//incoming packet (from queue)
    outport void out(PACKET& p);					//outgoing packet
    outport void next();							//tell the queue to send the next packet
	
    inport inline void packet_served(trigger_t& t);	//port associated to the servce timer
    Timer<trigger_t> serving;

  private:
	int ServerNumber;
    PACKET packet_to_serve;							//temporary variable packet 
};

Server :: Server(){
    connect serving.to_component,packet_served;
}

void Server :: Setup(const char *name, int id){
	ServerNumber = id;
	service_time=0.5;
};

void Server :: in(PACKET& packet){
	//printf("server %d receives at %f\n",ServerNumber,SimTime());
	packet_to_serve=packet;								//when a packet arrives,
	serving.Set(SimTime() + service_time + Exponential(service_time/100));						// we set up the timer
	return;
}

void Server :: packet_served(trigger_t& t){
	//printf("server %d serves t %f\n",ServerNumber,SimTime());
    out(packet_to_serve);							//when serving timer expires, we send the packet
    next();											//and notify the queue that the server is available
    return;
}
