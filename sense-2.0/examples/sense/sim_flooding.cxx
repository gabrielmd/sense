
#line 48 "sim_routing.cc"
#define queue_t HeapQueue






#line 1 "../../common/sense.h"































#ifndef SENSE_H
#define SENSE_H


#line 1 "../../common/config.h"
#define HAVE_INT8_T 1
#define HAVE_INT16_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1

#define HAVE_UINT8_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1

#if 1
#include <inttypes.h>
#endif

#if !HAVE_INT8_T
typedef signed char int8_t;
#endif
#if !HAVE_INT16_T
typedef signed short int16_t;
#endif
#if !HAVE_INT32_T
typedef signed long int32_t;
#endif
#if !HAVE_INT64_T
typedef signed long long int64_t;
#endif


#if !HAVE_UINT8_T
typedef unsigned char uint8_t;
#endif
#if !HAVE_UINT16_T
typedef unsigned short uint16_t;
#endif
#if !HAVE_UINT32_T
typedef unsigned long uint32_t;
#endif
#if !HAVE_UINT64_T
typedef unsigned long long uint64_t;
#endif



#line 35 "../../common/sense.h"

const double speed_of_light=299792458.0;

#include <string>
using std::string;







#line 1 "../../common/cost.h"
#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 "../../common/priority_q.h"

























#include <string.h>

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H














template < class ITEM >
class SimpleQueue 
{
public:
    SimpleQueue() :m_head(NULL) {};
    void EnQueue(ITEM*);
    ITEM* DeQueue();
    void Delete(ITEM*);
    ITEM* NextEvent() const { return m_head; };
    const char* GetName();
protected:
    ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
    static const char* name = "SimpleQueue";
    return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
    if( m_head==NULL || item->time < m_head->time )
    {
        if(m_head!=NULL)m_head->prev=item;
        item->next=m_head;
        m_head=item;
	item->prev=NULL;
        return;
    }
    
    ITEM* i=m_head;
    while( i->next!=NULL && item->time > i->next->time)
        i=i->next;
    item->next=i->next;
    if(i->next!=NULL)i->next->prev=item;
    i->next=item;
    item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
    if(m_head==NULL)return NULL;
    ITEM* item = m_head;
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
    return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
    if(item==NULL) return;

    if(item==m_head)
    {
        m_head=m_head->next;
	if(m_head!=NULL)m_head->prev=NULL;
    }
    else
    {
        item->prev->next=item->next;
        if(item->next!=NULL)
	    item->next->prev=item->prev;
    }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
    void Delete(ITEM*);
    void EnQueue(ITEM*);
    bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{
    ITEM* i=SimpleQueue<ITEM>::m_head;
    while(i!=NULL)
    {
	if(i==item)
	{
	    pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
	}
	i=i->next;
    }
    SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
    ITEM* i=SimpleQueue<ITEM>::m_head;
    while(i!=item&&i!=NULL)i=i->next;
    if(i==NULL)
	pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
    else
        SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
    char out[1000],buff[100];

    ITEM* i=SimpleQueue<ITEM>::m_head;
    bool qerror=false;

    sprintf(out,"queue error %s : ",s);
    while(i!=NULL)
    {
	sprintf(buff,"%f ",i->time);
	strcat(out,buff);
	if(i->next!=NULL)
	    if(i->next->prev!=i)
	    {
		qerror=true;
		sprintf(buff," {broken} ");
		strcat(out,buff);
	    }
	if(i==i->next)
	{
	    qerror=true;
	    sprintf(buff,"{loop}");
	    strcat(out,buff);
	    break;
	}
	i=i->next;
    }
    if(qerror)
	printf("%s\n",out);
    return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
    ITEM* DeQueue(double);
    const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
    static const char* name = "ErrorQueue";
    return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
    

    if(drand48()>0.5)
	return SimpleQueue<ITEM>::DeQueue();

    int s=0;
    ITEM* e;
    e=SimpleQueue<ITEM>::m_head;
    while(e!=NULL&&e->time<stoptime)
    {
	s++;
	e=e->next;
    }
    e=SimpleQueue<ITEM>::m_head;
    s=(int)(s*drand48());
    while(s!=0)
    {
	e=e->next;
	s--;
    }
    Delete(e);
    return e;
}

template < class ITEM >
class HeapQueue 
{
public:
        HeapQueue();
        ~HeapQueue();
        void EnQueue(ITEM*);
        ITEM* DeQueue();
        void Delete(ITEM*);
	const char* GetName();
	ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
private:
        void SiftDown(int);
        void PercolateUp(int);
	void Validate(const char*);
        
        ITEM** elems;
        int num_of_elems;
        int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
    static const char* name = "HeapQueue";
    return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
    int i,j;
    char out[1000],buff[100];
    for(i=0;i<num_of_elems;i++)
	if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	     ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
	{
	    sprintf(out,"queue error %s : ",s);
	    for(j=0;j<num_of_elems;j++)
	    {
		if(i!=j)
		    sprintf(buff,"%f(%d) ",elems[j]->time,j);
		else
		    sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
		strcat(out,buff);
	    }
	    printf("%s\n",out);
	}
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
        curr_max=16;
        elems=new ITEM*[curr_max];
        num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
        delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
        if(num_of_elems<=1) return;
        int i=node,k,c1,c2;
        ITEM* temp;
        
        do{
                k=i;
                c1=c2=2*i+1;
                c2++;
                if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
                        i=c1;
                if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
                        i=c2;
                if(k!=i)
                {
                        temp=elems[i];
                        elems[i]=elems[k];
                        elems[k]=temp;
			elems[k]->pos=k;
			elems[i]->pos=i;
                }
        }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
        int i=node,k,p;
        ITEM* temp;
        
        do{
                k=i;
		if( (p=(i+1)/2) != 0)
                {
                        --p;
                        if(elems[i]->time < elems[p]->time)
                        {
                                i=p;
                                temp=elems[i];
                                elems[i]=elems[k];
                                elems[k]=temp;
				elems[k]->pos=k;
				elems[i]->pos=i;
                        }
                }
        }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
        if(num_of_elems>=curr_max)
        {
                curr_max*=2;
                ITEM** buffer=new ITEM*[curr_max];
                for(int i=0;i<num_of_elems;i++)
                        buffer[i]=elems[i];
                delete[] elems;
                elems=buffer;
        }
        
        elems[num_of_elems]=item;
	elems[num_of_elems]->pos=num_of_elems;
	num_of_elems++;
        PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
        if(num_of_elems<=0)return NULL;
        
        ITEM* item=elems[0];
	num_of_elems--;
        elems[0]=elems[num_of_elems];
	elems[0]->pos=0;
        SiftDown(0);
        return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
    int i=item->pos;

    num_of_elems--;
    elems[i]=elems[num_of_elems];
    elems[i]->pos=i;
    SiftDown(i);
    PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
public:
        CalendarQueue();
        const char* GetName();
        ~CalendarQueue();
        void enqueue(ITEM*);
        ITEM* dequeue();
	void EnQueue(ITEM*);
	ITEM* DeQueue();
	ITEM* NextEvent() const { return m_head;}
        void Delete(ITEM*);
private:
        long last_bucket,number_of_buckets;
        double bucket_width;
        
        void ReSize(long);
        double NewWidth();

        ITEM ** buckets;
        long total_number;
        double bucket_top;
        long bottom_threshold;
        long top_threshold;
        double last_priority;
        bool resizable;

	ITEM* m_head;
	char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
    sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	    bucket_width,number_of_buckets);
    return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
        long i;
        
        number_of_buckets=16;
        bucket_width=1.0;
        bucket_top=bucket_width;
        total_number=0;
        last_bucket=0;
        last_priority=0.0;
        top_threshold=number_of_buckets*2;
        bottom_threshold=number_of_buckets/2-2;
        resizable=true;
        
        buckets= new ITEM*[number_of_buckets];
        for(i=0;i<number_of_buckets;i++)
                buckets[i]=NULL;
	m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
        delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
        long i;
        ITEM** old_buckets=buckets;
        long old_number=number_of_buckets;
        
        resizable=false;
        bucket_width=NewWidth();
        buckets= new ITEM*[newsize];
        number_of_buckets=newsize;
        for(i=0;i<newsize;i++)
                buckets[i]=NULL;
        last_bucket=0;
        total_number=0;

	
        
        ITEM *item;
        for(i=0;i<old_number;i++)
        {
                while(old_buckets[i]!=NULL)
                {
                        item=old_buckets[i];
                        old_buckets[i]=item->next;
                        enqueue(item);
                }
        }
        resizable=true;
        delete[] old_buckets;
        number_of_buckets=newsize;
        top_threshold=number_of_buckets*2;
        bottom_threshold=number_of_buckets/2-2;
        bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
        last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
    ITEM* head=m_head;
    m_head=dequeue();
    return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
        long i;
        for(i=last_bucket;;)
        {
                if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
                {
                        ITEM * item=buckets[i];
                        buckets[i]=buckets[i]->next;
                        total_number--;
                        last_bucket=i;
                        last_priority=item->time;
                        
                        if(resizable&&total_number<bottom_threshold)
                                ReSize(number_of_buckets/2);
                        item->next=NULL;
                        return item;
                }
                else
                {
                        i++;
                        if(i==number_of_buckets)i=0;
                        bucket_top+=bucket_width;
                        if(i==last_bucket)
                                break;
                }
        }
        
        
        long smallest;
        for(smallest=0;smallest<number_of_buckets;smallest++)
                if(buckets[smallest]!=NULL)break;

        if(smallest >= number_of_buckets)
	{
	    last_priority=bucket_top;
	    return NULL;
	}

        for(i=smallest+1;i<number_of_buckets;i++)
        {
                if(buckets[i]==NULL)
                        continue;
                else
                        if(buckets[i]->time<buckets[smallest]->time)
                                smallest=i;
        }
        ITEM * item=buckets[smallest];
        buckets[smallest]=buckets[smallest]->next;
        total_number--;
        last_bucket=smallest;
        last_priority=item->time;
        bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
        item->next=NULL;
        return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
    
    if(m_head==NULL)
    {
	m_head=item;
	return;
    }
    if(m_head->time>item->time)
    {
	enqueue(m_head);
	m_head=item;
    }
    else
	enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
    long i;
    if(item->time<last_priority)
    {
        i=(long)(item->time/bucket_width);
        last_priority=item->time;
        bucket_top=bucket_width*(i+1)+bucket_width*0.5;
        i=i%number_of_buckets;
	last_bucket=i;
    }
    else
    {
        i=(long)(item->time/bucket_width);
        i=i%number_of_buckets;
    }

        
        

        if(buckets[i]==NULL||item->time<buckets[i]->time)
        {
                item->next=buckets[i];
                buckets[i]=item;
        }
        else
        {

	    ITEM* pos=buckets[i];
                while(pos->next!=NULL&&item->time>pos->next->time)
                {
                        pos=pos->next;
                }
                item->next=pos->next;
                pos->next=item;
        }
        total_number++;
        if(resizable&&total_number>top_threshold)
                ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
    if(item==m_head)
    {
	m_head=dequeue();
	return;
    }
    long j;
    j=(long)(item->time/bucket_width);
    j=j%number_of_buckets;
        
    

    
    

    ITEM** p = &buckets[j];
    
    ITEM* i=buckets[j];
    
    while(i!=NULL)
    {
        if(i==item)
	{ 
            (*p)=item->next;
            total_number--;
            if(resizable&&total_number<bottom_threshold)
                ReSize(number_of_buckets/2);
            return;
        }
        p=&(i->next);
        i=i->next;
    }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
        long i, nsamples;
        
        if(total_number<2) return 1.0;
        if(total_number<=5)
                nsamples=total_number;
        else
                nsamples=5+total_number/10;
        if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
        long _last_bucket=last_bucket;
        double _bucket_top=bucket_top;
        double _last_priority=last_priority;
        
        double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
        ITEM* list,*next,*item;
        
        list=dequeue(); 
        long real_samples=0;
        while(real_samples<nsamples)
        {
                item=dequeue();
                if(item==NULL)
                {
                        item=list;
                        while(item!=NULL)
                        {
                                next=item->next;
                                enqueue(item);
                                item=next;      
                        }

                        last_bucket=_last_bucket;
                        bucket_top=_bucket_top;
                        last_priority=_last_priority;

                        
                        return 1.0;
                }
                AVG[real_samples]=item->time-list->time;
                avg1+=AVG[real_samples];
                if(AVG[real_samples]!=0.0)
                        real_samples++;
                item->next=list;
                list=item;
        }
        item=list;
        while(item!=NULL)
        {
                next=item->next;
                enqueue(item);
                item=next;      
        }
        
        last_bucket=_last_bucket;
        bucket_top=_bucket_top;
        last_priority=_last_priority;
        
        avg1=avg1/(double)(real_samples-1);
        avg1=avg1*2.0;
        
        
        long count=0;
        for(i=0;i<real_samples-1;i++)
        {
                if(AVG[i]<avg1&&AVG[i]!=0)
                {
                        avg2+=AVG[i];
                        count++;
                }
        }
        if(count==0||avg2==0)   return 1.0;
        
        avg2 /= (double) count;
        avg2 *= 3.0;
        
        return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 14 "../../common/cost.h"


#line 1 "../../common/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 15 "../../common/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
    double time;
    CostEvent* next;
    union {
        CostEvent* prev;
        int pos;  
    };
    TimerBase* object;
    int index;
    unsigned char active;
};



class TimerBase
{
public:
    virtual void activate(CostEvent*) = 0;
    virtual ~TimerBase() {}
};

class TypeII;



class CostSimEng
{
public:

	class seed_t
	{
	public:
		void operator = (long seed) { srand48(seed); };
	};
	seed_t Seed;

	CostSimEng()
	{
        if(m_instance==NULL)
            m_instance= this;
        else
        	printf("Error: only one simulation engine can be created\n");
	}
	virtual ~CostSimEng() { }
    static CostSimEng* Instance()
    {
        if(m_instance==NULL)
        {
        	printf("Error: a simulation engine has not been initialized\n");
        	m_instance = new CostSimEng;
        }
        return m_instance;
    }
    CorsaAllocator* GetAllocator(unsigned int datasize)
	{
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
			if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
	}
    void AddComponent(TypeII*c)
    {
        m_components.push_back(c);
    }
    void ScheduleEvent(CostEvent*e)
    {
        assert(e->time>=m_clock);
        
        m_queue.EnQueue(e);
    }
    void CancelEvent(CostEvent*e)
    {
        
        m_queue.Delete(e);
    }
    double Random(double v=1.0) { return v*drand48();}
    int Random(int v) { return (int)(v*drand48()); }
    double Exponential(double mean) { return -mean*log(Random());}

	virtual void Start() {};
	virtual void Stop() {};
    void Run();
    double SimTime() { return m_clock; } 
    double StopTime;
    
	double EventRate;
    double RunningTime;
    long EventsProcessed;

 private:
    double m_clock;
    queue_t<CostEvent> m_queue;
    std::vector<TypeII*> m_components;
    static CostSimEng* m_instance;
    std::vector<CorsaAllocator*> m_allocators;

};




class TypeII
{
public: 
    virtual void Start() {};
    virtual void Stop() {};
    virtual ~TypeII() {}
    TypeII()
    {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
    }

    #ifdef COST_DEBUG
    void Print(const bool, const char*, ...);
    #endif
    
    double Random(double v=1.0) { return v*drand48();}
    int Random(int v) { return (int)(v*drand48());}
    double Exponential(double mean) { return -mean*log(Random());}
    inline double SimTime() const { return m_simeng->SimTime(); }
    inline double StopTime() const { return m_simeng->StopTime; }
private:
    CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
    if(flag==false) return;
    va_list ap;
    va_start(ap, format);
    printf("[%f] ",SimTime());
    vprintf(format,ap);
    va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
    m_clock=0.0;
    EventsProcessed=0l;
    std::vector<TypeII*>::iterator iter;
      
    struct timeval start_time;    
    gettimeofday(&start_time,NULL);

    Start();
    
    for(iter=m_components.begin();iter!=m_components.end();iter++)
	    (*iter)->Start();

    CostEvent* e=m_queue.DeQueue();
    while(e!=NULL&&e->time<StopTime)
    {
	
        assert(e->time>=m_clock);
        m_clock=e->time;
        e->object->activate(e);
	    EventsProcessed++;
        e=m_queue.DeQueue();
    }

    m_clock=StopTime;
    for(iter=m_components.begin();iter!=m_components.end();iter++)
        (*iter)->Stop();
	    
    Stop();

    struct timeval stop_time;    
    gettimeofday(&stop_time,NULL);

    RunningTime = stop_time.tv_sec-start_time.tv_sec                          
		+ (stop_time.tv_usec-start_time.tv_usec)/1000000.0;
    EventRate = EventsProcessed/RunningTime;

    printf("---------------------------------------------------------------------------\n");	
    printf("CostSimEng with %s, stopped at %f\n ", m_queue.GetName(), StopTime);	
    printf("%ld events processed in %.3f seconds, event processing rate: %.0f\n",	
             EventsProcessed,RunningTime,EventRate);
}







#line 46 "../../common/sense.h"


#line 1 "../../common/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 47 "../../common/sense.h"


































class ether_addr_t
{
 public:
    enum { LENGTH = 6, BROADCAST = -1 };

    ether_addr_t () : addr (0) {} ;
    ether_addr_t (int a) : addr (a) {};

    bool operator == (const ether_addr_t& another) const { return addr==another.addr; }
    bool operator == (const int& another) const { return addr==another; }
    bool operator < (const ether_addr_t& another) const { return addr<another.addr; }
    bool operator > (const ether_addr_t& another) const { return addr>another.addr; }

    operator const int& () const { return addr; };
    struct compare 
    {
	    bool operator() ( const ether_addr_t & e1, const ether_addr_t & e2)
	    {
	        return e1.addr > e2.addr;
	    }
    };
 private:
    int addr;
};













template <class T>
class packet_trait
{
 public:
    typedef T nonpointer_t;
    static void free(const T&) { }
    static void free(T&t) { t.~T(); }
    static void init(T&t) { new (&t) T; }
    static void init(const T&) { }
    static void inc_ref(const T&) { }
    static void dump(std::string& str, const T&p) { p.dump(str) ;}  
    static void check_ref(const T&, int) { }
};




























template <class H, class P>
class smart_packet_t
{
 public:
    inline static smart_packet_t<H,P> * alloc();
    inline void free();
    inline void destroy();
    inline void inc_ref();
    inline smart_packet_t<H,P> * copy();
    inline void inc_pld_ref() const;

    inline bool check_ref(int r) { return r<=refcount; }

    static P& access_pld(smart_packet_t<H,P>* p) { return p->pld; }
    static P& access_pld(smart_packet_t<H,P>& p) { return p.pld; }

    static H& access_hdr(smart_packet_t<H,P>* p) { return p->hdr; }
    static H& access_hdr(smart_packet_t<H,P>& p) { return p.hdr; }

    H* operator ->() { return &hdr; }

    std::string dump() const;

    H hdr;
    P pld;

 private:

    static CorsaAllocator m_allocator;
    int refcount;
};







template <class H, class P> CorsaAllocator smart_packet_t<H,P>::m_allocator (sizeof(smart_packet_t<H,P>));






template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::alloc()
{
    smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
    packet_trait<H>::init(p->hdr);
    packet_trait<P>::init(p->pld);
    p->refcount=1;
    return p;
}





template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::copy()
{
    smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
    packet_trait<H>::init(p->hdr);
    packet_trait<P>::init(p->pld);
    p->hdr=hdr;
    p->pld=pld;
    inc_pld_ref();
    p->refcount=1;
    return p;
}







template <class H, class P>
void smart_packet_t<H,P>::free()
{
#ifdef SENSE_DEBUG
    packet_trait<P>::check_ref(pld,refcount);
#endif
    packet_trait<P>::free(pld);
    refcount--;
    if(refcount==0)
    {
	    packet_trait<H>::free(hdr);
	    m_allocator.free(this);
    }
}





template <class H, class P>
void smart_packet_t<H,P>::destroy()
{
    packet_trait<H>::free(hdr);
    packet_trait<P>::free(pld);
    m_allocator.free(this);
}






template <class H, class P>
void smart_packet_t<H,P>::inc_ref()
{
    packet_trait<P>::inc_ref(pld);
    refcount++;
    return;
}








template <class H, class P>
void smart_packet_t<H,P>::inc_pld_ref() const
{
    packet_trait<P>::inc_ref(pld);
}







template <class H, class P>
std::string smart_packet_t<H,P>::dump() const
{
    std::string str="[";
    std::string h;
    if( hdr.dump(h) )
    {
	    std::string m;
	    packet_trait<P>::dump(m,pld);
	    str = str + h + ", " + m + "]";
    }
    else
    {
	    str = str + h + ", NULL]";
    }
    return str;
}












template <class H, class P>
class packet_trait< smart_packet_t<H,P>* >
{
 public:
    typedef smart_packet_t<H,P> nonpointer_t;
    static void free(nonpointer_t* const &p) { if(p!=NULL) p->free(); };
    static void inc_ref(nonpointer_t* const &p) { if(p!=NULL) p->inc_ref(); };
    static void init(nonpointer_t* &p) { p=NULL; }
    static void dump(std::string& str,nonpointer_t* const &p)
	{ if(p!=NULL) str= p->dump(); else str="NULL"; }
    static void check_ref(nonpointer_t* const &p, int ref)
	{
	    if(p!=NULL&&!p->check_ref(ref))
		printf("Payload refcount is smaller than the current refcount\n");
	}

};







class coordinate_t
{
 public:
    coordinate_t () : x(0.0), y(0.0) { }
    coordinate_t ( double _x, double _y ) : x(_x), y(_y) { }
    double x,y;
};







class coordinate3d_t
{
 public:
    coordinate3d_t () : x(0.0), y(0.0), z(0.0) { }
    coordinate3d_t ( double _x, double _y, double _z ) : x(_x), y(_y), z(_z) { }
    double x,y,z;
};









#include <utility>
using std::make_pair;
using std::pair;









template <class T1, class T2, class T3>
class triple
{
 public:
    typedef T1 first_type;
    typedef T2 second_type;
    typedef T3 third_type;

    T1 first;
    T2 second;
    T3 third;

    triple() : first(T1()), second(T2()), third(T3()) {}
    triple(const T1& a, const T2& b, const T3& c) : first(a), second(b), third(c) {}

    template <class U1, class U2, class U3>
	triple(const triple<U1, U2, U3>& t) : first(t.first), second(t.second), third(t.third) {}
};

template <class T1, class T2, class T3>
inline bool operator == (const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y)
{
    return x.first == y.first && x.second == y.second && x.third == y.third;
}

template <class T1, class T2, class T3>
inline triple<T1, T2, T3> make_triple(const T1& x, const T2& y, const T3& z)
{
  return triple<T1, T2, T3>(x, y, z);
}













































#endif /* SENSE_H */


#line 54 "sim_routing.cc"








#line 1 "../../app/cbr.h"


































#ifndef cbr_h
#define cbr_h
#include <vector>











struct CBR_Struct
{






	struct packet_t
	{
		ether_addr_t src_addr;
		ether_addr_t dst_addr;
		int data_size;
		simtime_t send_time;








		std::string dump() const 
		{
		    char buffer[50];
		    sprintf(buffer,"%d %d %d",(int)src_addr,(int)dst_addr,data_size);
	    	return buffer;
		}
		void dump(std::string& str) const { str=dump();}
	};








	
    typedef triple<ether_addr_t,int,double> connection_t;
    std::vector<connection_t> Connections;
};













#line 268 "../../app/cbr.h"
#endif /* cbr_h*/

#line 61 "sim_routing.cc"


#line 1 "../../mob/immobile.h"























#ifndef immobile_h
#define immobile_h


#line 53 "../../mob/immobile.h"
#endif /* immobile_h*/

#line 62 "sim_routing.cc"


#line 1 "../../net/flooding.h"


































#ifndef flooding_h
#define flooding_h
#include <map>















template <class PLD>
struct Flooding_Struct
{







    struct hdr_struct
    {
		unsigned int seq_number;
		unsigned int size;
		double send_time;









		
		bool dump(std::string& str) const 
		{ 
	    	char buffer[30];
	    	sprintf(buffer,"%d %d",seq_number,size); 
	    	str=buffer;
	    	return true;
		}
    };










	typedef PLD payload_t;
    typedef smart_packet_t<hdr_struct,PLD> packet_t;

};














#line 402 "../../net/flooding.h"
#endif /*flooding_h*/

#line 63 "sim_routing.cc"


#line 1 "../../net/aodvi.h"



























#include <map>
#include <list>

#ifndef AODVI_H
#define AODVI_H

template <class PLD>
struct AODVI_Struct 
{
 public:

    enum { RREQ=0,RREP,RERR,RACK,HELLO,DATA,RTABLE};
    enum { JOIN=0, REPAIR=1, GRATUITOUS=2, DESTINATION=3, UNKNOWN=4};
    enum { RREQ_SIZE=24, RREP_SIZE=20, RERR_SIZE=5, RACK_SIZE=2, DATA_SIZE=12};
    
    
    struct flag_struct
    {
	    unsigned int join:1;
	    unsigned int repair:1;
	    unsigned int gratuitous:1;
	    unsigned int destination:1;
	    unsigned int unknown:1;
	    unsigned int acknowledgement:1;
	    unsigned int nodelete:1;
    };
  
    struct rreq_tuple_t
    {
    	int rreq_id;
	    simtime_t rreq_time;
    };
  
    struct route_entry_t
    {
    	int dst_seq_num;
    	bool valid_dst_seq;
	    int hop_count;
	    ether_addr_t next_hop;
	    double lifetime;
    };
  
    struct hdr_struct
    {
	    int type;
    	int TTL;
        flag_struct flags;
	    int hop_count;
	    int rreq_id;
	    ether_addr_t dst_addr;
	    int dst_seq_num;
	    ether_addr_t src_addr;
	    ether_addr_t prev_hop_addr;
	    int src_seq_num;
	    double lifetime;
	    int prefix_size;
	    unsigned int size;

	    bool dump(std::string& str) const ;
    };

    typedef smart_packet_t<hdr_struct, PLD> packet_t;
    typedef PLD payload_t;

    static const int TimeToLive;
    static const simtime_t PathDiscoveryTime;
    static const simtime_t NetTraversalTime;
    static const simtime_t NodeTraversalTime;
    static const simtime_t ActiveRouteTimeout;
    static const simtime_t MyRouteTimeout;

};

template <class PLD> const int AODVI_Struct<PLD>::TimeToLive = 10;
template <class PLD> const simtime_t AODVI_Struct<PLD>::ActiveRouteTimeout = 3;
template <class PLD> const simtime_t AODVI_Struct<PLD>::NodeTraversalTime = 40;
template <class PLD> const simtime_t AODVI_Struct<PLD>::NetTraversalTime = 2 * NodeTraversalTime * 35;
template <class PLD> const simtime_t AODVI_Struct<PLD>::PathDiscoveryTime = 2 * NetTraversalTime;
template <class PLD> const simtime_t AODVI_Struct<PLD>::MyRouteTimeout = 2 * ActiveRouteTimeout; 



#line 562 "../../net/aodvi.h"
template <class PLD>
bool AODVI_Struct<PLD>::hdr_struct::dump(std::string& str) const 
{ 
    char buffer[100];
    std::string t;
    switch(type)
    {
    case RREQ:
        t="REQUEST";
	break;    
    case RREP:
        t="REPLY";
	break;
    case DATA:
        t="DATA";
	break;
    case HELLO:
        t="HELLO";
	break;
    }
    sprintf(buffer,"%s %d %d %d %d->%d->%d (%d %d) ",
	    t.c_str(),TTL,hop_count, rreq_id, (int)prev_hop_addr,
	    (int)src_addr,(int)dst_addr,src_seq_num,dst_seq_num);
    str=buffer;
    return type==DATA;
}


#endif /* AODVI_H */

#line 64 "sim_routing.cc"


#line 1 "../../net/dsri.h"
























#ifndef dsri_h
#define dsri_h

#include <map>
#include <list>

using std::list;
using std::map;

#ifndef DSRI_TIMETOLIVE
#define DSRI_TIMETOLIVE 10
#endif

template <class PLD>
struct DSRI_Struct
{
 public:
    enum { RREQ=0,RREP=1,DATA=2};
    enum { RREQ_SIZE=68, RREP_SIZE=64, DATA_SIZE=64}; 

    struct path_t
    {
    	ether_addr_t addrs[DSRI_TIMETOLIVE+1];
    	int get_hops() const 
	    {
	        return hops;
	    }
	    int get_size() const
	    {
	        return hops*ether_addr_t::LENGTH+1;
	    }
	    ether_addr_t get_addr(int pos) const 
	    {
    	    assert(hops>pos);
	        return addrs[pos];
	    }
	    void clear()
	    {
	        hops=0;
	    }
	    void append(const ether_addr_t&);
	    void copy(const path_t&);
	    void shift_copy(const path_t&);  
	    int  truncate_copy(const path_t&, const ether_addr_t&);
	    bool contain(const ether_addr_t&);  
     private:
	    int hops;   
    };

    struct hdr_struct
    {
    	int type;
    	int TTL;
    	ether_addr_t dst_addr;
	    ether_addr_t src_addr;
	    path_t srcrt;    
	    int rreq_id;
	    unsigned int size;
	    bool dump (std::string& str) const;
    };

    typedef smart_packet_t<hdr_struct,PLD> packet_t;
    typedef PLD payload_t;
};



#line 373 "../../net/dsri.h"
template <class PLD>
bool DSRI_Struct<PLD>::hdr_struct::dump(std::string& str) const
{
    char buffer[100];
    std::string t;
    switch(type)
    {
        case RREQ:
	    t="REQUEST";
	    break;
    case RREP:
	    t="REPLY";
	    break;
    case DATA:
	    t="DATA";
	    break;
    }
    sprintf(buffer,"%s %d %d %d",t.c_str(),TTL,(int)src_addr,(int)dst_addr);
    str=buffer;
    return type==DATA;
}        

template <class PLD>
void DSRI_Struct<PLD>::path_t::append(const ether_addr_t& addr)
{
    assert(hops<DSRI_TIMETOLIVE+1);
    addrs[hops]=addr;
    hops++;
} 

template <class PLD>
void DSRI_Struct<PLD>::path_t::copy(const path_t& path) 
{ 
    for(int i=0;i<path.hops;i++)
	addrs[i]=path.addrs[i];
    hops=path.hops;
}

template <class PLD>
void DSRI_Struct<PLD>::path_t::shift_copy(const path_t& path)
{
    assert(path.hops>0);
    for(int i=0;i<path.hops-1;i++)
	addrs[i]=path.addrs[i+1];
    hops=path.hops-1;
}    	

template <class PLD>
int DSRI_Struct<PLD>::path_t::truncate_copy(const path_t& path, const ether_addr_t& addr)
{
    int i=0;
    while(i<path.hops)
    {
        if (addr==path.addrs[i])
	{
	    for(int j=i+1;j<path.hops;j++)
		addrs[j-i-1]=path.addrs[j];
	    hops=path.hops-i;
	    return i;
	}
	i++;
    }
    return -1;
}

template <class PLD> 
bool DSRI_Struct<PLD>::path_t::contain(const ether_addr_t& addr)
{
    for(int i=0;i<hops;i++)
	if(addr==addrs[i])
	    return true;
    return false;
}      	    	

#endif /*dsri_h*/

#line 65 "sim_routing.cc"


#line 1 "../../mac/null_mac.h"



































#ifndef simple_mac_h
#define simple_mac_h















template <class PLD>
struct NullMAC_Struct
{
    



    
	struct hdr_t
	{
		ether_addr_t dst_addr;
		ether_addr_t src_addr;
		unsigned int size;

		double tx_time;       
		double wave_length;   










		bool dump(std::string& str) const 
		{ 
	    	char buffer[50];
	    	sprintf(buffer,"%d %d %d",int(src_addr),int(dst_addr),size); 
	    	str=buffer;
	    	return true;
		}
	};










	typedef PLD payload_t;
	typedef smart_packet_t<hdr_t, PLD> packet_t;

};












#line 261 "../../mac/null_mac.h"
#endif /* simple_mac_h */

#line 66 "sim_routing.cc"


#line 1 "../../mac/mac_80211.h"























#ifndef mac80211_h
#define mac80211_h
#include <map>

template <class PLD>
struct MAC80211_Struct
{
    
    enum 
    {
		MAC_ProtocolVersion=0x00,
		MAC_Type_Management=0x00,
		MAC_Type_Control=0x01,
		MAC_Type_Data=0x02,
		MAC_Type_Reserved=0x03,   
		MAC_Subtype_RTS=0x0b,
		MAC_Subtype_CTS=0x0c,
		MAC_Subtype_ACK=0x0d,
		MAC_Subtype_Data=0x00
    };

    
    struct frame_control 
    {
        u_char          fc_subtype              : 4;
        u_char          fc_type                 : 2;
        u_char          fc_protocol_version     : 2;

        u_char          fc_order                : 1;
        u_char          fc_wep                  : 1;
        u_char          fc_more_data            : 1;
        u_char          fc_pwr_mgt              : 1;
        u_char          fc_retry                : 1;
        u_char          fc_more_frag            : 1;
        u_char          fc_from_ds              : 1;
        u_char          fc_to_ds                : 1;
    }; 

    struct hdr_struct
    {
		
        frame_control    	dh_fc;
        uint16_t            dh_duration;
        ether_addr_t        dh_da;
        ether_addr_t        dh_sa;
        ether_addr_t        dh_bssid;
        uint16_t            dh_scontrol;

		
		
		double tx_time;                              
		double wave_length;                          
		unsigned int size;

        bool dump(std::string& str) const
        {
            std::string type;
            bool payload=false;
            switch(dh_fc.fc_type)
            {
            case MAC_Type_Control:
	            switch(dh_fc.fc_subtype)
	            {
	            case MAC_Subtype_RTS:
	                type="RTS";
	                break;
	            case MAC_Subtype_CTS:
	                type="CST";
	                break;
	            case MAC_Subtype_ACK:
	                type="ACK";
	                break;
	            default:
	                type="UNKNOWN";
	            }
	            break;
            case MAC_Type_Data:
	            if(dh_fc.fc_subtype == MAC_Subtype_Data)
	            {
	                payload=true;
	                if(dh_da!=ether_addr_t::BROADCAST)
		                type="DATA";
	                else
		                type="BROADCAST";
	            }
	            else
	                type="UNKNOWN";
	            break;
            default:
	            type="UNKNOWN";
	            break;
            }
            char buffer[100];
            sprintf(buffer,"%f %s %d -> %d", tx_time, type.c_str(), (int)(dh_sa), (int)(dh_da));
            str=buffer;
            return payload;
        }        
    };	

    typedef smart_packet_t<hdr_struct,PLD> packet_t;
    typedef PLD payload_t;
    
    
    static int CWMin;
    static int CWMax;
    static double SlotTime;
    static double SIFSTime;
    static int PreambleLength;
    static int PLCPHeaderLength;

    
    static unsigned int RTSThreshold;
    static unsigned int ShortRetryLimit;
    static unsigned int LongRetryLimit;


    
    static int PLCP_HDR_LEN;
    static int HDR_LEN;
    static int RTS_LEN;
    static int CTS_LEN;
    static int ACK_LEN;
    static int FCS_LEN;
    
    
    
    static double DataRate;
    static double BasicRate;
    static double CPThreshold;    
};


template <class PLD> int MAC80211_Struct<PLD>::CWMin=31;
template <class PLD> int MAC80211_Struct<PLD>::CWMax=1023;
template <class PLD> double MAC80211_Struct<PLD>::SlotTime=0.000020;
template <class PLD> double MAC80211_Struct<PLD>::SIFSTime=0.000010;
template <class PLD> int MAC80211_Struct<PLD>::PreambleLength=144;
template <class PLD> int MAC80211_Struct<PLD>::PLCPHeaderLength=48;

template <class PLD> unsigned int MAC80211_Struct<PLD>::RTSThreshold=3000;
template <class PLD> unsigned int MAC80211_Struct<PLD>::ShortRetryLimit=7;
template <class PLD> unsigned int MAC80211_Struct<PLD>::LongRetryLimit=4;

template <class PLD> int MAC80211_Struct<PLD>::PLCP_HDR_LEN = (PreambleLength + PLCPHeaderLength ) >> 3;
template <class PLD> int MAC80211_Struct<PLD>::HDR_LEN = PLCP_HDR_LEN + FCS_LEN + 4*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::RTS_LEN = PLCP_HDR_LEN + FCS_LEN + 2*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::CTS_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::ACK_LEN = PLCP_HDR_LEN + FCS_LEN + 1*ether_addr_t::LENGTH + 6;
template <class PLD> int MAC80211_Struct<PLD>::FCS_LEN = 4;

template <class PLD> double MAC80211_Struct<PLD>::DataRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::BasicRate=1.0e6/8;
template <class PLD> double MAC80211_Struct<PLD>::CPThreshold=10; 










#line 991 "../../mac/mac_80211.h"
#endif /* mac80211_h */

#line 67 "sim_routing.cc"


#line 1 "../../phy/transceiver.h"























#ifndef transceiver_h
#define transceiver_h









#line 124 "../../phy/transceiver.h"
#endif /* transceiver_h */



#line 68 "sim_routing.cc"


#line 1 "../../phy/simple_channel.h"
























#ifndef simple_channel_h
#define simple_channel_h


#line 119 "../../phy/simple_channel.h"
#endif /* wireless_channel_h */

#line 69 "sim_routing.cc"


#line 1 "../../energy/battery.h"























#ifndef battery_h
#define battery_h


#line 206 "../../energy/battery.h"
#endif /* battery_h */


#line 70 "sim_routing.cc"


#line 1 "../../energy/power.h"
#ifndef power_h
#define power_h

#include <utility>

struct PM_Struct
{
    enum {TX=0,RX,IDLE,SLEEP,OFF};
};


#line 82 "../../energy/power.h"
#endif /* power_h */

#line 71 "sim_routing.cc"


#line 1 "../../util/fifo_ack.h"























#include <deque>


#line 72 "sim_routing.cc"





































typedef CBR_Struct::packet_t app_packet_t;
typedef Flooding_Struct<app_packet_t>::packet_t net_packet_t;
typedef MAC80211_Struct<net_packet_t*>::packet_t mac_packet_t;














#include "compcxx_sim_routing.h"
class compcxx_SimpleChannel_24;
#line 388 "../../common/cost.h"
/*template <class T> */
#line 388 "../../common/cost.h"
class compcxx_InfiTimer_14 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { triple<mac_packet_t *, double, int>  data; };
    compcxx_InfiTimer_14();
	virtual ~compcxx_InfiTimer_14();
	
	/*outport void to_component(triple<mac_packet_t *, double, int>  &, unsigned int i)*/;
	
    void activate(CostEvent*event);

    inline unsigned int Set(double t);
    inline unsigned int Set( triple<mac_packet_t *, double, int>  const & data, double t);
    inline void Cancel (unsigned int index);
    inline event_t* GetEvent(unsigned int index);

    inline bool Active(unsigned int index) { return GetEvent(index)->active; }
    inline double GetTime(unsigned int index) { return GetEvent(index)->time; }
    inline triple<mac_packet_t *, double, int>  & GetData(unsigned int index) { return GetEvent(index)->data; }
    inline void SetData(triple<mac_packet_t *, double, int>  const &d, unsigned int index) { GetEvent(index)->data = d; }

 private:
    inline void ReleaseSlot(unsigned int i) { m_free_slots.push_back(i); }
    inline unsigned int GetSlot();
    
    std::vector<event_t*> m_events;
    std::vector<unsigned int> m_free_slots;
    CostSimEng* m_simeng;

    CorsaAllocator* m_allocator;
public:compcxx_SimpleChannel_24* p_compcxx_parent;};

/*template <class PACKET>

*/
#line 28 "../../phy/simple_channel.h"
class compcxx_SimpleChannel_24 : public compcxx_component, public TypeII
{
 public:
    enum { FreeSpace, TwoRay };

    int NumNodes;
    bool DumpPackets;
    double CSThresh;
    double RXThresh;
    int PropagationModel;

	/*inport */void from_phy ( mac_packet_t  * p, double power, int id);
	class my_SimpleChannel_to_phy_f_t:public compcxx_functor<SimpleChannel_to_phy_f_t>{ public:void  operator() (mac_packet_t  * p, double power ) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p,power ); return (c[0]->*f[0])(p,power );};};compcxx_array<my_SimpleChannel_to_phy_f_t > to_phy;/*outportvoid to_phy (mac_packet_t  * p, double power )*/;
	/*inport */void pos_in ( const coordinate_t& pos, int id);
	
	compcxx_InfiTimer_14/*< triple<mac_packet_t  *, double, int> > */propagation_delay;
	/*inport */void depart( const triple<mac_packet_t  *, double, int> & data, unsigned int );

    compcxx_SimpleChannel_24() { propagation_delay.p_compcxx_parent=this /*connect propagation_delay.to_component, */; }
    virtual ~compcxx_SimpleChannel_24() {}
    void Start();
    void Stop();
    void Setup();

 private:    

    std::vector<coordinate_t > m_positions;
};

class compcxx_CBR_15;
#line 290 "../../common/cost.h"
/*template <class T> */
#line 290 "../../common/cost.h"
class compcxx_MultiTimer_2 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t  data; };
    compcxx_MultiTimer_2();
	virtual ~compcxx_MultiTimer_2();
	
	/*outport void to_component(trigger_t  &, unsigned int i)*/;
	
    inline void Set(double t, unsigned int index =0);
    inline void Set( trigger_t  const & data, double t,unsigned int index=0);
    void Cancel (unsigned int index=0);
    void activate(CostEvent*event);

    inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
    inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
    inline trigger_t  & GetData(unsigned int index=0) { return GetEvent(index)->data; }
    inline void SetData(trigger_t  const &d, unsigned int index) { GetEvent(index)->data = d; }

    event_t* GetEvent(unsigned int index);
    
 private:
    std::vector<event_t*> m_events;
    CostSimEng* m_simeng;
public:compcxx_CBR_15* p_compcxx_parent;};

class compcxx_Flooding_16;
#line 103 "../../app/cbr.h"
class compcxx_CBR_15 : public compcxx_component, public TypeII, public CBR_Struct
{
public:













	/*outport void to_transport (packet_t & packet, ether_addr_t & dst, unsigned int size)*/;
	/*inport */inline void from_transport ( packet_t & pkt);










    compcxx_MultiTimer_2 /*< trigger_t > */connection_timer;





    /*inport */inline void create(trigger_t& t, int i);
    
    











    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    double FinishTime;






    long SentPackets, RecvPackets;
    double TotalDelay;







    compcxx_CBR_15();
    virtual ~compcxx_CBR_15();
    void Start();
    void Stop();
public:compcxx_Flooding_16* p_compcxx_Flooding_16;};






#line 27 "../../energy/battery.h"
class compcxx_SimpleBattery_19 : public compcxx_component, public TypeII
{
 public:
    double InitialEnergy;
    double RemainingEnergy;


    /*inport */inline double power_in ( double power, simtime_t t);
    /*inport */inline double query_in ( );
    
    void Start();
    void Stop();
 private:
    double m_last_time;
};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_10 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { MAC80211_Struct<net_packet_t*>::packet_t* data; };
    

    compcxx_Timer_10() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(MAC80211_Struct<net_packet_t*>::packet_t* const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline MAC80211_Struct<net_packet_t*>::packet_t* & GetData() { return m_event.data; }
    inline void SetData(MAC80211_Struct<net_packet_t*>::packet_t* const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(MAC80211_Struct<net_packet_t*>::packet_t* &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_9 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { MAC80211_Struct<net_packet_t*>::packet_t* data; };
    

    compcxx_Timer_9() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(MAC80211_Struct<net_packet_t*>::packet_t* const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline MAC80211_Struct<net_packet_t*>::packet_t* & GetData() { return m_event.data; }
    inline void SetData(MAC80211_Struct<net_packet_t*>::packet_t* const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(MAC80211_Struct<net_packet_t*>::packet_t* &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_7 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_7() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(trigger_t const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline trigger_t & GetData() { return m_event.data; }
    inline void SetData(trigger_t const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(trigger_t &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_11 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_11() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(trigger_t const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline trigger_t & GetData() { return m_event.data; }
    inline void SetData(trigger_t const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(trigger_t &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_12 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { bool data; };
    

    compcxx_Timer_12() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(bool const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline bool & GetData() { return m_event.data; }
    inline void SetData(bool const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(bool &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_13 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { MAC80211_Struct<net_packet_t*>::packet_t* data; };
    

    compcxx_Timer_13() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(MAC80211_Struct<net_packet_t*>::packet_t* const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline MAC80211_Struct<net_packet_t*>::packet_t* & GetData() { return m_event.data; }
    inline void SetData(MAC80211_Struct<net_packet_t*>::packet_t* const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(MAC80211_Struct<net_packet_t*>::packet_t* &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_MAC80211_17;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_8 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_8() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(trigger_t const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline trigger_t & GetData() { return m_event.data; }
    inline void SetData(trigger_t const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(trigger_t &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_MAC80211_17* p_compcxx_parent;};

class compcxx_FIFOACK3_22;class compcxx_Flooding_16;class compcxx_DuplexTransceiver_18;/*template <class PLD>
*/
#line 186 "../../mac/mac_80211.h"
class compcxx_MAC80211_17 : public compcxx_component, public TypeII, public MAC80211_Struct<net_packet_t* >
{
 public:

    int SentPackets,RecvPackets;

    enum MAC_STATE {
        MAC_IDLE        = 0x0000,   
        MAC_DEFER       = 0x0001,   
        MAC_CTS         = 0x0002,   
        MAC_ACK         = 0x0004,   
    };

    ether_addr_t MyEtherAddr;
    bool DumpPackets;
    bool Promiscuity;

 	/*inport */void from_network ( const payload_t& pld, const ether_addr_t& dst, unsigned int size );
 	/*outport void to_network_data ( payload_t& pld, const ether_addr_t & dst)*/;
 	/*outport void to_network_ack ( bool errflag )*/;
 	
 	/*inport */void from_phy ( packet_t* pkt, bool errflag, double  power );
 	/*outport void to_phy (packet_t* pkt)*/;
    
    compcxx_Timer_7 /*<trigger_t> */defer_timer;                   
    compcxx_Timer_8 /*<trigger_t> */recv_timer;                    
    compcxx_Timer_9 /*<MAC80211_Struct<net_packet_t* >::packet_t*> */cts_timer;                     
    compcxx_Timer_10 /*<MAC80211_Struct<net_packet_t* >::packet_t*> */ack_timer;                     
    compcxx_Timer_11 /*<trigger_t> */nav_timer;                     
    
    compcxx_Timer_12 /*<bool> */network_ack_timer;
    /*inport */void NetworkAckTimer(bool&);
    compcxx_Timer_13 /*<MAC80211_Struct<net_packet_t* >::packet_t*> */phy_timer;
    /*inport */void PhyTimer(packet_t*&);
    
    void Start();                                    
    void Stop();                                     
    compcxx_MAC80211_17();
	virtual ~compcxx_MAC80211_17();

    
    
    /*inport */void DeferTimer(trigger_t&);
    /*inport */void RecvTimer(trigger_t&);
    /*inport */void CTSTimer(packet_t*&);
    /*inport */void ACKTimer(packet_t*&);
    /*inport */void NAVTimer(trigger_t&);

 private:

    
    void RecvRTS(packet_t* p);
    void RecvCTS(packet_t* p);
    void RecvData(packet_t* p);
    void RecvACK(packet_t* p);

    
    
    inline void TxPacket(packet_t* p, simtime_t tx_time);
    inline void TxPacket(packet_t* p, simtime_t start_time, simtime_t tx_time);

    
    
    
    
    
    void           StartDefer(bool backoff);
    inline void    ResumeDefer();

    
    void DropPacket(packet_t* p, const char*);
    
    uint16_t usec(simtime_t t) { return (uint16_t)floor((t *= 1e6) + 0.5); }

    MAC_STATE m_state;       
    double   m_nav;          
    int      m_cw;           
    double   m_backoff_time; 

    unsigned int m_ssrc;         
    unsigned int m_slrc;         
    double   m_sifs;         
    double   m_difs;         
    double   m_eifs;         

    double   m_ifs;          
    double   m_defer_start;  

    bool     m_in_session;   
    ether_addr_t m_session_peer; 

	struct {
		payload_t pld;
		ether_addr_t dst_addr;
		unsigned int size;
	} m_pldinfo;
	struct {
		packet_t * packet;
		bool error;
		double power;
	} m_rxinfo; 
	
    bool     m_tx_failed;    
    bool     m_long_pld;     
    simtime_t m_last_send;   
    uint16_t m_seq_no;      

    
    
    
    typedef std::map<ether_addr_t, uint16_t, ether_addr_t::compare> cache_t;
    cache_t m_recv_cache;
public:compcxx_FIFOACK3_22* p_compcxx_FIFOACK3_22;public:compcxx_Flooding_16* p_compcxx_Flooding_16;public:compcxx_DuplexTransceiver_18* p_compcxx_DuplexTransceiver_18;};

class compcxx_Immobile_21;/*template <class T> */
#line 228 "../../common/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
    inline void Set(trigger_t const &, double );
    inline void Set(double );
    inline double GetTime() { return m_event.time; }
    inline bool Active() { return m_event.active; }
    inline trigger_t & GetData() { return m_event.data; }
    inline void SetData(trigger_t const &d) { m_event.data = d; }
    void Cancel();
    /*outport void to_component(trigger_t &)*/;
    void activate(CostEvent*);
private:
    CostSimEng* m_simeng;
    event_t m_event;
public:compcxx_Immobile_21* p_compcxx_parent;};

class compcxx_SensorNode_23;
#line 27 "../../mob/immobile.h"
class compcxx_Immobile_21 : public compcxx_component, public TypeII
{
public:
	compcxx_Timer_3 /*<trigger_t> */timer;    
    /*outport */void pos_out (coordinate_t& pos, int id);
	/*inport */void announce_pos (trigger_t& t);

    double InitX, InitY;
    int ID;

	compcxx_Immobile_21 () { timer.p_compcxx_parent=this /*connect timer.to_component, */; }
    void Start();
    void Stop();
public:compcxx_SensorNode_23* p_compcxx_parent;};
class compcxx_Flooding_16;
#line 388 "../../common/cost.h"
/*template <class T> */
#line 388 "../../common/cost.h"
class compcxx_InfiTimer_4 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { Flooding_Struct<app_packet_t>::packet_t* data; };
    compcxx_InfiTimer_4();
	virtual ~compcxx_InfiTimer_4();
	
	/*outport void to_component(Flooding_Struct<app_packet_t>::packet_t* &, unsigned int i)*/;
	
    void activate(CostEvent*event);

    inline unsigned int Set(double t);
    inline unsigned int Set( Flooding_Struct<app_packet_t>::packet_t* const & data, double t);
    inline void Cancel (unsigned int index);
    inline event_t* GetEvent(unsigned int index);

    inline bool Active(unsigned int index) { return GetEvent(index)->active; }
    inline double GetTime(unsigned int index) { return GetEvent(index)->time; }
    inline Flooding_Struct<app_packet_t>::packet_t* & GetData(unsigned int index) { return GetEvent(index)->data; }
    inline void SetData(Flooding_Struct<app_packet_t>::packet_t* const &d, unsigned int index) { GetEvent(index)->data = d; }

 private:
    inline void ReleaseSlot(unsigned int i) { m_free_slots.push_back(i); }
    inline unsigned int GetSlot();
    
    std::vector<event_t*> m_events;
    std::vector<unsigned int> m_free_slots;
    CostSimEng* m_simeng;

    CorsaAllocator* m_allocator;
public:compcxx_Flooding_16* p_compcxx_parent;};

class compcxx_CBR_15;class compcxx_FIFOACK3_22;/*template <class PLD>
*/
#line 113 "../../net/flooding.h"
class compcxx_Flooding_16 : public compcxx_component, public TypeII, public Flooding_Struct<app_packet_t >
{
 public:












    ether_addr_t MyEtherAddr;
    simtime_t ForwardDelay;
    bool DumpPackets;





    int SentPackets;
    int RecvPackets;
    int RecvUniPackets;
    double TotalDelay;













    
    /*inport */inline void from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size);
    /*inport */inline void from_mac_data (packet_t* pkt, ether_addr_t& dst);
    /*inport */inline void from_mac_ack (bool errflag);






	/*outport void to_transport ( payload_t& pld )*/;
	/*outport void to_mac (packet_t* pkt, ether_addr_t& dst, unsigned int size)*/;













	compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t >::packet_t*> */delay;
	/*inport */inline void depart(packet_t* p, unsigned int i);
	





    void Start();
    void Stop();
    compcxx_Flooding_16();
    virtual ~compcxx_Flooding_16();
	
 protected:

    bool m_mac_busy;                  
    unsigned int m_seq_number;        


















    class seq_number_t
    {
    	public:
		seq_number_t () : current(0),bits(0u) {}
		bool check(int n)
		{
	    	if( n + (int)sizeof(unsigned long) <= current ) return true;
	    	if ( n > current )
	    	{
				bits = bits << (n - current);
				current = n;
	    	}

	    	unsigned long flag = 1 << (current - n);
	    	unsigned long r = flag & bits;
	    	bits |= flag;
	    	return r;
		}
    	private:
		int current;
		uint32_t bits;
    };
	






    typedef std::map<ether_addr_t, seq_number_t, ether_addr_t::compare> cache_t;
    cache_t m_seq_cache;

public:compcxx_CBR_15* p_compcxx_CBR_15;public:compcxx_FIFOACK3_22* p_compcxx_FIFOACK3_22;};

class compcxx_MAC80211_17;class compcxx_PowerManager_20;class compcxx_SensorNode_23;/*template <class PACKET>
*/
#line 34 "../../phy/transceiver.h"
class compcxx_DuplexTransceiver_18  : public compcxx_component, public TypeII
{
 public:
    
    
    
    
    
     
    double TXPower;
    double TXGain;
    double RXGain;
    double Frequency;

    double RXThresh;       
    double CSThresh;       
    int ID;
   

	/*inport */inline void from_mac (mac_packet_t  * p);    
	/*outport void to_mac (mac_packet_t  * p, bool errflag, unsigned int size)*/;
    
    /*inport */inline void from_channel (mac_packet_t  * p, double power);
    /*outport void to_channel (mac_packet_t  * p, double power, int id)*/;

	/*outport double to_power_switch ( int state, double time)*/;
    
    void Start();
    void Stop();

public:compcxx_MAC80211_17* p_compcxx_MAC80211_17;public:compcxx_PowerManager_20* p_compcxx_PowerManager_20;public:compcxx_SensorNode_23* p_compcxx_parent;};

class compcxx_SimpleBattery_19;
#line 11 "../../energy/power.h"
class compcxx_PowerManager_20 : public compcxx_component, public TypeII, public PM_Struct
{
public:

    double RXPower;
    double TXPower;
    double IdlePower;
    double SleepPower;

	/*inport */inline double switch_state(int state, double time);
	/*inport */inline int state_query();
	/*inport */inline double energy_query();
	/*outport double to_battery_power(double power, simtime_t time)*/;
	/*outport double to_battery_query()*/;
	
    void Start();
    void Stop();
   
 private:
    int m_state; 
public:compcxx_SimpleBattery_19* p_compcxx_SimpleBattery_19;};

class compcxx_MAC80211_17;class compcxx_Flooding_16;/*template < class T1, class T2, class T3 >
*/
#line 26 "../../util/fifo_ack.h"
class compcxx_FIFOACK3_22 : public compcxx_component, public TypeII
{
public:

    compcxx_FIFOACK3_22() {};
    virtual ~compcxx_FIFOACK3_22() {};

    struct data_t 
    {
        data_t ( net_packet_t* const& _t1, ether_addr_t const& _t2, unsigned int const& _t3): t1(_t1), t2(_t2), t3(_t3) {};
        net_packet_t* t1; ether_addr_t t2; unsigned int t3; 
    };

    void Start();

    /*inport */void in (net_packet_t* const& t1, ether_addr_t const& t2, unsigned int const& t3);
    /*inport */void next (const bool&);
    /*outport void out (net_packet_t* t1, ether_addr_t t2, unsigned int t3)*/;
    /*outport void ack (bool)*/;

private:
    bool m_busy; 
    std::deque <data_t> m_queue; 
public:compcxx_MAC80211_17* p_compcxx_MAC80211_17;public:compcxx_Flooding_16* p_compcxx_Flooding_16;};







#line 128 "sim_routing.cc"
class compcxx_SensorNode_23 : public compcxx_component, public TypeII
{
public:

    compcxx_CBR_15 app;
    compcxx_Flooding_16 /*<app_packet_t> */net;
    compcxx_MAC80211_17 /*<net_packet_t*> */mac;
    
    
    compcxx_DuplexTransceiver_18 /*< mac_packet_t > */phy;
    
    compcxx_SimpleBattery_19 battery;
    
    compcxx_PowerManager_20 pm;
    
    compcxx_Immobile_21 mob;
    
    compcxx_FIFOACK3_22/*<net_packet_t*,ether_addr_t,unsigned int> */queue;
    
    double MaxX, MaxY;  
    ether_addr_t MyEtherAddr; 
    int ID; 

    virtual ~compcxx_SensorNode_23();
    void Start();
    void Stop();
    void Setup();






	class my_SensorNode_to_channel_packet_f_t:public compcxx_functor<SensorNode_to_channel_packet_f_t>{ public:void  operator() (mac_packet_t* packet, double power, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet,power,id); return (c[0]->*f[0])(packet,power,id);};};my_SensorNode_to_channel_packet_f_t to_channel_packet_f;/*outport */void to_channel_packet(mac_packet_t* packet, double power, int id);
	/*inport */void from_channel (mac_packet_t* packet, double power);
	class my_SensorNode_to_channel_pos_f_t:public compcxx_functor<SensorNode_to_channel_pos_f_t>{ public:void  operator() (coordinate_t& pos, int id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(pos,id); return (c[0]->*f[0])(pos,id);};};my_SensorNode_to_channel_pos_f_t to_channel_pos_f;/*outport */void to_channel_pos(coordinate_t& pos, int id);
};


#line 324 "sim_routing.cc"
class compcxx_RoutingSim_25 : public compcxx_component, public CostSimEng
{
public:
    void Start();
    void Stop();






    double MaxX, MaxY;
    int NumNodes;
    int NumSourceNodes;
    int NumConnections;
    int PacketSize;
    double Interval;






    compcxx_array<compcxx_SensorNode_23 >nodes;
    compcxx_SimpleChannel_24 /*< mac_packet_t > */channel;

    void Setup();
};


#line 421 "../../common/cost.h"
compcxx_InfiTimer_14
#line 420 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_InfiTimer_14()
{
	m_simeng = CostSimEng::Instance();
	m_allocator = m_simeng->GetAllocator(sizeof(event_t));
	GetEvent(0);
}


#line 429 "../../common/cost.h"
compcxx_InfiTimer_14
#line 428 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_InfiTimer_14()
{
	for(unsigned int i=0;i<m_events.size();i++)
		m_allocator->free(m_events[i]);
}


#line 435 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_InfiTimer_14/*<T>*/::event_t* compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::GetEvent(unsigned int index)
{
   	if (index>=m_events.size())
   	{
        for (unsigned int i=m_events.size();i<=index;i++)
   	    {
       	    m_events.push_back( (event_t*) m_allocator->alloc() );
           	m_events[i]->active=false;
           	m_events[i]->index=i;
           	m_free_slots.push_back(i);
       	}
   	}
   	return m_events[index];
}


#line 451 "../../common/cost.h"

#line 451 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::Set(triple<mac_packet_t *, double, int>  const & data, double time)
{
	int index=GetSlot();
    event_t * e = GetEvent(index);
    assert(e->active==false);
    e->time = time;
    e->data = data;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
    return index;
}


#line 465 "../../common/cost.h"

#line 465 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::Set(double time)
{
	int index=GetSlot();
    event_t * e = GetEvent(index);
    assert(e->active==false);
    e->time = time;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
    return index;
}


#line 478 "../../common/cost.h"

#line 478 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::Cancel(unsigned int index)
{
    event_t * e = GetEvent(index);
    assert(e->active);
   	m_simeng->CancelEvent(e);
   	ReleaseSlot(index);
    e->active = false;
}


#line 488 "../../common/cost.h"

#line 488 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::activate(CostEvent*e)
{
	event_t * event = (event_t*)e;
	event->active = false;
   	ReleaseSlot(event->index);
    (p_compcxx_parent->depart(event->data,event->index));
}


#line 497 "../../common/cost.h"

#line 497 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_14/*<triple<mac_packet_t *, double, int>  >*/::GetSlot()
{
    if(m_free_slots.empty())GetEvent(m_events.size()*2-1);
    int i=m_free_slots.back();
    m_free_slots.pop_back();
    return i;
}



#line 46 "../../phy/simple_channel.h"

#line 59 "../../phy/simple_channel.h"

#line 59 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::Setup()
{
    m_positions.reserve(NumNodes);
    for(int i=0;i<NumNodes;i++)
		m_positions.push_back( coordinate_t (0.0, 0.0) );
	to_phy.SetSize(NumNodes);
}

#line 67 "../../phy/simple_channel.h"

#line 67 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::Start()
{
}

#line 71 "../../phy/simple_channel.h"

#line 71 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::Stop()
{
}


#line 76 "../../phy/simple_channel.h"

#line 76 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::pos_in(const coordinate_t & pos, int id)
{
    m_positions[id]=pos;
}


#line 82 "../../phy/simple_channel.h"

#line 82 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::from_phy ( mac_packet_t  * p, double power, int in)
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


#line 113 "../../phy/simple_channel.h"

#line 113 "../../phy/simple_channel.h"
/*template <class PACKET>
*/void compcxx_SimpleChannel_24/*<mac_packet_t  >*/::depart ( const triple<mac_packet_t  *, double, int> & data , unsigned int index)
{
   	to_phy[data.third](data.first,data.second);
}


#line 317 "../../common/cost.h"
compcxx_MultiTimer_2
#line 316 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_MultiTimer_2()
{
	m_simeng = CostSimEng::Instance(); 
	GetEvent(0);
}


#line 324 "../../common/cost.h"
compcxx_MultiTimer_2
#line 323 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_MultiTimer_2()
{
	for(unsigned int i=0;i<m_events.size();i++)
		delete m_events[i];
}


#line 330 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_MultiTimer_2/*<T>*/::event_t* compcxx_MultiTimer_2/*<trigger_t  >*/::GetEvent(unsigned int index)
{
   	if (index>=m_events.size())
   	{
        for (unsigned int i=m_events.size();i<=index;i++)
   	    {
       	    m_events.push_back(new event_t);
           	m_events[i]->active=false;
           	m_events[i]->index=i;
       	}
   	}
   	return m_events[index];
}


#line 345 "../../common/cost.h"

#line 345 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Set(trigger_t  const & data, double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->data = data;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 357 "../../common/cost.h"

#line 357 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Set(double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 368 "../../common/cost.h"

#line 368 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::Cancel(unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)
    	m_simeng->CancelEvent(e);
    e->active = false;
}


#line 377 "../../common/cost.h"

#line 377 "../../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_2/*<trigger_t  >*/::activate(CostEvent*e)
{
	event_t * event = (event_t*)e;
	event->active = false;
    (p_compcxx_parent->create(event->data,event->index));
}





#line 137 "../../app/cbr.h"

#line 179 "../../app/cbr.h"
compcxx_CBR_15::compcxx_CBR_15()
{
	connection_timer.p_compcxx_parent=this /*connect connection_timer.to_component, */;
}


#line 184 "../../app/cbr.h"
compcxx_CBR_15::~compcxx_CBR_15()
{
}








#line 194 "../../app/cbr.h"
void compcxx_CBR_15::Start()
{
    SentPackets=RecvPackets=0l;
    TotalDelay=0.0;
    for(unsigned int i=0;i<Connections.size();i++)
    {
		connection_timer.Set(Random(Connections[i].third),i);
    }
}






#line 208 "../../app/cbr.h"
void compcxx_CBR_15::Stop()
{
    

}






#line 218 "../../app/cbr.h"
void compcxx_CBR_15::from_transport(packet_t& p)
{









    Printf((DumpPackets,"receives %s\n",p.dump().c_str()));

    if(p.dst_addr==MyEtherAddr)
    {
		RecvPackets++;
		TotalDelay += SimTime() - p.send_time;
    }
    else
    {
		printf("cbr %d received a packet destined for %d\n",
	       (int)MyEtherAddr,(int)p.dst_addr);
    }
}










#line 251 "../../app/cbr.h"
void compcxx_CBR_15::create(trigger_t&, int index)
{
    if(SimTime()>FinishTime) return;

    packet_t p;
    p.data_size=Connections[index].second + 2*ether_addr_t::LENGTH;
    p.dst_addr=Connections[index].first;
    p.src_addr=MyEtherAddr;
    p.send_time=SimTime();

    Printf((DumpPackets,"sends %s\n",p.dump().c_str()));

    (p_compcxx_Flooding_16->from_transport(p,p.dst_addr,p.data_size));
    SentPackets++;
    connection_timer.Set(SimTime()+Connections[index].third,index);
}


#line 43 "../../energy/battery.h"
void compcxx_SimpleBattery_19::Start()
{
    RemainingEnergy=InitialEnergy;
    m_last_time=0.0;
}


#line 49 "../../energy/battery.h"
void compcxx_SimpleBattery_19::Stop()
{
}


#line 53 "../../energy/battery.h"
double compcxx_SimpleBattery_19::power_in(double power, simtime_t t)
{
	
	if(t>m_last_time)
	{
        RemainingEnergy -= (t-m_last_time)*power;
        m_last_time=t;
    }
    return RemainingEnergy;
}

#line 63 "../../energy/battery.h"
double compcxx_SimpleBattery_19::query_in()
{
    return RemainingEnergy;
}












































































































































#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_10/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(MAC80211_Struct<net_packet_t*>::packet_t* const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_10/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_10/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_10/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->ACKTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(MAC80211_Struct<net_packet_t*>::packet_t* const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->CTSTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->DeferTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_11/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_11/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_11/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_11/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->NAVTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<bool >*/::Set(bool const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<bool >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<bool >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<bool >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->NetworkAckTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(MAC80211_Struct<net_packet_t*>::packet_t* const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_13/*<MAC80211_Struct<net_packet_t*>::packet_t* >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->PhyTimer(m_event.data));
}




#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->RecvTimer(m_event.data));
}




#line 218 "../../mac/mac_80211.h"

#line 220 "../../mac/mac_80211.h"

#line 229 "../../mac/mac_80211.h"

#line 230 "../../mac/mac_80211.h"

#line 231 "../../mac/mac_80211.h"

#line 232 "../../mac/mac_80211.h"

#line 233 "../../mac/mac_80211.h"

#line 302 "../../mac/mac_80211.h"
compcxx_MAC80211_17
#line 301 "../../mac/mac_80211.h"
/*template <class PLD>
*//*<PLD>*/::compcxx_MAC80211_17()
{
	defer_timer.p_compcxx_parent=this /*connect defer_timer.to_component, */;
	recv_timer.p_compcxx_parent=this /*connect recv_timer.to_component, */;
	cts_timer.p_compcxx_parent=this /*connect cts_timer.to_component, */;
	ack_timer.p_compcxx_parent=this /*connect ack_timer.to_component, */;
	nav_timer.p_compcxx_parent=this /*connect nav_timer.to_component, */;
	
	network_ack_timer.p_compcxx_parent=this /*connect network_ack_timer.to_component, */;
	phy_timer.p_compcxx_parent=this /*connect phy_timer.to_component, */;
}


#line 315 "../../mac/mac_80211.h"
compcxx_MAC80211_17
#line 314 "../../mac/mac_80211.h"
/*template <class PLD>
*//*<PLD>*/::~compcxx_MAC80211_17()
{
	
}


#line 320 "../../mac/mac_80211.h"

#line 320 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::Start()
{
    
    m_state=MAC_IDLE;
    m_nav=0.0;
    m_cw=CWMin;

    m_last_send=0.0;

    m_ssrc=0;
    m_slrc=0;
    m_sifs=SIFSTime;
    m_difs=m_sifs+SlotTime;
    m_eifs=m_sifs+ACK_LEN/DataRate+m_difs;

    m_tx_failed=false;
    m_seq_no=0;

    SentPackets=RecvPackets=0;
}


#line 342 "../../mac/mac_80211.h"

#line 342 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::Stop()
{
    
}


#line 348 "../../mac/mac_80211.h"

#line 348 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::from_network ( const payload_t& pld, const ether_addr_t& dst_addr, unsigned int size )
{
    
    if(m_state!=MAC_IDLE)
    {
		Printf((DumpPackets,"mac%d state: %d, m_last_send: %f, simtime: %f\n",(int)MyEtherAddr,
			m_state,m_last_send,SimTime()));
		packet_trait<net_packet_t* >::free(pld);
		
		
		
		
		(p_compcxx_FIFOACK3_22->next(false));
		return;
    }
    if(size < RTSThreshold || dst_addr == ether_addr_t::BROADCAST )
    {
		m_long_pld=false;
		m_ssrc=0;
    }
    else
    {
		m_long_pld=true;
		m_slrc=0;
    }
    
    m_pldinfo.pld=pld;
    m_pldinfo.dst_addr=dst_addr;
    m_pldinfo.size=size;
    m_seq_no++;

    
    
    
    StartDefer(false);
}

#line 385 "../../mac/mac_80211.h"

#line 385 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::DeferTimer(trigger_t&)
{
    

    assert(m_state==MAC_DEFER);
    Printf((DumpPackets,"mac%d deferal timer fires \n", (int)MyEtherAddr));
    packet_t * np = packet_t::alloc();              
    if(m_long_pld==false)
    {
		
		np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
		np->hdr.dh_fc.fc_type             = MAC_Type_Data;
		np->hdr.dh_fc.fc_subtype          = MAC_Subtype_Data;
		np->hdr.dh_da                     = m_pldinfo.dst_addr;
		np->hdr.dh_sa                     = MyEtherAddr;
		np->hdr.dh_scontrol               = m_seq_no;

    	np->hdr.size = m_pldinfo.size + HDR_LEN;
		np->hdr.tx_time = np->hdr.size / DataRate;
		np->pld = m_pldinfo.pld;
		
	
		if( np->hdr.dh_da != ether_addr_t::BROADCAST)
		{
	    	
	    	
	    	
	    	
	    	
	    	np->inc_ref();
	    	np->hdr.dh_duration = usec (m_sifs+ACK_LEN/BasicRate + np->hdr.size/DataRate);
	    	
	    	
	    	
	    	ack_timer.Set(np, np->hdr.dh_duration*1e-6 + m_difs + SimTime() );
	    	m_state = MAC_ACK;          
		}
		else
		{
	    	
	    	np->hdr.tx_time = np->hdr.size /BasicRate;
	    	np->hdr.dh_duration = 0;
	    	
	    	assert(!network_ack_timer.Active());
	    	network_ack_timer.Set(true,SimTime()+np->hdr.tx_time);
	    	m_state = MAC_IDLE;
		}
    }
    else
    {
		
		np->inc_ref();
		np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
		np->hdr.dh_fc.fc_type             = MAC_Type_Control;
		np->hdr.dh_fc.fc_subtype          = MAC_Subtype_RTS;
		np->hdr.dh_da                     = m_pldinfo.dst_addr;
		np->hdr.dh_sa                     = MyEtherAddr;

		np->hdr.size                      = RTS_LEN;                     
		np->hdr.tx_time                   = np->hdr.size/BasicRate;
		np->hdr.dh_duration = usec ( 3*m_sifs + CTS_LEN/BasicRate + 
				     (m_pldinfo.size + HDR_LEN)/DataRate 
				     + ACK_LEN/BasicRate );
		
		cts_timer.Set(np,np->hdr.dh_duration*1e-6 + m_sifs + SimTime());
		m_state = MAC_CTS;
    }

    
    TxPacket(np,np->hdr.tx_time);
}


#line 458 "../../mac/mac_80211.h"

#line 458 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::from_phy ( packet_t* pkt, bool errflag, double  power )

{
    
    

    Printf((DumpPackets,"mac%d start receiving %s (%e,%d)\n",(int)MyEtherAddr,pkt->dump().c_str(),power,errflag));

    
    if( recv_timer.Active() )
    {
		packet_t* rx_packet = m_rxinfo.packet;
	
		if( power > m_rxinfo.power * CPThreshold)
		{
	    	DropPacket(m_rxinfo.packet,"power too weak");
	    	m_rxinfo.packet=pkt;
	    	m_rxinfo.error=errflag;
	    	m_rxinfo.power=power;
	    	recv_timer.Set(SimTime()+pkt->hdr.tx_time);
		}
		else if (m_rxinfo.power > power * CPThreshold)
		{
	    	DropPacket(pkt, "power too weak");
		}
		else
		{
	    	simtime_t end_time = SimTime() + pkt->hdr.tx_time;
	    	
	    	if( end_time > recv_timer.GetTime() )
	    	{
				recv_timer.Set(end_time);
				DropPacket(rx_packet, "receive-receive collision");
		    	m_rxinfo.packet=pkt;
		    	m_rxinfo.error=errflag;
	    		m_rxinfo.power=power;
	    	}
	    	else
	    	{
				DropPacket(pkt,"receive-receive collision");
	    	}
	    	m_rxinfo.error=1;   
		}
    }
    else
    {
    	m_rxinfo.packet=pkt;
    	m_rxinfo.error=errflag;
   		m_rxinfo.power=power;
		
		recv_timer.Set(pkt->hdr.tx_time + SimTime());
	    if(m_state==MAC_DEFER)
	    {
	        
	        
	        assert(defer_timer.Active());
	        
	        double pt=SimTime()-m_defer_start;
	        if(pt>m_ifs)
	        {
		        
		        
		        
		        m_backoff_time-= pt-m_ifs;
		        assert(m_backoff_time>=0);
    	    }
	        defer_timer.Cancel();
	        Printf((DumpPackets,"mac%d deferral suspended\n",(int)MyEtherAddr));
	    }
    }
}


#line 531 "../../mac/mac_80211.h"

#line 531 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::RecvTimer(trigger_t&)
{
    
    
    
    
    
    

    packet_t* p=m_rxinfo.packet;

    Printf((DumpPackets,"mac%d received %s (%e,%d)\n",(int)MyEtherAddr,p->dump().c_str(),m_rxinfo.power,m_rxinfo.error));

    if(m_rxinfo.error==1)  
    {  
	    DropPacket(p, "frame error");
	    m_tx_failed = true;
	    ResumeDefer();
	    return ;
    }
    simtime_t now = SimTime();
    
    if( now - p->hdr.tx_time < m_last_send )
    {
	    
	    DropPacket(p,"frame error");
	    m_tx_failed = true;
        ResumeDefer();
	    return;
    }

    ether_addr_t dst = p->hdr.dh_da;
    uint8_t type = p->hdr.dh_fc.fc_type;
    uint8_t subtype = p->hdr.dh_fc.fc_subtype;  

    
    simtime_t nav = p->hdr.dh_duration*1e-6 + SimTime();
    if(nav>m_nav) m_nav=nav;
    RecvPackets++;
    

    if( dst != MyEtherAddr && dst != ether_addr_t::BROADCAST)
    {
	    if( !Promiscuity || type!= MAC_Type_Data)
  	    { 
            DropPacket(p,"wrong destination");
	        m_tx_failed=false;
	        ResumeDefer();
            return;
	    }
    }

    m_tx_failed=false;  

    switch(type)
    {
    case MAC_Type_Management:
        DropPacket(p, "unknown MAC packet");
        break;
    case MAC_Type_Control:
        switch(subtype)
        {
        case MAC_Subtype_RTS:
            RecvRTS(p);
            ResumeDefer();
            break;
        case MAC_Subtype_CTS:
            RecvCTS(p);
            ResumeDefer();
            break;
        case MAC_Subtype_ACK:
            ResumeDefer();
            RecvACK(p);
	    break;
        default:
            fprintf(stderr, "recv_timer(1):Invalid MAC Control Subtype %x\n",
                    subtype);
            break;
        }
        break;
    case MAC_Type_Data:
        switch(subtype)
        {
        case MAC_Subtype_Data:
            RecvData(p);
	    break;
        default:
            fprintf(stderr, "recv_timer(2):Invalid MAC Data Subtype %x\n",
                    subtype);
            break;
        }
        break;
    default:
        fprintf(stderr, "recv_timer(3):Invalid MAC Type %x\n", subtype);
        break;
    }
}


#line 630 "../../mac/mac_80211.h"

#line 630 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::RecvRTS(packet_t* p)
{
    if( m_state != MAC_IDLE && m_state != MAC_DEFER )
    {
	    
	    
	    
	    
	    DropPacket(p,"mac is busy");
	    return;
    }

    
    
    
    if( !nav_timer.Active() )
    {
	    
	    packet_t* np = packet_t::alloc();

	    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
	    np->hdr.dh_fc.fc_type       = MAC_Type_Control;
	    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_CTS;

	    np->hdr.size=CTS_LEN;
	    np->hdr.dh_da = p->hdr.dh_sa ;
	    np->hdr.dh_sa=MyEtherAddr;
	    np->hdr.tx_time = np->hdr.size/BasicRate;
	    np->hdr.dh_duration = p->hdr.dh_duration - usec(m_sifs + RTS_LEN/BasicRate);

	    
	    TxPacket(np,m_sifs,np->hdr.tx_time);
	
	    
	    m_in_session=true;
	    m_session_peer=p->hdr.dh_sa;
	    assert(m_nav>=SimTime());
	    nav_timer.Set(m_nav);

	    
	    
	    p->free();  
    }
    else
    {
	    DropPacket(p,"unexpected RTS");
	    return;
    }
}


#line 681 "../../mac/mac_80211.h"

#line 681 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::RecvCTS(packet_t* p)
{
    if(m_state != MAC_CTS)
    {
	    
	    
	    DropPacket(p, "CTS without RTS");
	    return;
    }
    
    
    
    packet_t* np = packet_t::alloc();

    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
    np->hdr.dh_fc.fc_type       = MAC_Type_Data;
    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_Data;
    np->hdr.dh_da=p->hdr.dh_sa;;
    np->hdr.dh_sa=MyEtherAddr;
    np->hdr.dh_scontrol=m_seq_no;
	
    np->hdr.size=m_pldinfo.size+HDR_LEN;
    np->pld=m_pldinfo.pld;
    np->hdr.tx_time = np->hdr.size/DataRate;
    np->hdr.dh_duration = usec(2*m_sifs+ACK_LEN/BasicRate + np->hdr.size/DataRate);
    m_state = MAC_ACK;
    ack_timer.Set(np, np->hdr.dh_duration*1e-6 + m_difs + SimTime());

    np->inc_ref();  
    TxPacket(np,m_sifs,np->hdr.tx_time);

    
    p->free();
    
    cts_timer.GetData()->free();
    cts_timer.Cancel();  
                         

    
    
    
    
    
    
    

}


#line 730 "../../mac/mac_80211.h"

#line 730 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::RecvData(packet_t* p)
{
    if( m_state != MAC_IDLE && m_state != MAC_DEFER )
    {
	    
	    DropPacket(p,"mac is busy");
	    return;
    }

    if(p->hdr.dh_da==MyEtherAddr )
    {
	    
	    
	    
	    
	    
	    if( nav_timer.Active() && !(m_in_session&&p->hdr.dh_sa==m_session_peer) )
	    {
            ResumeDefer();
	        DropPacket(p,"unexpected DATA");
	        return;
	    }
	    
	    packet_t* np = packet_t::alloc();

	    np->hdr.dh_fc.fc_protocol_version = MAC_ProtocolVersion;
	    np->hdr.dh_fc.fc_type       = MAC_Type_Control;
	    np->hdr.dh_fc.fc_subtype    = MAC_Subtype_ACK;

	    np->hdr.dh_da=p->hdr.dh_sa;
	    np->hdr.dh_sa=MyEtherAddr;

	    np->hdr.size = ACK_LEN;
	    np->hdr.tx_time = np->hdr.size /BasicRate;
	    np->hdr.dh_duration = 0;

	    TxPacket(np,m_sifs,np->hdr.tx_time);
    }

    ResumeDefer();

    
    
    cache_t::iterator src=m_recv_cache.find(p->hdr.dh_sa);
    if(src!=m_recv_cache.end())
    {
	    
	    
	    if(src->second==p->hdr.dh_scontrol)
	    {
	        
	        
	        
	        DropPacket(p,"duplicated DATA packet");
	        return;
	    }
	    else
	    {
	        
	        src->second=p->hdr.dh_scontrol;
	    }
    }
    else
    {
	    
	    m_recv_cache.insert(make_pair(p->hdr.dh_sa,p->hdr.dh_scontrol));
    }

    
    
    
    p->inc_pld_ref();
    
    (p_compcxx_Flooding_16->from_mac_data(p->pld,p->hdr.dh_da));
    p->free(); 
}

#line 807 "../../mac/mac_80211.h"

#line 807 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::RecvACK(packet_t* p)
{
    if(m_state != MAC_ACK)
    {
	    DropPacket(p, "ACK without DATA");
	    return;
    }
    p->free();                   
    ack_timer.GetData()->free(); 
    ack_timer.Cancel(); 
    m_state=MAC_IDLE;            
    m_cw=CWMin;                  
    (p_compcxx_FIFOACK3_22->next(true));  
                                 
}

#line 823 "../../mac/mac_80211.h"

#line 823 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::CTSTimer(packet_t*& p)
{
    
    
    
    p->free();                   
    assert(m_state==MAC_CTS);

    
    m_ssrc++;
    if(m_ssrc< ShortRetryLimit ) 
    {
	    if(m_cw<CWMax)m_cw=2*m_cw+1;   
	    StartDefer(true);              
    }
    else
    {
	    m_state=MAC_IDLE;              
	    m_cw=CWMin;                    
	    (p_compcxx_FIFOACK3_22->next(false));   
    }
}

#line 846 "../../mac/mac_80211.h"

#line 846 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::ACKTimer(packet_t*& p)
{
    assert(m_state==MAC_ACK);
    bool retry=false;
    if(m_long_pld)
    { 
	    m_slrc++;
	    if(m_slrc < LongRetryLimit)
	        retry=true;
    }
    else
    {
	    m_ssrc++;
	    if(m_ssrc < ShortRetryLimit)
	        retry=true;
    }

    if(retry)                          
    {
	    if(m_cw<CWMax)m_cw=2*m_cw+1;
	    StartDefer(true);
	    p->inc_pld_ref();
    }
    else
    {
	    m_state=MAC_IDLE;
	    m_cw=CWMin;
      	(p_compcxx_FIFOACK3_22->next(false));
    } 
    p->free();
}


#line 879 "../../mac/mac_80211.h"

#line 879 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::NAVTimer(trigger_t&)
{
    
    m_in_session=false;
}


#line 886 "../../mac/mac_80211.h"

#line 886 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::StartDefer(bool backoff)
{
    
    assert(m_state!=MAC_DEFER);
    assert(defer_timer.Active()==false);
    m_backoff_time = Random( m_cw * SlotTime);  

    Printf((DumpPackets, "mac%d deferring (%d)\n",(int)MyEtherAddr,m_state));
    
    
    
    if(!recv_timer.Active())                    
    {
	    
	    if(m_tx_failed) m_ifs=m_eifs;
	    else m_ifs=m_difs;
	
	    double pt = m_ifs;
	    if(backoff) pt+=m_backoff_time; 
	    double now=SimTime();
	    if(now<m_last_send)now=m_last_send;
	    if(m_nav>now)
	        m_defer_start=m_nav;
	    else
	        m_defer_start=now;
	    defer_timer.Set(m_defer_start+pt);
    }
    m_state=MAC_DEFER;
}



#line 918 "../../mac/mac_80211.h"

#line 918 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::ResumeDefer()
{
    
    if(m_state!=MAC_DEFER)return;

    
    if(defer_timer.Active())printf("Mac%d\n",(int)MyEtherAddr);
    assert(defer_timer.Active()==false);
    if(m_tx_failed) m_ifs=m_eifs;
    else m_ifs=m_difs;
    double now=SimTime();
    if(now<m_last_send)now=m_last_send;
    if(m_nav>now)
	    m_defer_start=m_nav;
    else
	    m_defer_start=now;
    defer_timer.Set(m_defer_start+m_ifs+m_backoff_time);  
    Printf((DumpPackets, "mac%d resumes deferring\n", (int)MyEtherAddr));
}


#line 939 "../../mac/mac_80211.h"

#line 939 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::TxPacket(packet_t* p, simtime_t tx_time)
{
    
    double now=SimTime();
    Printf((DumpPackets,"mac%d sends %s until %f\n",(int)MyEtherAddr, p->dump().c_str(),now+tx_time));
    if(now<m_last_send)
    {
	    printf("two sends overlap\n");
	    return;
    }
    m_last_send=now+tx_time;
    (p_compcxx_DuplexTransceiver_18->from_mac(p));
    SentPackets++;
}

#line 954 "../../mac/mac_80211.h"

#line 954 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::TxPacket(packet_t* p, simtime_t start_time, simtime_t tx_time)
{
    
    double start=SimTime()+start_time;
    Printf((DumpPackets,"mac%d sends %s from %f to %f\n",(int)MyEtherAddr,p->dump().c_str(),start,start+tx_time));
    if(start<m_last_send)
    {
	    printf("two sends overlap\n");
	    return;
    }
    m_last_send=start+tx_time;
    assert(!phy_timer.Active());
    phy_timer.Set(p,start);
    SentPackets++;
}


#line 971 "../../mac/mac_80211.h"

#line 971 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::NetworkAckTimer(bool& ack)
{
    (p_compcxx_FIFOACK3_22->next(ack));
}


#line 977 "../../mac/mac_80211.h"

#line 977 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::PhyTimer(packet_t* &p)
{
    (p_compcxx_DuplexTransceiver_18->from_mac(p));
}



#line 984 "../../mac/mac_80211.h"

#line 984 "../../mac/mac_80211.h"
/*template <class PLD>
*/void compcxx_MAC80211_17/*<net_packet_t* >*/::DropPacket(packet_t* p, const char* reason)
{
    Printf((DumpPackets,"mac%d drops %s (%s)\n",(int)MyEtherAddr,p->dump().c_str(),reason));
    p->free();
}


#line 249 "../../common/cost.h"

#line 249 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 261 "../../common/cost.h"

#line 261 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 272 "../../common/cost.h"

#line 272 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 280 "../../common/cost.h"

#line 280 "../../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->announce_pos(m_event.data));
}




#line 31 "../../mob/immobile.h"
void compcxx_Immobile_21::pos_out(coordinate_t& pos, int id){return (p_compcxx_parent->to_channel_pos(pos,id));}
#line 32 "../../mob/immobile.h"

#line 41 "../../mob/immobile.h"
void compcxx_Immobile_21::announce_pos(trigger_t&)
{
    coordinate_t pos=coordinate_t(InitX,InitY);
    (p_compcxx_parent->to_channel_pos(pos,ID));
}

#line 46 "../../mob/immobile.h"
void compcxx_Immobile_21::Start()
{
	timer.Set(0.0);
}

#line 50 "../../mob/immobile.h"
void compcxx_Immobile_21::Stop()
{
}

#line 421 "../../common/cost.h"
compcxx_InfiTimer_4
#line 420 "../../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_InfiTimer_4()
{
	m_simeng = CostSimEng::Instance();
	m_allocator = m_simeng->GetAllocator(sizeof(event_t));
	GetEvent(0);
}


#line 429 "../../common/cost.h"
compcxx_InfiTimer_4
#line 428 "../../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_InfiTimer_4()
{
	for(unsigned int i=0;i<m_events.size();i++)
		m_allocator->free(m_events[i]);
}


#line 435 "../../common/cost.h"
/*template <class T>
*//*typename */compcxx_InfiTimer_4/*<T>*/::event_t* compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::GetEvent(unsigned int index)
{
   	if (index>=m_events.size())
   	{
        for (unsigned int i=m_events.size();i<=index;i++)
   	    {
       	    m_events.push_back( (event_t*) m_allocator->alloc() );
           	m_events[i]->active=false;
           	m_events[i]->index=i;
           	m_free_slots.push_back(i);
       	}
   	}
   	return m_events[index];
}


#line 451 "../../common/cost.h"

#line 451 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::Set(Flooding_Struct<app_packet_t>::packet_t* const & data, double time)
{
	int index=GetSlot();
    event_t * e = GetEvent(index);
    assert(e->active==false);
    e->time = time;
    e->data = data;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
    return index;
}


#line 465 "../../common/cost.h"

#line 465 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::Set(double time)
{
	int index=GetSlot();
    event_t * e = GetEvent(index);
    assert(e->active==false);
    e->time = time;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
    return index;
}


#line 478 "../../common/cost.h"

#line 478 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::Cancel(unsigned int index)
{
    event_t * e = GetEvent(index);
    assert(e->active);
   	m_simeng->CancelEvent(e);
   	ReleaseSlot(index);
    e->active = false;
}


#line 488 "../../common/cost.h"

#line 488 "../../common/cost.h"
/*template <class T>
*/void compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::activate(CostEvent*e)
{
	event_t * event = (event_t*)e;
	event->active = false;
   	ReleaseSlot(event->index);
    (p_compcxx_parent->depart(event->data,event->index));
}


#line 497 "../../common/cost.h"

#line 497 "../../common/cost.h"
/*template <class T>
*/unsigned int compcxx_InfiTimer_4/*<Flooding_Struct<app_packet_t>::packet_t* >*/::GetSlot()
{
    if(m_free_slots.empty())GetEvent(m_events.size()*2-1);
    int i=m_free_slots.back();
    m_free_slots.pop_back();
    return i;
}



#line 180 "../../net/flooding.h"

#line 249 "../../net/flooding.h"
compcxx_Flooding_16
#line 248 "../../net/flooding.h"
/*template <class PLD>
*//*<PLD>*/::compcxx_Flooding_16()
{
	delay.p_compcxx_parent=this /*connect delay.to_component, */;
}

#line 254 "../../net/flooding.h"
compcxx_Flooding_16
#line 253 "../../net/flooding.h"
/*template <class PLD>
*//*<PLD>*/::~compcxx_Flooding_16()
{
}


#line 258 "../../net/flooding.h"

#line 258 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::Start() 
{
    m_mac_busy=false;
    m_seq_number=1u;
    m_seq_cache.insert(make_pair(MyEtherAddr,seq_number_t()));

    SentPackets=RecvPackets=RecvUniPackets=0l;
    TotalDelay=0.0;
}

#line 268 "../../net/flooding.h"

#line 268 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::Stop() 
{
    
    
}









#line 282 "../../net/flooding.h"

#line 282 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::from_transport( payload_t& pld, ether_addr_t& dst, unsigned int size)
{









    if(m_mac_busy)
    {
		packet_trait<app_packet_t >::free(pld);
		return;               
    }

    packet_t* p=packet_t::alloc();
    p->pld=pld;
    m_seq_number++;
    m_seq_cache[MyEtherAddr].check(m_seq_number);
    p->hdr.seq_number=m_seq_number;
    p->hdr.size=size+2*sizeof(unsigned int);
    m_mac_busy=true;
    SentPackets++;









    Printf((DumpPackets,"creates %s\n",p->dump().c_str()));

    p->hdr.send_time=SimTime();
    (p_compcxx_FIFOACK3_22->in(p,ether_addr_t::BROADCAST,size));
}






#line 327 "../../net/flooding.h"

#line 327 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::from_mac_data (packet_t* pkt, ether_addr_t& dst)
{
    ether_addr_t src;
    src=pkt->pld.src_addr;
    RecvPackets++;
    TotalDelay+=SimTime()-pkt->hdr.send_time;

    Printf((DumpPackets,"receives %s\n",pkt->dump().c_str()));
    cache_t::iterator iter=m_seq_cache.find(src);
    if(iter==m_seq_cache.end())
    {
	
	
		iter=(m_seq_cache.insert(make_pair(src,seq_number_t()))).first;
    }

    if( !iter->second.check(pkt->hdr.seq_number) )
    {
	
		RecvUniPackets++;
		if(pkt->pld.dst_addr==MyEtherAddr||pkt->pld.dst_addr==ether_addr_t::BROADCAST)
		{
	    	
	    	pkt->inc_pld_ref();
	    	(p_compcxx_CBR_15->from_transport(pkt->pld));
		}
		else  
		{
	    	
	    	if(m_mac_busy==false)
	    	{
				Printf((DumpPackets,"forwards %s\n",pkt->dump().c_str()));















				SentPackets++;
				pkt->hdr.send_time=SimTime();
				delay.Set(pkt,SimTime()+Random(ForwardDelay));
				return;
	    	}
		}
    }
    pkt->free();
}







#line 390 "../../net/flooding.h"

#line 390 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::from_mac_ack(bool)
{
    m_mac_busy=false;
}


#line 396 "../../net/flooding.h"

#line 396 "../../net/flooding.h"
/*template <class PLD>
*/void compcxx_Flooding_16/*<app_packet_t >*/::depart(packet_t* p, unsigned int i)
{
	m_mac_busy=true;
	(p_compcxx_FIFOACK3_22->in(p,ether_addr_t::BROADCAST,p->hdr.size));
}

#line 67 "../../phy/transceiver.h"

#line 67 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_18/*<mac_packet_t  >*/::Start()
{
}


#line 72 "../../phy/transceiver.h"

#line 72 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_18/*<mac_packet_t  >*/::Stop()
{
}


#line 77 "../../phy/transceiver.h"

#line 77 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_18/*<mac_packet_t  >*/::from_mac(mac_packet_t  * p)
{
	double now = SimTime();
	if((p_compcxx_PowerManager_20->switch_state(PM_Struct::TX,now))<0.0)  
    {
    	p->free();
		return;
    }	 
   
    if((p_compcxx_PowerManager_20->switch_state(PM_Struct::IDLE,now+p->hdr.tx_time))<0.0)
    {
    	p->free();
    	return;
    } 
    p->hdr.wave_length=speed_of_light/Frequency;
    (p_compcxx_parent->to_channel_packet(p,TXPower * TXGain, ID));
}


#line 96 "../../phy/transceiver.h"

#line 96 "../../phy/transceiver.h"
/*template <class PACKET>
*/void compcxx_DuplexTransceiver_18/*<mac_packet_t  >*/::from_channel(mac_packet_t  * p, double power)
{
	double now = SimTime();
    if((p_compcxx_PowerManager_20->switch_state(PM_Struct::RX,now))< 0.0)  
    {
       p->free();
       return;
    }
          
    if((p_compcxx_PowerManager_20->switch_state(PM_Struct::IDLE, now+p->hdr.tx_time))<0.0)
    {
		p->free();
		return;
    } 
  
    double recv_power = power * RXGain;
    if( recv_power < CSThresh )
    {
    	p->free(); 
    }
	else
	{
		bool error = ( recv_power < RXThresh ) ;
	  	(p_compcxx_MAC80211_17->from_phy( p, error, recv_power ));
    }
}


#line 33 "../../energy/power.h"
void compcxx_PowerManager_20::Start()
{
    m_state=OFF;
}


#line 38 "../../energy/power.h"
void compcxx_PowerManager_20::Stop()
{

}


#line 43 "../../energy/power.h"
double compcxx_PowerManager_20::switch_state(int state, double time) 
{
    double power=0.0;	
    switch(m_state)
    {
    case TX:
		power = (p_compcxx_SimpleBattery_19->power_in(TXPower,time));
		break;
    case RX:
		power = (p_compcxx_SimpleBattery_19->power_in(RXPower,time));
		break;
    case IDLE:
		power = (p_compcxx_SimpleBattery_19->power_in(IdlePower,time));
		break;
    case SLEEP:
		power = (p_compcxx_SimpleBattery_19->power_in(SleepPower,time));
		break;
    case OFF:
		power = (p_compcxx_SimpleBattery_19->power_in(0.0,time));
		break;
    default:
        fprintf(stderr,"invalid state to power manager\n");
        break;
    }
    m_state=state;
    return power;

} 
       

#line 72 "../../energy/power.h"
int compcxx_PowerManager_20::state_query()
{
    return m_state; 
}


#line 77 "../../energy/power.h"
double compcxx_PowerManager_20::energy_query()
{
    return (p_compcxx_SimpleBattery_19->query_in());
}


#line 57 "../../util/fifo_ack.h"

#line 57 "../../util/fifo_ack.h"
/*template < class T1, class T2, class T3 >
*/void compcxx_FIFOACK3_22 /*<net_packet_t* ,ether_addr_t ,unsigned int > */:: Start()
{
    m_busy=false;
}











#line 72 "../../util/fifo_ack.h"

#line 72 "../../util/fifo_ack.h"
/*template < class T1, class T2, class T3 >
*/void compcxx_FIFOACK3_22 /*<net_packet_t* ,ether_addr_t ,unsigned int >*/::in (net_packet_t* const& t1, ether_addr_t const& t2, unsigned int const& t3)
{
    if (!m_busy)
    { 
        m_busy=true;
        (p_compcxx_MAC80211_17->from_network(t1,t2,t3));
    } else
    {
        data_t d (t1,t2,t3);
        m_queue.push_back(d);
    }
    (p_compcxx_Flooding_16->from_mac_ack(true));
    return;
}









#line 95 "../../util/fifo_ack.h"

#line 95 "../../util/fifo_ack.h"
/*template <  class T1, class T2, class T3  >
*/void compcxx_FIFOACK3_22 /*<net_packet_t* ,ether_addr_t ,unsigned int > */:: next (const bool&)
{
    if (m_queue.size()>0)
    {
	    data_t data = m_queue.front();
        m_queue.pop_front();
        (p_compcxx_MAC80211_17->from_network(data.t1,data.t2,data.t3));
    } else
    {
        m_busy=false;
    }
    return;
}

#line 161 "sim_routing.cc"
void compcxx_SensorNode_23::to_channel_packet(mac_packet_t* packet, double power, int id){return (to_channel_packet_f(packet,power,id));}
#line 162 "sim_routing.cc"
void compcxx_SensorNode_23::from_channel(mac_packet_t* packet, double power){return (phy.from_channel(packet,power));}
#line 163 "sim_routing.cc"
void compcxx_SensorNode_23::to_channel_pos(coordinate_t& pos, int id){return (to_channel_pos_f(pos,id));}
#line 166 "sim_routing.cc"
compcxx_SensorNode_23::~compcxx_SensorNode_23()
{
}


#line 170 "sim_routing.cc"
void compcxx_SensorNode_23::Start()
{
}


#line 174 "sim_routing.cc"
void compcxx_SensorNode_23::Stop()
{
}





#line 181 "sim_routing.cc"
void compcxx_SensorNode_23::Setup()
{




    battery.InitialEnergy=1e6;
    









    
    app.MyEtherAddr=MyEtherAddr;
    app.FinishTime=StopTime()*0.9;
    app.DumpPackets=false;
    






    mob.InitX=Random(MaxX);
    mob.InitY=Random(MaxY);
    mob.ID=ID;










    net.MyEtherAddr=MyEtherAddr;
    net.ForwardDelay=0.1;
    net.DumpPackets=true;
    





 
    mac.MyEtherAddr=MyEtherAddr;
    mac.Promiscuity=true;
    mac.DumpPackets=true;
   






   	pm.TXPower=1.6;
   	pm.RXPower=1.2;
   	pm.IdlePower=1.15;











    phy.TXPower=0.0280;
    phy.TXGain=1.0;
    phy.RXGain=1.0; 
    phy.Frequency=9.14e8;
    phy.RXThresh=3.652e-10;
    phy.CSThresh=1.559e-11;
    phy.ID=ID;







    app.p_compcxx_Flooding_16=&net /*connect app.to_transport, net.from_transport*/;
    net.p_compcxx_CBR_15=&app /*connect net.to_transport, app.from_transport*/;
    
    net.p_compcxx_FIFOACK3_22=&queue /*connect net.to_mac, queue.in*/;
    queue.p_compcxx_MAC80211_17=&mac /*connect queue.out, mac.from_network*/;
    mac.p_compcxx_FIFOACK3_22=&queue /*connect mac.to_network_ack, queue.next*/;
    queue.p_compcxx_Flooding_16=&net /*connect queue.ack, net.from_mac_ack*/;
    mac.p_compcxx_Flooding_16=&net /*connect mac.to_network_data, net.from_mac_data */;
    






    
    
    
    
    mac.p_compcxx_DuplexTransceiver_18=&phy /*connect mac.to_phy, phy.from_mac*/;
    phy.p_compcxx_MAC80211_17=&mac /*connect phy.to_mac, mac.from_phy*/;

    phy.p_compcxx_PowerManager_20=&pm /*connect phy.to_power_switch, pm.switch_state*/;
    pm.p_compcxx_SimpleBattery_19=&battery /*connect pm.to_battery_query, battery.query_in*/;
    pm.p_compcxx_SimpleBattery_19=&battery /*connect pm.to_battery_power, battery.power_in*/;















    phy.p_compcxx_parent=this /*connect phy.to_channel, */;
    mob.p_compcxx_parent=this /*connect mob.pos_out, */;
     /*connect , phy.from_channel*/;
}











#line 353 "sim_routing.cc"
void compcxx_RoutingSim_25 :: Start()
{

}






#line 362 "sim_routing.cc"
void compcxx_RoutingSim_25 :: Stop()
{
    int i,sent,recv;
    double delay;
    for(sent=recv=i=0,delay=0.0;i<NumNodes;i++)
    {
		sent+=nodes[i].app.SentPackets;
		recv+=nodes[i].app.RecvPackets;
		delay+=nodes[i].app.TotalDelay;
    }
    printf("APP -- packets sent: %d, received: %d, success rate: %.3f, delay: %.3f\n",
		sent,recv,(double)recv/sent,delay/recv);
    for(sent=recv=i=0;i<NumNodes;i++)
    {
		sent+=nodes[i].net.SentPackets;
		recv+=nodes[i].net.RecvPackets;
    }
    printf("NET -- packets sent: %d, received: %d\n",sent,recv);
    for(sent=recv=i=0;i<NumNodes;i++)
    {
		sent+=nodes[i].mac.SentPackets;
		recv+=nodes[i].mac.RecvPackets;
    }
    printf("MAC -- packets sent: %d, received: %d\n",sent,recv);
}














#line 400 "sim_routing.cc"
void compcxx_RoutingSim_25 :: Setup()
{
    int i,j;
    




    
    nodes.SetSize(NumNodes);
    for(i=0;i<NumNodes;i++)
    {
		nodes[i].MaxX=MaxX;
		nodes[i].MaxY=MaxY;
		nodes[i].MyEtherAddr=i;
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
		nodes[i].to_channel_packet_f.Connect(channel,(compcxx_component::SensorNode_to_channel_packet_f_t)&compcxx_SimpleChannel_24::from_phy) /*connect nodes[i].to_channel_packet,channel.from_phy*/;
		nodes[i].to_channel_pos_f.Connect(channel,(compcxx_component::SensorNode_to_channel_pos_f_t)&compcxx_SimpleChannel_24::pos_in) /*connect nodes[i].to_channel_pos,channel.pos_in*/;
		channel.to_phy[i].Connect(nodes[i],(compcxx_component::SimpleChannel_to_phy_f_t)&compcxx_SensorNode_23::from_channel) /*connect channel.to_phy[i],nodes[i].from_channel */;
    }




    int src,dst;
    for(i=0;i<NumSourceNodes;i++)
    {
		for(j=0;j<NumConnections;j++)
		{
	    	do
	    	{
				src=Random(NumNodes);
				dst=Random(NumNodes);
	    	}while(src==dst); 
	    	nodes[src].app.Connections.push_back(
				make_triple(ether_addr_t(dst),Random(PacketSize)+PacketSize/2,
				Random(Interval)+Interval/2));
		}
    }
}






















#line 488 "sim_routing.cc"
int main(int argc, char* argv[])
{
    compcxx_RoutingSim_25 sim;

    sim.StopTime = 1000;
    sim.Seed = 1234;

    sim.MaxX = 2000;
    sim.MaxY = 2000;

    sim.NumNodes = 110;
    sim.NumConnections = 2;
    sim.PacketSize = 2000;
    sim.Interval = 100.0;
   
    if(argc >= 2) sim.StopTime = atof(argv[1]);
    if(argc >= 3) sim.NumNodes = atoi(argv[2]);
    sim.NumSourceNodes = sim.NumNodes / 10;
    if(argc >= 4) sim.MaxX = sim.MaxY = atof(argv[3]);
    if(argc >= 5) sim.NumSourceNodes = atoi(argv[4]);
    if(argc >= 6) sim.PacketSize = atoi(argv[5]);
    if(argc >= 7) sim.Interval = atof(argv[6]); 

    printf("StopTime: %.0f, Number of Nodes: %d, Terrain: %.0f by %.0f\n",
	   sim.StopTime, sim.NumNodes, sim.MaxX, sim.MaxY);
    printf("Number of Sources: %d, Packet Size: %d, Interval: %f\n",
	   sim.NumSourceNodes, sim.PacketSize, sim.Interval);

    sim.Setup();
    sim.Run();

    return 0;
}
