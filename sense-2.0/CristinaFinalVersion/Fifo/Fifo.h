#include "../Defs/packet.h"

component Fifo : public TypeII{
  public:
	Fifo(){};
	virtual ~Fifo(){};
	void Setup(const char *, int, int);
	void Start();

	unsigned int queue_length;

	inport inline void in(PACKET& p);	//incoming packet
	outport void out(PACKET& p);		//outgoing packet
	inport inline void next();			//trigger from the server when next packet has to be delivered (for serving)

  private:
	int bridge_id;
	int port_id;
	int FifoNumber;
	bool busy_server;					//indicates if the server is busy
	std::deque<PACKET> queue;			//standard queue object
};

void Fifo :: Setup(const char *name, int bid, int pid){
	bridge_id=bid;
	port_id=pid;
	//FifoNumber = pid;
	queue_length=100;//global_queue_length;
};

void Fifo :: Start(){
	busy_server=false;					//initialization: the server is available
}

void Fifo::in(PACKET& packet){			//function called when a packet triggers the in port
	//printf("fifo %d receives at %f\n",FifoNumber,SimTime());
	if (!busy_server){					// if server is free, we can pass it through
        	out(packet);					// and directly send the packet to the out port (where the server is connected)
        	busy_server=true;				//then, the server will be busy, until it warns us 
	} 
	else{
		if (queue.size() < queue_length){	// queue is not full
			queue.push_back(packet);		//i add the packet at the end
		}
		else{					//the packet gets lost
			if(packet.type==DATA) queue_lost_data_packets[bridge_id][port_id]++;
			if(packet.type==BPDU) queue_lost_bpdu_packets[bridge_id][port_id]++;
		}
	}
    return;
}

void Fifo :: next(){
	//printf("fifo %d delivers at %f\n",FifoNumber,SimTime());
	if (queue.size()>0){				//if the queue is not empty
		out(queue.front());				//send to the ouport (server) the first packet in the queue	
		queue.pop_front();				//remove this first packet
	} 
	else{
		busy_server=false;					//if the queue is empty, no more packets to serve, then server is available
	}
	return;
}