#include "../Defs/packet.h"

component CAC : public TypeII
{
	public: 
		inport inline void in(PACKET& request);
		outport void out(PACKET& response);

		CAC(){}; 
		virtual ~CAC(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int CACNumber;	

};

void CAC :: Setup(const char *name, int id)
{
	CACNumber = id;

};

void CAC :: Start(){

	cac_info[CACNumber].CACId = CACNumber;	
	cac_info[CACNumber].ActiveFlows = 0;
	cac_info[CACNumber].AdjacentCells = 0;		//For the moment
	cac_info[CACNumber].BW = CellBW;
	cac_info[CACNumber].N = (int)(CellBW / FlowRate);	//2e6 / 64e3 = 31 flows could be accepted
};

void CAC :: Stop(){
};

void CAC :: in(PACKET &request)
{

PACKET response;

	if (request.destination == CACNumber){

	if(request.type == 300){			//Flow Request
		response.source = CACNumber;
		response.destination = request.source;
	
		if (cac_info[CACNumber].ActiveFlows < cac_info[CACNumber].N){

			response.status = ACCEPT;
			cac_info[CACNumber].ActiveFlows++;
				
		}
		else
			response.status = REJECT;

		out(response);
	}
	if (request.type == 400)			//Flow Delete
		cac_info[CACNumber].ActiveFlows--;
	}
};

