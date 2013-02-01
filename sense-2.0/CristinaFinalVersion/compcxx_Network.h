#include <assert.h> 
 #include<vector> 
template <class T> class compcxx_array { public: 
virtual ~compcxx_array() { for (typename std::vector<T*>::iterator i=m_elements.begin();i!=m_elements.end();i++) delete (*i); } 
void SetSize(unsigned int n) { for(unsigned int i=0;i<n;i++)m_elements.push_back(new T); } 
T& operator [] (unsigned int i) { assert(i<m_elements.size()); return(*m_elements[i]); } 
unsigned int size() { return m_elements.size();} 
private: std::vector<T*> m_elements; }; 
class compcxx_component; 
template <class T> class compcxx_functor {public: 
void Connect(compcxx_component&_c, T _f){ c.push_back(&_c); f.push_back(_f); } 
protected: std::vector<compcxx_component*>c; std::vector<T> f; }; 
class compcxx_component { public: 
typedef void  (compcxx_component::*Bridging_out_f_t)(PACKET& p);
typedef void  (compcxx_component::*Scheduler_out_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out0_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out1_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out2_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out3_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out4_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out5_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_out6_f_t)(PACKET& p);
typedef void  (compcxx_component::*Bridge_cac_out_f_t)(PACKET& p);
typedef void  (compcxx_component::*Station_flowGenOut_f_t)(PACKET& p);
typedef void  (compcxx_component::*Station_sourceOut_f_t)(PACKET& p);
};
