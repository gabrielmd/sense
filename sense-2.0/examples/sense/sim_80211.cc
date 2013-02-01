#define queue_t HeapQueue

#include "../../common/sense.h"
#include "../../app/random_neighbor.h"
#include "../../mob/immobile.h"
#include "../../mac/mac_80211.h"
#include "../../mac/null_mac.h"
#include "../../phy/transceiver.h"
#include "../../phy/simple_channel.h"
#include "../../energy/battery.h"
#include "../../energy/power.h"

#cxxdef mac_component MAC80211
#cxxdef mac_struct MAC80211_Struct

typedef mac_struct <RandomNeighbor_Struct::packet_t> :: packet_t mac_packet_t;

component SensorNode : public TypeII
{
public:

    RandomNeighbor app;
    Immobile mob;
    mac_component <RandomNeighbor_Struct::packet_t> mac;
    DuplexTransceiver < mac_packet_t > phy;
    SimpleBattery battery;
    PowerManager pm;

    double MaxX, MaxY;
    ether_addr_t MyEtherAddr;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int ID;

    int SentPackets, RecvPackets;

    SensorNode();
    virtual ~SensorNode();
    void Start();
    void Stop();
    void Setup();

	outport void to_channel_packet(mac_packet_t* packet, double power, int id);
	inport void from_channel (mac_packet_t* packet, double power);
	outport void to_channel_pos(coordinate_t& pos, int id);
};

SensorNode::SensorNode()
{
}
SensorNode::~SensorNode()
{
}

void SensorNode::Start()
{

}

void SensorNode::Stop()
{
    SentPackets=app.SentPackets;
    RecvPackets=app.RecvPackets;
}

void SensorNode::Setup()
{

    battery.InitialEnergy=1e6;
    
    app.BroadcastInterval=BroadcastInterval;
    app.UnicastInterval=UnicastInterval;
    app.MyEtherAddr=MyEtherAddr;
    app.MaxPacketSize=2*mac.RTSThreshold;
    app.DumpPackets=true;

    mob.InitX=Random(MaxX);
    mob.InitY=Random(MaxY);
    mob.ID=ID;

    mac.MyEtherAddr=MyEtherAddr;
    mac.DumpPackets=true;
    mac.Promiscuity=false;

   	pm.TXPower=1.6;
   	pm.RXPower=1.2;
   	pm.IdlePower=1.15;

    phy.TXPower=0.02283;
    phy.TXGain=1.0;
    phy.RXGain=1.0;
    phy.Frequency=9.14e8;
    phy.RXThresh=3.652e-10;
    phy.CSThresh=1.559e-11;
    phy.ID=ID;

    connect app.to_transport, mac.from_network;
    connect mac.to_network_data, app.from_transport_data;
    connect mac.to_network_ack, app.from_transport_ack;

    connect mac.to_phy, phy.from_mac;
    connect phy.to_mac, mac.from_phy;

    connect phy.to_channel, to_channel_packet;
    connect from_channel, phy.from_channel;
    connect phy.to_power_switch, pm.switch_state;
    
    connect pm.to_battery_query, battery.query_in;
    connect pm.to_battery_power, battery.power_in;

    connect mob.pos_out, to_channel_pos;
}

component Sim_80211 : public CostSimEng
{
public:

    void Stop();

    double MaxX, MaxY;
    simtime_t BroadcastInterval;
    simtime_t UnicastInterval;
    int NumNodes;

    SensorNode [] nodes;
    SimpleChannel < mac_packet_t > channel;
    
    void Setup();
};

void Sim_80211 :: Stop()
{
    int i,sent=0,recv=0;
    for(i=0;i<NumNodes;i++)
    {
	    sent+=nodes[i].SentPackets;
	    recv+=nodes[i].RecvPackets;
    }

    printf("total packets sent: %d, received: %d\n",sent,recv);
}
void Sim_80211 :: Setup()
{
    int i;
    nodes.SetSize(NumNodes);
    for(i=0;i<NumNodes;i++)
    {
	    nodes[i].MaxX=MaxX;
	    nodes[i].MaxY=MaxY;
    	nodes[i].MyEtherAddr=i;
	    nodes[i].BroadcastInterval=BroadcastInterval;
	    nodes[i].UnicastInterval=UnicastInterval;
	    nodes[i].ID=i;
        nodes[i].Setup();
    }    

    channel.NumNodes=NumNodes;
    channel.DumpPackets=false;
    channel.CSThresh=nodes[0].phy.CSThresh;   
    channel.RXThresh=nodes[0].phy.RXThresh;
    channel.PropagationModel=channel.FreeSpace;
    channel.Setup();

    for(i=0;i<NumNodes;i++)
    {
		connect nodes[i].to_channel_packet,channel.from_phy;
		connect nodes[i].to_channel_pos,channel.pos_in;
		connect channel.to_phy[i],nodes[i].from_channel ;
    }

}

int main(int argc, char* argv[])
{
    Sim_80211 sim;

    sim.StopTime=1000;
    sim.Seed=1234;

    sim.MaxX=5000;
    sim.MaxY=5000;
    sim.BroadcastInterval=5;
    sim.UnicastInterval=10;
    sim.NumNodes=100;

    if(argc>=2)sim.StopTime=atof(argv[1]);
    if(argc>=3)sim.NumNodes=atoi(argv[2]);
    if(argc>=4)sim.MaxX=sim.MaxY=atof(argv[3]);

    printf("StopTime: %.1f, NumNodes: %d, Terrain: %.0f by %.0f\n", 
	   sim.StopTime, sim.NumNodes, sim.MaxX, sim.MaxY);

    sim.Setup();
    sim.Run();

    return 0;
}
