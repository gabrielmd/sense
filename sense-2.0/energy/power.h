#ifndef power_h
#define power_h

#include <utility>

struct PM_Struct
{
    enum {TX=0,RX,IDLE,SLEEP,OFF};
};

component PowerManager : public TypeII, public PM_Struct
{
public:

    double RXPower;
    double TXPower;
    double IdlePower;
    double SleepPower;

	inport inline double switch_state(int state, double time);
	inport inline int state_query();
	inport inline double energy_query();
	outport double to_battery_power(double power, simtime_t time);
	outport double to_battery_query();
	
    void Start();
    void Stop();
   
 private:
    int m_state; 
};

void PowerManager::Start()
{
    m_state=OFF;
}

void PowerManager::Stop()
{

}

double PowerManager::switch_state(int state, double time) 
{
    double power=0.0;	
    switch(m_state)
    {
    case TX:
		power = to_battery_power(TXPower,time);
		break;
    case RX:
		power = to_battery_power(RXPower,time);
		break;
    case IDLE:
		power = to_battery_power(IdlePower,time);
		break;
    case SLEEP:
		power = to_battery_power(SleepPower,time);
		break;
    case OFF:
		power = to_battery_power(0.0,time);
		break;
    default:
        fprintf(stderr,"invalid state to power manager\n");
        break;
    }
    m_state=state;
    return power;

} 
       
int PowerManager::state_query()
{
    return m_state; 
}

double PowerManager::energy_query()
{
    return to_battery_query();
}

#endif /* power_h */
