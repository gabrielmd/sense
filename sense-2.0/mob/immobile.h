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

#ifndef immobile_h
#define immobile_h

component Immobile : public TypeII
{
public:
	Timer <trigger_t> timer;    
    outport void pos_out (coordinate_t& pos, int id);
	inport void announce_pos (trigger_t& t);

    double InitX, InitY;
    int ID;

	Immobile () { connect timer.to_component, announce_pos; }
    void Start();
    void Stop();
};
void Immobile::announce_pos(trigger_t&)
{
    coordinate_t pos=coordinate_t(InitX,InitY);
    pos_out(pos,ID);
}
void Immobile::Start()
{
	timer.Set(0.0);
}
void Immobile::Stop()
{
}
#endif /* immobile_h*/
