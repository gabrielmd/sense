
/*************************************************************************
*   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
*   Rensselaer Polytechnic Institute. All worldwide rights reserved.
*   A license to use, copy, modify and distribute this software for
*   non-commercial research purposes only is hereby granted, provided
*   that this copyright notice and accompanying disclaimer is not
*   modified or removed from the software.
*
*   DISCLAIMER: The software is distributed "AS IS" without any
*   express or implied warranty, including but not limited to, any
*   implied warranties of merchantability or fitness for a particular
*   purpose or any warranty of non-infringement of any current or
*   pending patent rights. The authors of the software make no
*   representations about the suitability of this software for any
*   particular purpose. The entire risk as to the quality and
*   performance of the software is with the user. Should the software
*   prove defective, the user assumes the cost of all necessary
*   servicing, repair or correction. In particular, neither Rensselaer
*   Polytechnic Institute, nor the authors of the software are liable
*   for any indirect, special, consequential, or incidental damages
*   related to the software, to the maximum extent the law permits.
*************************************************************************/

#ifndef simple_channel_h
#define simple_channel_h

template <class PACKET>

component SimpleChannel : public TypeII
{
 public:
    enum { FreeSpace, TwoRay };

    int NumNodes;
    bool DumpPackets;
    double CSThresh;
    double RXThresh;
    int PropagationModel;

	inport void from_phy ( PACKET* p, double power, int id);
	outport[] void to_phy (PACKET* p, double power );
	inport void pos_in ( const coordinate_t& pos, int id);
	
	InfiTimer< triple<PACKET*, double, int> > propagation_delay;
	inport void depart( const triple<PACKET*, double, int> & data, unsigned int );

    SimpleChannel() { connect propagation_delay.to_component, depart; }
    virtual ~SimpleChannel() {}
    void Start();
    void Stop();
    void Setup();

 private:    

    std::vector<coordinate_t > m_positions;
};

template <class PACKET>
void SimpleChannel<PACKET>::Setup()
{
    m_positions.reserve(NumNodes);
    for(int i=0;i<NumNodes;i++)
		m_positions.push_back( coordinate_t (0.0, 0.0) );
	to_phy.SetSize(NumNodes);
}
template <class PACKET>
void SimpleChannel<PACKET>::Start()
{
}
template <class PACKET>
void SimpleChannel<PACKET>::Stop()
{
}

template <class PACKET>
void SimpleChannel<PACKET>::pos_in(const coordinate_t & pos, int id)
{
    m_positions[id]=pos;
}

template <class PACKET>
void SimpleChannel<PACKET>::from_phy ( PACKET* p, double power, int in)
{
    double rx_power,sqd,v,dx,dy;

    simtime_t now=SimTime();
    for(int out=0;out<NumNodes;out++)
    {
		if(out==in)continue;
		dx = m_positions[in].x - m_positions[out].x;
		dy = m_positions[in].y - m_positions[out].y;
		sqd = dx*dx + dy*dy ;
		v = p->hdr.wave_length / (4.0*3.14159265);

		if(PropagationModel==FreeSpace)
	    	rx_power = power * v * v / sqd;
		if(PropagationModel==TwoRay)
	    	rx_power = v*v*v*v*power*power / ( sqd * sqd * RXThresh );
	
		if(rx_power>CSThresh)
		{
	    	p->inc_ref();
	    	Printf((DumpPackets,"transmits from %f %f to %f %f\n", 
		    	m_positions[in].x, m_positions[in].y,
		    	m_positions[out].x,m_positions[out].y));
		    propagation_delay.Set(make_triple(p,rx_power,out),now+sqrt(sqd)/speed_of_light);
		}
    }
    p->free();
}

template <class PACKET>
void SimpleChannel<PACKET>::depart ( const triple<PACKET*, double, int> & data , unsigned int index)
{
   	to_phy[data.third](data.first,data.second);
}

#endif /* wireless_channel_h */
