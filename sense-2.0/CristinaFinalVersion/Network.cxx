
#line 1 "../common/cost.h"
#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 "../common/priority_q.h"

























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

#line 13 "../common/cost.h"


#line 1 "../common/corsa_alloc.h"
































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

#line 14 "../common/cost.h"


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







#line 1 "Network.cc"



#line 1 "Station/Station.h"

#line 1 "Station/../FlowGen/FlowGen.h"

#line 1 "Station/../FlowGen/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Station/../FlowGen/FlowGen.h"



#line 36 "Station/../FlowGen/FlowGen.h"
;


#line 43 "Station/../FlowGen/FlowGen.h"
;


#line 46 "Station/../FlowGen/FlowGen.h"
;


#line 60 "Station/../FlowGen/FlowGen.h"
;


#line 81 "Station/../FlowGen/FlowGen.h"
;


#line 122 "Station/../FlowGen/FlowGen.h"
;

#line 166 "Station/../FlowGen/FlowGen.h"
;

#line 1 "Station/Station.h"


#line 1 "Station/../Source/Source.h"

#line 1 "Station/../Source/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Station/../Source/Source.h"



#line 35 "Station/../Source/Source.h"
;


#line 39 "Station/../Source/Source.h"
;


#line 42 "Station/../Source/Source.h"
;


#line 72 "Station/../Source/Source.h"
;


#line 85 "Station/../Source/Source.h"
;


#line 2 "Station/Station.h"


#line 1 "Station/../Mobility/Mobility.h"

#line 1 "Station/../Mobility/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Station/../Mobility/Mobility.h"



#line 28 "Station/../Mobility/Mobility.h"
;



#line 66 "Station/../Mobility/Mobility.h"
;

#line 3 "Station/Station.h"



#line 37 "Station/Station.h"
;



#line 3 "Network.cc"


#line 1 "Sink/Sink.h"

#line 1 "Sink/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Sink/Sink.h"



#line 21 "Sink/Sink.h"
;


#line 30 "Sink/Sink.h"
;

#line 4 "Network.cc"


#line 1 "Bridge/Bridge.h"

#line 1 "Bridge/../Scheduler/Scheduler.h"

#line 1 "Bridge/../Scheduler/../Fifo/Fifo.h"

#line 1 "Bridge/../Scheduler/../Fifo/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../Scheduler/../Fifo/Fifo.h"



#line 29 "Bridge/../Scheduler/../Fifo/Fifo.h"
;


#line 1 "Bridge/../Scheduler/Scheduler.h"


#line 1 "Bridge/../Scheduler/../Server/Server.h"

#line 1 "Bridge/../Scheduler/../Server/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../Scheduler/../Server/Server.h"



#line 30 "Bridge/../Scheduler/../Server/Server.h"
;


#line 2 "Bridge/../Scheduler/Scheduler.h"


#line 1 "Bridge/../Scheduler/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 3 "Bridge/../Scheduler/Scheduler.h"



#line 37 "Bridge/../Scheduler/Scheduler.h"
;


#line 1 "Bridge/Bridge.h"


#line 1 "Bridge/../Bridging/Bridging.h"

#line 1 "Bridge/../Bridging/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../Bridging/Bridging.h"



#line 31 "Bridge/../Bridging/Bridging.h"
;


#line 124 "Bridge/../Bridging/Bridging.h"
;


#line 134 "Bridge/../Bridging/Bridging.h"
;


#line 2 "Bridge/Bridge.h"


#line 1 "Bridge/../Driver/Driver.h"

#line 1 "Bridge/../Driver/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../Driver/Driver.h"



#line 31 "Bridge/../Driver/Driver.h"
;

#line 34 "Bridge/../Driver/Driver.h"
;

#line 37 "Bridge/../Driver/Driver.h"
;

#line 40 "Bridge/../Driver/Driver.h"
;

#line 43 "Bridge/../Driver/Driver.h"
;

#line 46 "Bridge/../Driver/Driver.h"
;

#line 49 "Bridge/../Driver/Driver.h"
;

#line 3 "Bridge/Bridge.h"


#line 1 "Bridge/../CAC/CAC.h"

#line 1 "Bridge/../CAC/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../CAC/CAC.h"



#line 32 "Bridge/../CAC/CAC.h"
;


#line 50 "Bridge/../CAC/CAC.h"
;


#line 94 "Bridge/../CAC/CAC.h"
;


#line 139 "Bridge/../CAC/CAC.h"
;


#line 4 "Bridge/Bridge.h"


#line 1 "Bridge/../STP/stp.h"

#line 1 "Bridge/../STP/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Bridge/../STP/stp.h"


#define HELLO_TIME 2
#define PRE_BACKUP_TIMER 5
#define PRE_FORWARDING_TIMER 5


#line 121 "Bridge/../STP/stp.h"
;





#line 5 "Bridge/Bridge.h"


#include <vector>

#include <string>
#include <iostream>
using namespace std;


#line 102 "Bridge/Bridge.h"
;






#line 5 "Network.cc"


#line 1 "Stats/stats.h"


#line 6 "Network.cc"


#line 1 "Empty/empty.h"

#line 1 "Empty/../Defs/packet.h"
#ifndef _MPDU_PACKET_
#define _MPDU_PACKET_

#define ACCEPT 1
#define REJECT 0

#define START 1
#define STOP 0

#define PRINT_STP 0
#define PRINT_CAC 0

#define ALLOW_OUT 0 	//Model that nodes could exit the topology (1) or not (0)


#define MAX_PORTS 7
#define N_PORTS 7	//actual number of ports that will be used
#define MAX_BRIDGES 100
#define MAX_STATIONS 100
#define LINK_COST 1


#define DATA 100
#define BPDU 200


#define ROOT_PORT 100
#define DESIGNATED_PORT 200
#define NON_DESIGNATED_PORT 300


#define FORWARDING 100
#define PRE_FORWARDING 200
#define BACKUP 300
#define PRE_BACKUP 400

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 100		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow
#define FlowAT 1
#define FlowAR 64e3





int n_stations;
int n_bridges;
int n_sinks;

struct PACKET{
	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		
	int num_hops;		
	int current_bridge;	

	
	int status; 		

	
	int begin;		

	
	int bridge_id;		
	int root_id;		
	int root_path_cost;	
	int port_id;		
	double age;		
	double MAX_AGE;		
};

struct PORT_INFO{
	
	int des_port_id;	
	int des_bridge_id;	
	int des_root_id;	
	int des_root_path_cost;	
	int state;		
	int role;		
};

PORT_INFO port[MAX_BRIDGES][MAX_PORTS];	

struct MAC_ENTRY{
	
	int mac_destination;
	int outport;
	double age;
};

struct ActiveFlows_INFO{

	int FlagActive;		
	float TimeOfActivation;	
	int SourceId;		
	float AT;		
	float AR;		
};

struct CAC_INFO{
	
	int CACId;			
	int ActiveFlows;		
	int numAdjacentCells;		
	int AdjacentCells[MAX_BRIDGES];	
	double BW;			
	int N; 				
	ActiveFlows_INFO active_flows_info[MAX_STATIONS];

};

CAC_INFO cac_info[MAX_BRIDGES];

struct STATION_INFO{
	
	int AttachedBridge;

};

STATION_INFO station_info[MAX_STATIONS];






int generated_packets[MAX_STATIONS][MAX_STATIONS];		
int received_packets[MAX_STATIONS][MAX_STATIONS];		


int received_data_packets[MAX_BRIDGES][MAX_PORTS];		
int received_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_data_packets[MAX_BRIDGES][MAX_PORTS];		
int forwarded_bpdu_packets[MAX_BRIDGES][MAX_PORTS];		
int total_bridging_received_data_packets;
int total_bridging_forwarded_data_packets;
int total_bridging_broadcasted_data_packets;


int backup_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int backup_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	


int queue_lost_data_packets[MAX_BRIDGES][MAX_PORTS];	
int queue_lost_bpdu_packets[MAX_BRIDGES][MAX_PORTS];	



double utilization_data_packets[MAX_BRIDGES][MAX_PORTS]; 
double utilization_bpdu_packets[MAX_BRIDGES][MAX_PORTS]; 


double queue_waiting_time;								
double queue_number_enqueued;							


int BlockedNewFlows[MAX_BRIDGES];
int BlockedHandoffFlows[MAX_BRIDGES];
int AcceptedNewFlows[MAX_BRIDGES];
int AcceptedHandoffFlows[MAX_BRIDGES];

int TotalFlowRequests[MAX_BRIDGES];
int TotalHandoffRequests[MAX_BRIDGES];

int StationsOutOfTopology;


#endif 


#line 1 "Empty/empty.h"



#line 7 "Network.cc"



#line 230 "Network.cc"
;

#include "compcxx_Network.h"
class compcxx_STP_19;
#line 3 "Bridge/../Bridging/Bridging.h"
class compcxx_Bridging_15 : public compcxx_component, public TypeII
{
	public: 

		int bridge_id;
		vector<MAC_ENTRY> mac_table; 

		/*inport */inline void in(PACKET& p, int ip);

		class my_Bridging_out_f_t:public compcxx_functor<Bridging_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};compcxx_array<my_Bridging_out_f_t > out;/*outportvoid out(PACKET& p)*/;

		/*inport */inline void in_bpdu(PACKET& p, int op);	
		/*outport void out_bpdu(PACKET& p, int op)*/;

		int search_destination_MAC_table(int d);
		void add_destination_MAC_table(int s, int p);

		compcxx_Bridging_15(); 
		virtual ~compcxx_Bridging_15(){}; 
		void Setup(const char *, int );
		void print_mac_table();		

		int print_bridging_debugging_points;
	private:
public:compcxx_STP_19* p_compcxx_STP_19;};

class compcxx_Bridge_21;
#line 3 "Bridge/../CAC/CAC.h"
class compcxx_CAC_17 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in(PACKET& request);
		/*outport void out(PACKET& response)*/;

		compcxx_CAC_17(){}; 
		virtual ~compcxx_CAC_17(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int CACNumber;	

public:compcxx_Bridge_21* p_compcxx_parent;};

class compcxx_Bridge_21;
#line 3 "Bridge/../Driver/Driver.h"
class compcxx_Driver_16 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in0(PACKET& p);	
		/*inport */inline void in1(PACKET& p);	
		/*inport */inline void in2(PACKET& p);	
		/*inport */inline void in3(PACKET& p);	
		/*inport */inline void in4(PACKET& p);	
		/*inport */inline void in5(PACKET& p);	
		/*inport */inline void in6(PACKET& p);	

		/*outport void out0(PACKET& p)*/;
		/*outport void out1(PACKET& p)*/;
		/*outport void out2(PACKET& p)*/;
		/*outport void out3(PACKET& p)*/;
		/*outport void out4(PACKET& p)*/;
		/*outport void out5(PACKET& p)*/;
		/*outport void out6(PACKET& p)*/;

		compcxx_Driver_16(){}; 
		virtual ~compcxx_Driver_16(){}; 
		void Setup(const char *, int ){};

	private:
public:compcxx_Bridge_21* p_compcxx_parent;};

class compcxx_Server_11;
#line 3 "Bridge/../Scheduler/../Fifo/Fifo.h"
class compcxx_Fifo_10 : public compcxx_component, public TypeII{
  public:
	compcxx_Fifo_10(){};
	virtual ~compcxx_Fifo_10(){};
	void Setup(const char *, int, int);
	void Start();

	unsigned int queue_length;

	/*inport */inline void in(PACKET& p);	
	/*outport void out(PACKET& p)*/;		
	/*inport */inline void next();			

  private:
	int bridge_id;
	int port_id;
	int FifoNumber;
	bool busy_server;					
	std::deque<PACKET> queue;			
public:compcxx_Server_11* p_compcxx_Server_11;};

class compcxx_Server_11;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_9 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_9() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_Server_11* p_compcxx_parent;};

class compcxx_Fifo_10;class compcxx_Scheduler_18;
#line 3 "Bridge/../Scheduler/../Server/Server.h"
class compcxx_Server_11 : public compcxx_component, public TypeII{
  public:
    compcxx_Server_11();
	virtual ~compcxx_Server_11() {}
	void Setup(const char *, int);

    double service_time;							

    /*inport */inline void in(PACKET& p);				
    /*outport void out(PACKET& p)*/;					
    /*outport void next()*/;							
	
    /*inport */inline void packet_served(trigger_t& t);	
    compcxx_Timer_9/*<trigger_t> */serving;

  private:
	int ServerNumber;
    PACKET packet_to_serve;							
public:compcxx_Fifo_10* p_compcxx_Fifo_10;public:compcxx_Scheduler_18* p_compcxx_parent;};


#line 5 "Bridge/../Scheduler/Scheduler.h"
class compcxx_Scheduler_18 : public compcxx_component, public TypeII{
	public: 
		compcxx_Fifo_10 fifo;
		compcxx_Server_11 server;

		/*inport */inline void in(PACKET& p);			
		class my_Scheduler_out_f_t:public compcxx_functor<Scheduler_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Scheduler_out_f_t out_f;/*outport */void out(PACKET& p);				
		
		compcxx_Scheduler_18(); 
		virtual ~compcxx_Scheduler_18(){}; 
		void Setup(const char *, int, int);

	private:
		int bridge_id;
		int SchedulerNumber;
		int print_scheduler_debugging_points;
};

class compcxx_STP_19;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_12 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_12() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_STP_19* p_compcxx_parent;};

class compcxx_STP_19;
#line 289 "../common/cost.h"
/*template <class T> */
#line 289 "../common/cost.h"
class compcxx_MultiTimer_13 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    compcxx_MultiTimer_13();
	virtual ~compcxx_MultiTimer_13();
	
	/*outport void to_component(trigger_t &, unsigned int i)*/;
	
    inline void Set(double t, unsigned int index =0);
    inline void Set( trigger_t const & data, double t,unsigned int index=0);
    void Cancel (unsigned int index=0);
    void activate(CostEvent*event);

    inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
    inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
    inline trigger_t & GetData(unsigned int index=0) { return GetEvent(index)->data; }
    inline void SetData(trigger_t const &d, unsigned int index) { GetEvent(index)->data = d; }

    event_t* GetEvent(unsigned int index);
    
 private:
    std::vector<event_t*> m_events;
    CostSimEng* m_simeng;
public:compcxx_STP_19* p_compcxx_parent;};

class compcxx_STP_19;
#line 289 "../common/cost.h"
/*template <class T> */
#line 289 "../common/cost.h"
class compcxx_MultiTimer_14 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    compcxx_MultiTimer_14();
	virtual ~compcxx_MultiTimer_14();
	
	/*outport void to_component(trigger_t &, unsigned int i)*/;
	
    inline void Set(double t, unsigned int index =0);
    inline void Set( trigger_t const & data, double t,unsigned int index=0);
    void Cancel (unsigned int index=0);
    void activate(CostEvent*event);

    inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
    inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
    inline trigger_t & GetData(unsigned int index=0) { return GetEvent(index)->data; }
    inline void SetData(trigger_t const &d, unsigned int index) { GetEvent(index)->data = d; }

    event_t* GetEvent(unsigned int index);
    
 private:
    std::vector<event_t*> m_events;
    CostSimEng* m_simeng;
public:compcxx_STP_19* p_compcxx_parent;};

class compcxx_Bridging_15;
#line 7 "Bridge/../STP/stp.h"
class compcxx_STP_19 : public compcxx_component, public TypeII{
	public: 
		int bridge_id;			
		int root_id;			
		int root_path_cost;		

		/*inport */inline void in_bpdu(PACKET& p, int ip);		
		/*outport void out_bpdu(PACKET& p, int op)*/;		

		/*inport */inline void send_bpdu_hello_time(trigger_t& t);	
		compcxx_Timer_12/*<trigger_t> */hello_time;				

		/*inport */inline void prebackup_timer_expiration(trigger_t& t, int p);	
		compcxx_MultiTimer_13/*<trigger_t> */prebackup_timer;					

		/*inport */inline void preforwarding_timer_expiration(trigger_t& t, int p);	
		compcxx_MultiTimer_14/*<trigger_t> */preforwarding_timer;				

		compcxx_STP_19(){}; 
		virtual ~compcxx_STP_19(){}; 
		void Setup(const char *, int );
		void Start();
		void Stop();

		bool updates_info(PACKET& p, int p);	
		void send_bpdu(int op);			
		void reconfigure_bridge();		

		void print_bridge_info();		
		void print_port_info(int pid);		
		int print_STP_debugging_points;
public:compcxx_Bridging_15* p_compcxx_Bridging_15;};





#line 13 "Bridge/Bridge.h"
class compcxx_Bridge_21 : public compcxx_component, public TypeII{
	public: 
		int bridge_id;		

		compcxx_Bridging_15 bridging;	
		compcxx_Driver_16 driver;
		compcxx_CAC_17 cac;
		compcxx_array<compcxx_Scheduler_18 >scheduler;
		compcxx_STP_19 stp;

		/*inport */inline void in0(PACKET& p);
		/*inport */inline void in1(PACKET& p);
		/*inport */inline void in2(PACKET& p);
		/*inport */inline void in3(PACKET& p);
		/*inport */inline void in4(PACKET& p);
		/*inport */inline void in5(PACKET& p);
		/*inport */inline void in6(PACKET& p);

		class my_Bridge_out0_f_t:public compcxx_functor<Bridge_out0_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out0_f_t out0_f;/*outport */void out0(PACKET& p);
		class my_Bridge_out1_f_t:public compcxx_functor<Bridge_out1_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out1_f_t out1_f;/*outport */void out1(PACKET& p);
		class my_Bridge_out2_f_t:public compcxx_functor<Bridge_out2_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out2_f_t out2_f;/*outport */void out2(PACKET& p);
		class my_Bridge_out3_f_t:public compcxx_functor<Bridge_out3_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out3_f_t out3_f;/*outport */void out3(PACKET& p);
		class my_Bridge_out4_f_t:public compcxx_functor<Bridge_out4_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out4_f_t out4_f;/*outport */void out4(PACKET& p);
		class my_Bridge_out5_f_t:public compcxx_functor<Bridge_out5_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out5_f_t out5_f;/*outport */void out5(PACKET& p);
		class my_Bridge_out6_f_t:public compcxx_functor<Bridge_out6_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_out6_f_t out6_f;/*outport */void out6(PACKET& p);

		/*inport */inline void to_out0(PACKET& p);		
		/*inport */inline void to_out1(PACKET& p);		
		/*inport */inline void to_out2(PACKET& p);		
		/*inport */inline void to_out3(PACKET& p);		
		/*inport */inline void to_out4(PACKET& p);		
		/*inport */inline void to_out5(PACKET& p);		
		/*inport */inline void to_out6(PACKET& p);		

		/*inport */inline void cac_in(PACKET& p);		
		class my_Bridge_cac_out_f_t:public compcxx_functor<Bridge_cac_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_cac_out_f_t cac_out_f;/*outport */void cac_out(PACKET& p);		
	
		compcxx_Bridge_21(){};
		virtual ~compcxx_Bridge_21(){}; 
		void Setup(const char *, int);

		int print_bridge_debugging_points;
};


#line 3 "Empty/empty.h"
class compcxx_Empty_24 : public compcxx_component, public TypeII{ 
	public: 
		/*inport */inline void in(PACKET& p);
		
};


#line 3 "Sink/Sink.h"
class compcxx_Sink_22 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in(PACKET& p);

		void Setup(const char *, int);

	private:
		int SinkNumber;
		int print_sink_debugging_points;

};

class compcxx_Network_26;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_25 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_25() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_Network_26* p_compcxx_parent;};

class compcxx_FlowGen_7;/*template <class T> */
#line 227 "../common/cost.h"
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
public:compcxx_FlowGen_7* p_compcxx_parent;};

class compcxx_FlowGen_7;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_FlowGen_7* p_compcxx_parent;};

class compcxx_Source_6;class compcxx_Station_20;
#line 3 "Station/../FlowGen/FlowGen.h"
class compcxx_FlowGen_7 : public compcxx_component, public TypeII{
	public: 
		/*outport void out(PACKET& request)*/;		
		/*outport void control_out(PACKET& p)*/;
		/*inport */inline void in(PACKET& response);
		/*inport */inline void control_in(int& OldCell, int& newCell);

		compcxx_Timer_2 /*<trigger_t> */interFlowRequest;
		compcxx_Timer_3 /*<trigger_t> */FlowDuration;
		/*inport */inline void timer_interFlowRequest(trigger_t& t);
		/*inport */inline void timer_FlowDuration(trigger_t& t);

		compcxx_FlowGen_7(); 
		virtual ~compcxx_FlowGen_7(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int SourceNumber;
		int interFlowRequestTime;
		int FlowDurationTime;
public:compcxx_Source_6* p_compcxx_Source_6;public:compcxx_Station_20* p_compcxx_parent;};

class compcxx_Mobility_8;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_5 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_5() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_Mobility_8* p_compcxx_parent;};

class compcxx_FlowGen_7;
#line 3 "Station/../Mobility/Mobility.h"
class compcxx_Mobility_8 : public compcxx_component, public TypeII{

	public: 
		compcxx_Mobility_8();
		virtual ~compcxx_Mobility_8(){}; 
		void Setup(const char *, int);
		/*outport void control_out(int& OldCell, int& NewCell)*/;
		compcxx_Timer_5 /*<trigger_t> */ComputeNewPosition;
		/*inport */inline void timer_ComputeNewPosition(trigger_t& t);


	private:
		int StationNumber;
public:compcxx_FlowGen_7* p_compcxx_FlowGen_7;};

class compcxx_Source_6;/*template <class T> */
#line 227 "../common/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
public:
    struct event_t : public CostEvent { trigger_t data; };
    

    compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_Source_6* p_compcxx_parent;};

class compcxx_Station_20;
#line 3 "Station/../Source/Source.h"
class compcxx_Source_6 : public compcxx_component, public TypeII{
	public: 
		/*outport void out(PACKET& p)*/;

		compcxx_Timer_4 /*<trigger_t> */interarrival;
		/*inport */inline void timer_interarrival(trigger_t& t);
		/*inport */inline void in(PACKET& p);

		compcxx_Source_6(); 
		virtual ~compcxx_Source_6(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

		int seqNum;

		int print_source_debugging_points;

	private:
		int SourceNumber;
		int InterarrivalTime;
public:compcxx_Station_20* p_compcxx_parent;};


#line 5 "Station/Station.h"
class compcxx_Station_20 : public compcxx_component, public TypeII{
	public: 
		compcxx_Source_6 source;
		compcxx_FlowGen_7 flowgen;
		compcxx_Mobility_8 mobility;

		/*inport */inline void flowGenIn(PACKET& p);	
		class my_Station_flowGenOut_f_t:public compcxx_functor<Station_flowGenOut_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Station_flowGenOut_f_t flowGenOut_f;/*outport */void flowGenOut(PACKET& p);		
		class my_Station_sourceOut_f_t:public compcxx_functor<Station_sourceOut_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Station_sourceOut_f_t sourceOut_f;/*outport */void sourceOut(PACKET& p);		

		compcxx_Station_20();
		virtual ~compcxx_Station_20(){}; 
		void Setup(const char *, int);
		

	private:
		int StationNumber;
};


#line 2 "Stats/stats.h"
class compcxx_Stats_23 : public compcxx_component, public TypeII{
	public: 
		compcxx_Stats_23(){};
		virtual ~compcxx_Stats_23(){};
		
		void print_station_stats(int n);			
		void print_bridge_port_stats(int n_b, int n_p);
		void print_global_stats();
		void init_stats();
};


#line 9 "Network.cc"
class compcxx_Network_26 : public compcxx_component, public CostSimEng
{
		compcxx_array<compcxx_Station_20 >station;
		compcxx_array<compcxx_Bridge_21 >bridge;
		compcxx_array<compcxx_Sink_22 >sink;
		compcxx_Stats_23 stats;
		compcxx_Empty_24 empty;

	public:
		compcxx_Network_26();
		void Setup(const char *);
		void Stop();
		void Start();

		/*inport */inline void start_stats_collection(trigger_t& t);	
		compcxx_Timer_25/*<trigger_t> */start_stats;			
};


#line 29 "Bridge/../Bridging/Bridging.h"
compcxx_Bridging_15 :: compcxx_Bridging_15(){
	out.SetSize(MAX_PORTS);
}
#line 33 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_15 :: Setup(const char *name, int bid){
	bridge_id=bid;
	print_bridging_debugging_points=0;
}


#line 38 "Bridge/../Bridging/Bridging.h"
int compcxx_Bridging_15 :: search_destination_MAC_table(int d){ 
	int i=0;
	int found=0;
	int op=-1;
	int v_size=mac_table.size();
	while(!found && i<v_size){
		if((mac_table.at(i)).mac_destination==d){
			op=(mac_table.at(i)).outport;
			found=1;
		}
		i++; 
	}
	return(op);	
}


#line 53 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_15 :: add_destination_MAC_table(int s, int p){
	int i=0;
	int found=0;
	int v_size=mac_table.size();
	while(!found && i<v_size){ 				
		if((mac_table.at(i)).mac_destination==s){
			vector<MAC_ENTRY>::iterator elem;
			elem=mac_table.begin() + i;
			mac_table.erase(elem);	
			MAC_ENTRY temp_m_e;
			temp_m_e.mac_destination=s;
			temp_m_e.outport=p;
			temp_m_e.age=1000;
			mac_table.push_back(temp_m_e);		
			found=1;
		}
		i++; 
	}
	if(found==0){						
		MAC_ENTRY temp_m_e;				
		temp_m_e.mac_destination=s;
		temp_m_e.outport=p;
		temp_m_e.age=1000;
		mac_table.push_back(temp_m_e);
	}
}


#line 80 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_15 :: in(PACKET &packet,int ip){ 

	if(packet.type==DATA){		
		packet.num_hops++;
		received_data_packets[bridge_id][ip]++;
		total_bridging_received_data_packets++;
		
		
		add_destination_MAC_table(packet.source,ip);
		
		
		int op=search_destination_MAC_table(packet.destination);
		if(op!=-1){
			total_bridging_forwarded_data_packets++;
			if(port[bridge_id][op].state==FORWARDING){  
				out[op](packet);
				forwarded_data_packets[bridge_id][op]++;
			}
			else{
				backup_lost_data_packets[bridge_id][op]++;
			}
		}
		else{ 
			total_bridging_broadcasted_data_packets++;
			
			if(print_bridging_debugging_points) printf("Bridging %d broadcasts packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,ip,SimTime());
			for(int i=0;i<N_PORTS;i++){
				if(i!=ip){
					if(port[bridge_id][i].state==FORWARDING){  
						out[i](packet);
						forwarded_data_packets[bridge_id][i]++;
					}
					else{
						backup_lost_data_packets[bridge_id][i]++;
					}
				}
			}
		}
	}
	
	if(packet.type==BPDU){		
		received_bpdu_packets[bridge_id][ip]++;
		(p_compcxx_STP_19->in_bpdu(packet,ip));
	}
}
#line 126 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_15 :: in_bpdu(PACKET &packet, int op){

	if(packet.type==BPDU){		
		forwarded_bpdu_packets[bridge_id][op]++;
		out[op](packet);
	}
	else
		cout << "Bridging module: a data packet received through bpdu inport!!!\n\n";
}
#line 136 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_15 :: print_mac_table(){
	cout << "BRIDGE_ID:\t" << bridge_id << "\n";
	if(mac_table.empty())	cout << "\tEMPTY\n";
	for(int i=0; i<(signed int)mac_table.size();i++)
		cout << "\tMAC:\t" << mac_table[i].mac_destination << "\tOUTPORT:\t" << mac_table[i].outport << "\n";
	cout << "\n";
}

#line 20 "Bridge/../CAC/CAC.h"
void compcxx_CAC_17 :: Setup(const char *name, int id)
{
	CACNumber = id;

	BlockedNewFlows[CACNumber] = 0;
	BlockedHandoffFlows[CACNumber] = 0;
	AcceptedNewFlows[CACNumber] = 0;
	AcceptedHandoffFlows[CACNumber] = 0;

	TotalFlowRequests[CACNumber] = 0;
	TotalHandoffRequests[CACNumber] = 0;

}
#line 34 "Bridge/../CAC/CAC.h"
void compcxx_CAC_17 :: Start(){

	cac_info[CACNumber].CACId = CACNumber;	
	cac_info[CACNumber].ActiveFlows = 0;
	
	cac_info[CACNumber].BW = CellBW;
	cac_info[CACNumber].N = (int)(CellBW / FlowRate);	

	
	for(int i = 0; i < MAX_STATIONS; i++){
		cac_info[CACNumber].active_flows_info[i].FlagActive = 0;
		cac_info[CACNumber].active_flows_info[i].TimeOfActivation = -1;
		cac_info[CACNumber].active_flows_info[i].SourceId = -1;
		cac_info[CACNumber].active_flows_info[i].AT = -1;
		cac_info[CACNumber].active_flows_info[i].AR = -1;
	}
}
#line 52 "Bridge/../CAC/CAC.h"
void compcxx_CAC_17 :: Stop(){

float BlockingNewProbability;
float BlockingHandoffProbability;
int AttachedStations = 0;



if(PRINT_CAC==1){

	printf("\nSTATISTICS CAC id %d\n", CACNumber);

	printf("\n\tRequests Received \t %d\n", TotalFlowRequests[CACNumber]);
	printf("\n\tHandoffs Received \t %d\n", TotalHandoffRequests[CACNumber]);

	printf("\n\tAccepted New Flows \t %d\n", AcceptedNewFlows[CACNumber]);
	printf("\n\tAccepted Handoff Flows \t %d\n", AcceptedHandoffFlows[CACNumber]);

	printf("\n\tBlocked New Flows \t %d\n", BlockedNewFlows[CACNumber]);
	printf("\n\tBlocked Handoff Flows \t %d\n", BlockedHandoffFlows[CACNumber]);

	if(BlockedNewFlows[CACNumber] == 0)
		BlockingNewProbability = 0;
	else
		BlockingNewProbability = (float)BlockedNewFlows[CACNumber]/(float)TotalFlowRequests[CACNumber];

	if(BlockedHandoffFlows[CACNumber] == 0)
		BlockingHandoffProbability = 0;
	else
		BlockingHandoffProbability = (float)BlockedHandoffFlows[CACNumber]/(float)TotalHandoffRequests[CACNumber];

	printf("\n\tNew Flows Block. Prob.\t %f\n", BlockingNewProbability);	
	printf("\n\tHandoff Block. Prob.\t %f\n", BlockingHandoffProbability);	

}
	for (int i=0; i<n_stations; i++){

		if(station_info[i].AttachedBridge == CACNumber)
			AttachedStations++;
	}
	printf("\t %d", AttachedStations);		

}
#line 96 "Bridge/../CAC/CAC.h"
void compcxx_CAC_17 :: in(PACKET &request)
{

PACKET response;

	if (request.destination == CACNumber){

		if(request.type == 300){			
			response.source = CACNumber;
			response.destination = request.source;
			response.current_bridge = CACNumber;
		
			if (cac_info[CACNumber].ActiveFlows < cac_info[CACNumber].N){

				response.status = ACCEPT;
				cac_info[CACNumber].ActiveFlows++;

				

				cac_info[CACNumber].active_flows_info[request.source].FlagActive = 1;
				cac_info[CACNumber].active_flows_info[request.source].TimeOfActivation = SimTime();
				cac_info[CACNumber].active_flows_info[request.source].SourceId = request.source;
				cac_info[CACNumber].active_flows_info[request.source].AT = FlowAT;
				cac_info[CACNumber].active_flows_info[request.source].AR = FlowAR;
			}
			else
				response.status = REJECT;

			(p_compcxx_parent->cac_out(response));
		}
		if (request.type == 400){			
			cac_info[CACNumber].ActiveFlows--;

				

				cac_info[CACNumber].active_flows_info[request.source].FlagActive = 0;
				cac_info[CACNumber].active_flows_info[request.source].TimeOfActivation = -1;
				cac_info[CACNumber].active_flows_info[request.source].SourceId = -1;
				cac_info[CACNumber].active_flows_info[request.source].AT = -1;
				cac_info[CACNumber].active_flows_info[request.source].AR = -1;

		}
	}
}
#line 29 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in0(PACKET &packet){
	(p_compcxx_parent->to_out0(packet));
}
#line 32 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in1(PACKET &packet){
	(p_compcxx_parent->to_out1(packet));
}
#line 35 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in2(PACKET &packet){
	(p_compcxx_parent->to_out2(packet));
}
#line 38 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in3(PACKET &packet){
	(p_compcxx_parent->to_out3(packet));
}
#line 41 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in4(PACKET &packet){
	(p_compcxx_parent->to_out4(packet));
}
#line 44 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in5(PACKET &packet){
	(p_compcxx_parent->to_out5(packet));
}
#line 47 "Bridge/../Driver/Driver.h"
void compcxx_Driver_16 :: in6(PACKET &packet){
	(p_compcxx_parent->to_out6(packet));
}
#line 24 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10 :: Setup(const char *name, int bid, int pid){
	bridge_id=bid;
	port_id=pid;
	
	queue_length=100;
}
#line 31 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10 :: Start(){
	busy_server=false;					
}


#line 35 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10::in(PACKET& packet){			
	
	if (!busy_server){					
        	(p_compcxx_Server_11->in(packet));					
        	busy_server=true;				
	} 
	else{
		if (queue.size() < queue_length){	
			queue.push_back(packet);		
		}
		else{					
			if(packet.type==DATA) queue_lost_data_packets[bridge_id][port_id]++;
			if(packet.type==BPDU) queue_lost_bpdu_packets[bridge_id][port_id]++;
		}
	}
    return;
}


#line 53 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10 :: next(){
	
	if (queue.size()>0){				
		(p_compcxx_Server_11->in(queue.front()));				
		queue.pop_front();				
	} 
	else{
		busy_server=false;					
	}
	return;
}
#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_9/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->packet_served(m_event.data));
}




#line 23 "Bridge/../Scheduler/../Server/Server.h"
compcxx_Server_11 :: compcxx_Server_11(){
    serving.p_compcxx_parent=this /*connect serving.to_component,*/;
}


#line 27 "Bridge/../Scheduler/../Server/Server.h"
void compcxx_Server_11 :: Setup(const char *name, int id){
	ServerNumber = id;
	service_time=0.5;
}
#line 32 "Bridge/../Scheduler/../Server/Server.h"
void compcxx_Server_11 :: in(PACKET& packet){
	
	packet_to_serve=packet;								
	serving.Set(SimTime() + service_time + Exponential(service_time/100));						
	return;
}


#line 39 "Bridge/../Scheduler/../Server/Server.h"
void compcxx_Server_11 :: packet_served(trigger_t& t){
	
    (p_compcxx_parent->out(packet_to_serve));							
    (p_compcxx_Fifo_10->next());											
    return;
}

#line 10 "Bridge/../Scheduler/Scheduler.h"

#line 11 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_18::out(PACKET& p){return (out_f(p));}
#line 23 "Bridge/../Scheduler/Scheduler.h"
compcxx_Scheduler_18::compcxx_Scheduler_18(){
	
	fifo.p_compcxx_Server_11=&server /*connect fifo.out,server.in*/;
	server.p_compcxx_Fifo_10=&fifo /*connect server.next,fifo.next*/;
	server.p_compcxx_parent=this /*connect server.out,*/;
}


#line 30 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_18 :: Setup(const char *name, int bid, int sid){
	bridge_id=bid;
	SchedulerNumber = sid;
	print_scheduler_debugging_points=0;
	
	fifo.Setup("fifo",bid,sid);	
	server.Setup("server",bid);
}
#line 39 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_18 :: in(PACKET& packet){
	if(print_scheduler_debugging_points && packet.type==DATA) printf("Scheduler %d in bridge %d receives packet type %d from source %d to destination %d (%f sec)\n\n",SchedulerNumber,bridge_id,packet.type,packet.source,packet.destination,SimTime());
	fifo.in(packet);
}
#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_12/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->send_bpdu_hello_time(m_event.data));
}




#line 316 "../common/cost.h"
compcxx_MultiTimer_13
#line 315 "../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_MultiTimer_13()
{
	m_simeng = CostSimEng::Instance(); 
	GetEvent(0);
}


#line 323 "../common/cost.h"
compcxx_MultiTimer_13
#line 322 "../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_MultiTimer_13()
{
	for(unsigned int i=0;i<m_events.size();i++)
		delete m_events[i];
}


#line 329 "../common/cost.h"
/*template <class T>
*//*typename */compcxx_MultiTimer_13/*<T>*/::event_t* compcxx_MultiTimer_13/*<trigger_t >*/::GetEvent(unsigned int index)
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


#line 344 "../common/cost.h"

#line 344 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_13/*<trigger_t >*/::Set(trigger_t const & data, double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->data = data;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 356 "../common/cost.h"

#line 356 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_13/*<trigger_t >*/::Set(double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 367 "../common/cost.h"

#line 367 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_13/*<trigger_t >*/::Cancel(unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)
    	m_simeng->CancelEvent(e);
    e->active = false;
}


#line 376 "../common/cost.h"

#line 376 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_13/*<trigger_t >*/::activate(CostEvent*e)
{
	event_t * event = (event_t*)e;
	event->active = false;
    (p_compcxx_parent->prebackup_timer_expiration(event->data,event->index));
}





#line 316 "../common/cost.h"
compcxx_MultiTimer_14
#line 315 "../common/cost.h"
/*template <class T>
*//*<T>*/::compcxx_MultiTimer_14()
{
	m_simeng = CostSimEng::Instance(); 
	GetEvent(0);
}


#line 323 "../common/cost.h"
compcxx_MultiTimer_14
#line 322 "../common/cost.h"
/*template <class T>
*//*<T>*/::~compcxx_MultiTimer_14()
{
	for(unsigned int i=0;i<m_events.size();i++)
		delete m_events[i];
}


#line 329 "../common/cost.h"
/*template <class T>
*//*typename */compcxx_MultiTimer_14/*<T>*/::event_t* compcxx_MultiTimer_14/*<trigger_t >*/::GetEvent(unsigned int index)
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


#line 344 "../common/cost.h"

#line 344 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_14/*<trigger_t >*/::Set(trigger_t const & data, double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->data = data;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 356 "../common/cost.h"

#line 356 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_14/*<trigger_t >*/::Set(double time, unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)m_simeng->CancelEvent(e);
    e->time = time;
    e->object = this;
    e->active = true;
    m_simeng->ScheduleEvent(e);
}


#line 367 "../common/cost.h"

#line 367 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_14/*<trigger_t >*/::Cancel(unsigned int index)
{
    event_t * e = GetEvent(index);
    if(e->active)
    	m_simeng->CancelEvent(e);
    e->active = false;
}


#line 376 "../common/cost.h"

#line 376 "../common/cost.h"
/*template <class T>
*/void compcxx_MultiTimer_14/*<trigger_t >*/::activate(CostEvent*e)
{
	event_t * event = (event_t*)e;
	event->active = false;
    (p_compcxx_parent->preforwarding_timer_expiration(event->data,event->index));
}





#line 19 "Bridge/../STP/stp.h"

#line 22 "Bridge/../STP/stp.h"

#line 43 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: Setup(const char *name, int bid){


	hello_time.p_compcxx_parent=this /*connect hello_time.to_component,*/;
	preforwarding_timer.p_compcxx_parent=this /*connect preforwarding_timer.to_component,*/;
	prebackup_timer.p_compcxx_parent=this /*connect prebackup_timer.to_component,*/;

	bridge_id=bid;		
	root_id=bid;		
	root_path_cost=0;	

	for(int i=0;i<N_PORTS;i++){			
		port[bridge_id][i].des_port_id=i;
		port[bridge_id][i].des_bridge_id=bid;
		port[bridge_id][i].des_root_id=bid;
		port[bridge_id][i].des_root_path_cost=0;
		port[bridge_id][i].state=PRE_FORWARDING;
		port[bridge_id][i].role=DESIGNATED_PORT;
		preforwarding_timer.Set(0 + PRE_FORWARDING_TIMER, i);
		
	}

	hello_time.Set(0.0001);	
	print_STP_debugging_points=0;
}





#line 72 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: Start(){

}





#line 79 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: Stop(){

}





#line 86 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: in_bpdu(PACKET &bpdu, int in_port){


	if(bpdu.type!=BPDU)
		if(PRINT_STP==1)	cout << "STP module: a non-bpdu packet has been received!!!\n\n";
	else if(print_STP_debugging_points)
		if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") has received a BPDU in port " << in_port << " (at " << SimTime() << " sec)\n\tBRIDGE_ID: " << bpdu.bridge_id << "\tROOT_ID: " << bpdu.root_id << "\tRPC: " << bpdu.root_path_cost << "\tPORT_ID: " << bpdu.port_id << "\n\n";

	if(updates_info(bpdu, in_port)){	
		
		
		root_id=bpdu.root_id;
		root_path_cost=bpdu.root_path_cost + LINK_COST;

		
		port[bridge_id][in_port].des_port_id=bpdu.port_id;
		port[bridge_id][in_port].des_bridge_id=bpdu.bridge_id;
		port[bridge_id][in_port].des_root_id=bpdu.root_id;
		port[bridge_id][in_port].des_root_path_cost=bpdu.root_path_cost;

		reconfigure_bridge();

		
		for(int n=0;n<N_PORTS;n++){
			if(port[bridge_id][n].role==DESIGNATED_PORT)
				send_bpdu(n);
		}
		
	}
	else{	
		if(print_STP_debugging_points) cout << "STP(" << bridge_id << ") has received a non-updating BPDU in port " << in_port << " (at " << SimTime() << " sec)\n\n";
		if(port[bridge_id][in_port].role==DESIGNATED_PORT)
			send_bpdu(in_port); 
	}
	
}
#line 126 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: reconfigure_bridge(){


	
	int root_port_tmp=0;
	int min_rpc=1000000;
	for(int i=0;i<N_PORTS;i++){
		if(port[bridge_id][i].des_root_path_cost<min_rpc && port[bridge_id][i].des_root_id==root_id){ 
			root_port_tmp=i;
			min_rpc=port[bridge_id][i].des_root_path_cost;
		}
	}
	port[bridge_id][root_port_tmp].role=ROOT_PORT;
	root_path_cost=min_rpc + LINK_COST;

	
	for(int i=0;i<N_PORTS;i++){
		if(i!=root_port_tmp){

			if(port[bridge_id][i].des_root_id != root_id){		
				port[bridge_id][i].role=DESIGNATED_PORT;
			}
			else{
				if(port[bridge_id][i].des_root_path_cost > root_path_cost){
					port[bridge_id][i].role=DESIGNATED_PORT;
				}
				else{
					if(port[bridge_id][i].des_root_path_cost == root_path_cost && port[bridge_id][i].des_bridge_id > bridge_id){
						port[bridge_id][i].role=DESIGNATED_PORT;
					}
					else{
						if(port[bridge_id][i].des_root_path_cost  == root_path_cost && port[bridge_id][i].des_bridge_id == bridge_id && port[bridge_id][i].des_port_id > i){
						}
						else{
							if(port[bridge_id][i].des_root_path_cost  == root_path_cost && port[bridge_id][i].des_bridge_id == bridge_id && port[bridge_id][i].des_port_id == i) 
								port[bridge_id][i].role=DESIGNATED_PORT;
							else
								port[bridge_id][i].role=NON_DESIGNATED_PORT;
						}
					}
				}
			}
			if(port[bridge_id][i].role==DESIGNATED_PORT){	
				port[bridge_id][i].des_port_id=i;
				port[bridge_id][i].des_bridge_id=bridge_id;
				port[bridge_id][i].des_root_id=root_id;
				port[bridge_id][i].des_root_path_cost=root_path_cost;
			}
		}
	}
	
	
	for(int i=0;i<N_PORTS;i++){
		if(port[bridge_id][i].role==ROOT_PORT || port[bridge_id][i].role==DESIGNATED_PORT){ 
			switch (port[bridge_id][i].state){
				case FORWARDING: 	break;
				case PRE_FORWARDING: 	break;
				case BACKUP: 		port[bridge_id][i].state=PRE_FORWARDING; preforwarding_timer.Set(SimTime() + PRE_FORWARDING_TIMER, i); break;
				case PRE_BACKUP: 	port[bridge_id][i].state=FORWARDING; prebackup_timer.Cancel(i); break;
				default:		if(PRINT_STP==1)	cout << "State ERROR in reconfigure_bridge()\n";
			}
		}

		if(port[bridge_id][i].role==NON_DESIGNATED_PORT){ 
			switch (port[bridge_id][i].state){
				case FORWARDING: 	port[bridge_id][i].state=PRE_BACKUP; prebackup_timer.Set(SimTime() + PRE_BACKUP_TIMER, i); break;
				case PRE_FORWARDING: 	port[bridge_id][i].state=BACKUP; preforwarding_timer.Cancel(i); break;
				case BACKUP: 		break;
				case PRE_BACKUP: 	break;
				default:		if(PRINT_STP==1)	cout << "State ERROR in reconfigure_bridge()\n";
			}
		}

	}
	
 	if(print_STP_debugging_points) print_bridge_info();
}





#line 207 "Bridge/../STP/stp.h"
bool compcxx_STP_19 :: updates_info(PACKET& bpdu, int p){


	if(bpdu.root_id < port[bridge_id][p].des_root_id){
		return true;
	}
	else{
		if(bpdu.root_id == port[bridge_id][p].des_root_id && bpdu.root_path_cost < port[bridge_id][p].des_root_path_cost){
			return true;
		}
		else{
			if(bpdu.root_id == port[bridge_id][p].des_root_id && (bpdu.root_path_cost) == port[bridge_id][p].des_root_path_cost && bpdu.bridge_id < port[bridge_id][p].des_bridge_id){
				return true;	
			}
			else{
				if(bpdu.root_id == port[bridge_id][p].des_root_id && (bpdu.root_path_cost) == port[bridge_id][p].des_root_path_cost && bpdu.bridge_id == port[bridge_id][p].des_bridge_id && bpdu.port_id < port[bridge_id][p].des_port_id){
					return true;
				}
			}
		}
	}
	return false;
}





#line 234 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: send_bpdu(int op){


	PACKET bpdu;					
	bpdu.type=BPDU;
	bpdu.bridge_id=bridge_id;
	bpdu.root_id=root_id;
	bpdu.root_path_cost=root_path_cost;
	bpdu.port_id=op;

	bpdu.source=-1;
	bpdu.destination=-1;
	

	if(print_STP_debugging_points) cout << "STP(" << bridge_id << ") sends a BPDU through port " << op <<" (at " << SimTime() <<" sec)\n\tBRIDGE_ID: " << bpdu.bridge_id << "\tROOT_ID: " << bpdu.root_id << "\tRPC: " << bpdu.root_path_cost << "\tPORT_ID: " << bpdu.port_id << "\n\n";
	(p_compcxx_Bridging_15->in_bpdu(bpdu,op));
}





#line 255 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: send_bpdu_hello_time(trigger_t& t){


	if(bridge_id==root_id){		
		
		for(int n=0;n<N_PORTS;n++){
			if(port[bridge_id][n].role==DESIGNATED_PORT)
				send_bpdu(n);
		}
	}
	hello_time.Set(SimTime() + HELLO_TIME);
}





#line 271 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: preforwarding_timer_expiration(trigger_t& t, int p){

	port[bridge_id][p].state=FORWARDING;

	if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_forwarding timer expiration (at " << SimTime() << " sec)\n\n";

	if(print_STP_debugging_points){
		if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_forwarding timer expiration (at " << SimTime() << " sec)\n\n";
		if(PRINT_STP==1)	print_bridge_info();
	}
}





#line 286 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: prebackup_timer_expiration(trigger_t& t, int p){

	port[bridge_id][p].state=BACKUP;

	if(PRINT_STP==1)	cout << "STP(" << bridge_id << ") at port " << p << ": pre_backup timer expiration (at " << SimTime() << " sec)\n\n";

	if(print_STP_debugging_points){
		cout << "STP(" << bridge_id << ") at port " << p << ": pre_backup timer expiration (at " << SimTime() << " sec)\n\n";
		print_bridge_info();
	}
}





#line 301 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: print_bridge_info(){


	cout << "***************************************************\n";
	cout << "BRIDGE_ID:\t" << bridge_id << "\nROOT_ID:\t" << root_id << "\nROOT_PATH_COST:\t" << root_path_cost << "\n\n";
	for(int i=0;i<N_PORTS;i++)
		print_port_info(i);
	cout << "***************************************************\n\n";
}





#line 314 "Bridge/../STP/stp.h"
void compcxx_STP_19 :: print_port_info(int pid){


	string state_string,role_string;
	switch (port[bridge_id][pid].state){
		case 100: state_string="FORWARDING"; break;
		case 200: state_string="PRE_FORWARDING"; break;
		case 300: state_string="BACKUP"; break;
		case 400: state_string="PRE_BACKUP"; break;
	}

	switch (port[bridge_id][pid].role){
		case 100: role_string="ROOT_PORT"; break;
		case 200: role_string="DESIGNATED_PORT"; break;
		case 300: role_string="NON_DESIGNATED_PORT"; break;
	}

	cout << "\tPORT ID " << pid << "\n\t*********\n\tDES_PORT_ID:\t" << port[bridge_id][pid].des_port_id << "\n\tDES_BRIDGE_ID:\t" << port[bridge_id][pid].des_bridge_id << " \n\tDES_ROOT_ID:\t" << port[bridge_id][pid].des_root_id << "\n\tDES_RPC:\t" << port[bridge_id][pid].des_root_path_cost << "\n\tSTATE:\t\t" << state_string << "\n\tROLE:\t\t" << role_string << "\n\n";
}

#line 23 "Bridge/Bridge.h"

#line 24 "Bridge/Bridge.h"

#line 25 "Bridge/Bridge.h"

#line 26 "Bridge/Bridge.h"

#line 27 "Bridge/Bridge.h"

#line 28 "Bridge/Bridge.h"

#line 29 "Bridge/Bridge.h"

#line 31 "Bridge/Bridge.h"
void compcxx_Bridge_21::out0(PACKET& p){return (out0_f(p));}
#line 32 "Bridge/Bridge.h"
void compcxx_Bridge_21::out1(PACKET& p){return (out1_f(p));}
#line 33 "Bridge/Bridge.h"
void compcxx_Bridge_21::out2(PACKET& p){return (out2_f(p));}
#line 34 "Bridge/Bridge.h"
void compcxx_Bridge_21::out3(PACKET& p){return (out3_f(p));}
#line 35 "Bridge/Bridge.h"
void compcxx_Bridge_21::out4(PACKET& p){return (out4_f(p));}
#line 36 "Bridge/Bridge.h"
void compcxx_Bridge_21::out5(PACKET& p){return (out5_f(p));}
#line 37 "Bridge/Bridge.h"
void compcxx_Bridge_21::out6(PACKET& p){return (out6_f(p));}
#line 39 "Bridge/Bridge.h"

#line 40 "Bridge/Bridge.h"

#line 41 "Bridge/Bridge.h"

#line 42 "Bridge/Bridge.h"

#line 43 "Bridge/Bridge.h"

#line 44 "Bridge/Bridge.h"

#line 45 "Bridge/Bridge.h"

#line 47 "Bridge/Bridge.h"
inline void compcxx_Bridge_21::cac_in(PACKET& p){return (cac.in(p));}
#line 48 "Bridge/Bridge.h"
void compcxx_Bridge_21::cac_out(PACKET& p){return (cac_out_f(p));}
#line 57 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: Setup(const char *name, int bid){

	bridge_id=bid;

	bridging.Setup("bridging",bid);

	stp.Setup("STP",bid);

	scheduler.SetSize(MAX_PORTS);
	for(int i=0;i<MAX_PORTS;i++){
		scheduler[i].Setup("scheduler",bid,i);
	}
	cac.Setup("cac",bid);

	bridging.out[0].Connect(scheduler[0],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[0],scheduler[0].in*/;
	bridging.out[1].Connect(scheduler[1],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[1],scheduler[1].in*/;
	bridging.out[2].Connect(scheduler[2],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[2],scheduler[2].in*/;
	bridging.out[3].Connect(scheduler[3],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[3],scheduler[3].in*/;
	bridging.out[4].Connect(scheduler[4],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[4],scheduler[4].in*/;
	bridging.out[5].Connect(scheduler[5],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[5],scheduler[5].in*/;
	bridging.out[6].Connect(scheduler[6],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_18::in) /*connect bridging.out[6],scheduler[6].in*/;

	scheduler[0].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in0) /*connect scheduler[0].out,driver.in0*/;
	scheduler[1].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in1) /*connect scheduler[1].out,driver.in1*/;
	scheduler[2].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in2) /*connect scheduler[2].out,driver.in2*/;
	scheduler[3].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in3) /*connect scheduler[3].out,driver.in3*/;
	scheduler[4].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in4) /*connect scheduler[4].out,driver.in4*/;
	scheduler[5].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in5) /*connect scheduler[5].out,driver.in5*/;
	scheduler[6].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_16::in6) /*connect scheduler[6].out,driver.in6*/;

	driver.p_compcxx_parent=this /*connect driver.out0,*/;
	driver.p_compcxx_parent=this /*connect driver.out1,*/;
	driver.p_compcxx_parent=this /*connect driver.out2,*/;
	driver.p_compcxx_parent=this /*connect driver.out3,*/;
	driver.p_compcxx_parent=this /*connect driver.out4,*/;
	driver.p_compcxx_parent=this /*connect driver.out5,*/;
	driver.p_compcxx_parent=this /*connect driver.out6,*/;

	bridging.p_compcxx_STP_19=&stp /*connect bridging.out_bpdu,stp.in_bpdu*/;
	stp.p_compcxx_Bridging_15=&bridging /*connect stp.out_bpdu, bridging.in_bpdu*/;

	cac.p_compcxx_parent=this /*connect cac.out,*/;
	 /*connect ,cac.in*/;

	print_bridge_debugging_points=0;
}
#line 108 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in0(PACKET& packet){
	if(packet.current_bridge==bridge_id){
		if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,0,SimTime());
		if(PRINT_CAC == 1) 	printf("\tBridge %d -> Received packet from source %d to destination %d \n",bridge_id,packet.source,packet.destination);
		bridging.in(packet,0);
	}
}


#line 116 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in1(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,1,SimTime());
	bridging.in(packet,1);
}


#line 121 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in2(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,2,SimTime());
	bridging.in(packet,2);
}


#line 126 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in3(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,3,SimTime());
	bridging.in(packet,3);
}


#line 131 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in4(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,4,SimTime());
	bridging.in(packet,4);
}


#line 136 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in5(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,5,SimTime());
	bridging.in(packet,5);
}


#line 141 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: in6(PACKET& packet){
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d receives packet from source %d to destination %d in inport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,6,SimTime());
	bridging.in(packet,6);
}






#line 150 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out0(PACKET& packet){			
	
	packet.current_bridge=bridge_id; 
	(out0_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,0,SimTime());
}


#line 157 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out1(PACKET& packet){			
	
	(out1_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,1,SimTime());
}


#line 163 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out2(PACKET& packet){			
	
	(out2_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,2,SimTime());
}


#line 169 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out3(PACKET& packet){			
	
	(out3_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,3,SimTime());
}


#line 175 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out4(PACKET& packet){			
	
	(out4_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,4,SimTime());
}


#line 181 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out5(PACKET& packet){			
	
	(out5_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,5,SimTime());
}


#line 187 "Bridge/Bridge.h"
void compcxx_Bridge_21 :: to_out6(PACKET& packet){			
	
	(out6_f(packet));
	if(print_bridge_debugging_points && packet.type==DATA) printf("Bridge %d sends packet from source %d to destination %d to outport %d (%f sec)\n\n",bridge_id,packet.source,packet.destination,6,SimTime());
}

#line 9 "Empty/empty.h"
void compcxx_Empty_24 :: in(PACKET& p){}
#line 16 "Sink/Sink.h"
void compcxx_Sink_22 :: Setup(const char *name, int id)
{
	SinkNumber = id;
	print_sink_debugging_points=0;

}
#line 23 "Sink/Sink.h"
void compcxx_Sink_22 :: in(PACKET& packet){

	if(packet.type==DATA && packet.current_bridge==station_info[SinkNumber].AttachedBridge){
		if(print_sink_debugging_points) printf("\t\t\t\tSink %d -> Received packet\n \t\t\t\t\t seqNum: %d from %d to %d, num_hops: %d (at %f)\n\n",SinkNumber,packet.seqNum,packet.source,packet.destination,packet.num_hops,SimTime());
		
		if(packet.destination==SinkNumber) received_packets[packet.source][packet.destination]++;
	}
}
#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_25/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_25/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_25/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_25/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->start_stats_collection(m_event.data));
}




#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
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


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
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


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->timer_FlowDuration(m_event.data));
}




#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->timer_interFlowRequest(m_event.data));
}




#line 12 "Station/../FlowGen/FlowGen.h"

#line 13 "Station/../FlowGen/FlowGen.h"

#line 27 "Station/../FlowGen/FlowGen.h"
compcxx_FlowGen_7::compcxx_FlowGen_7(){

	interFlowRequest.p_compcxx_parent=this /*connect interFlowRequest.to_component,*/;
	FlowDuration.p_compcxx_parent=this /*connect FlowDuration.to_component,*/;
}


#line 33 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Setup(const char *name, int id){

	SourceNumber = id;
}
#line 38 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Start(){
	interFlowRequestTime = 10;	
	FlowDurationTime = 5;
	interFlowRequest.Set(5);
	
}
#line 45 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Stop(){
}
#line 48 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: timer_interFlowRequest(trigger_t &){

	PACKET request;

	TotalFlowRequests[station_info[SourceNumber].AttachedBridge]++;
	
	request.source = SourceNumber;
	request.destination = station_info[SourceNumber].AttachedBridge;
	request.type = 300;		
	if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowRequest packet from %d to %d at %f",SourceNumber, request.source, request.destination, SimTime());
	(p_compcxx_parent->flowGenOut(request));

}
#line 62 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: timer_FlowDuration(trigger_t &){

	PACKET stoppacketGen;
	PACKET notifyend;
	
	

	stoppacketGen.begin = STOP;
	interFlowRequest.Set(interFlowRequestTime + SimTime());
	(p_compcxx_Source_6->in(stoppacketGen));	
	
	

	notifyend.source = SourceNumber;
	notifyend.destination = station_info[SourceNumber].AttachedBridge;
	notifyend.type = 400;
	if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowDelete packet from %d to %d at %f",SourceNumber, notifyend.source, notifyend.destination, SimTime());
	(p_compcxx_parent->flowGenOut(notifyend));

}
#line 83 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: in(PACKET &response)
{

	

	PACKET startpacketGen;

	if (response.destination == SourceNumber && response.current_bridge == station_info[SourceNumber].AttachedBridge){
			
		if(response.status == ACCEPT){

			startpacketGen.begin = START;
			if(!FlowDuration.Active()){		
				FlowDuration.Set(FlowDurationTime + SimTime());
				AcceptedNewFlows[station_info[SourceNumber].AttachedBridge]++;

			}
			else
				AcceptedHandoffFlows[station_info[SourceNumber].AttachedBridge]++;
			if(PRINT_CAC==1)	printf("\nFlowGen %d -> Received FlowResponse packet from %d at %f with status ACCEPT\n", response.destination, response.source, SimTime());

			(p_compcxx_Source_6->in(startpacketGen));


		}
		else{		

			if(FlowDuration.Active()){		
					FlowDuration.Cancel();	
					BlockedHandoffFlows[station_info[SourceNumber].AttachedBridge]++;
			}
			else
				BlockedNewFlows[station_info[SourceNumber].AttachedBridge]++;

			interFlowRequest.Set(interFlowRequestTime + SimTime());
			if(PRINT_CAC==1)	printf("\nFlowGen %d -> Received FlowResponse packet from %d at %f with status REJECT\n", response.destination,response.source, SimTime());
			
		}
	}
}
#line 123 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: control_in(int &OldCell, int &NewCell){




	PACKET stoppacketGen;
	PACKET notifyend;
	PACKET request;

	if(FlowDuration.Active()){			

		

		stoppacketGen.begin = STOP;
		(p_compcxx_Source_6->in(stoppacketGen));	
		
		

		notifyend.source = SourceNumber;
		notifyend.destination = OldCell;
		notifyend.type = 400;
		if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowDelete packet from %d to %d at %f",SourceNumber, notifyend.source, notifyend.destination, SimTime());
		(p_compcxx_parent->flowGenOut(notifyend));

		if(station_info[SourceNumber].AttachedBridge != -1){

			

			TotalHandoffRequests[station_info[SourceNumber].AttachedBridge]++;

			request.source = SourceNumber;
			request.destination = station_info[SourceNumber].AttachedBridge;
			request.type = 300;		
			if(PRINT_CAC==1)	printf("\n\nFlowGen %d -> Send FlowRequest Handoff packet from %d to %d at %f",SourceNumber, request.source, request.destination, SimTime());
			(p_compcxx_parent->flowGenOut(request));
		}
		else
			FlowDuration.Cancel();
		
	}
	if(station_info[SourceNumber].AttachedBridge == -1 && interFlowRequest.Active()){
		interFlowRequest.Cancel();
	}
}
#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->timer_ComputeNewPosition(m_event.data));
}




#line 11 "Station/../Mobility/Mobility.h"

#line 18 "Station/../Mobility/Mobility.h"
compcxx_Mobility_8::compcxx_Mobility_8(){		
	ComputeNewPosition.p_compcxx_parent=this /*connect ComputeNewPosition.to_component,*/;
}


#line 22 "Station/../Mobility/Mobility.h"
void compcxx_Mobility_8 :: Setup(const char *name, int id){

	StationNumber = id;
	ComputeNewPosition.Set(Tobs);		
	station_info[StationNumber].AttachedBridge = 0;

}
#line 31 "Station/../Mobility/Mobility.h"
void compcxx_Mobility_8 :: timer_ComputeNewPosition(trigger_t &){

int change = rand() % 101;
int OldCell;
int NewCell;
int Out = 0;

	OldCell = station_info[StationNumber].AttachedBridge;

	if((ALLOW_OUT == 1) && (OldCell > 6))		
		Out = 1;

	if(change < ProbHandoff){			

		NewCell = rand()%(cac_info[OldCell].numAdjacentCells+Out);

		while(NewCell==station_info[StationNumber].AttachedBridge){
			NewCell = rand()%(cac_info[OldCell].numAdjacentCells+Out); 
		}
		station_info[StationNumber].AttachedBridge = cac_info[OldCell].AdjacentCells[NewCell];
		
			if(station_info[StationNumber].AttachedBridge == -1){
				if(PRINT_CAC == 1) printf("\n\t\t--Station %d - Out of the topology--\n", StationNumber);
				StationsOutOfTopology++;
				if(StationsOutOfTopology == n_stations)
					printf("\n\t\t--All stations are out of the topology at time %f--\n", SimTime());
			}
			else
				if(PRINT_CAC == 1) printf("\n\t\t--Station %d - Change Cell from %d to %d--\n", StationNumber, OldCell, station_info[StationNumber].AttachedBridge);
		
	(p_compcxx_FlowGen_7->control_in(OldCell, station_info[StationNumber].AttachedBridge));
		
	}
	if(station_info[StationNumber].AttachedBridge != -1)	
		ComputeNewPosition.Set(Tobs + SimTime());
}
#line 248 "../common/cost.h"

#line 248 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.data = data;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 260 "../common/cost.h"

#line 260 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
    if(m_event.active)
   		m_simeng->CancelEvent(&m_event);
    m_event.time = time;
    m_event.object = this;
    m_event.active=true;
    m_simeng->ScheduleEvent(&m_event);
}


#line 271 "../common/cost.h"

#line 271 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
    if(m_event.active)
    	m_simeng->CancelEvent(&m_event);
    m_event.active = false;
}


#line 279 "../common/cost.h"

#line 279 "../common/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
	assert(e==&m_event);
	m_event.active=false;
    (p_compcxx_parent->timer_interarrival(m_event.data));
}




#line 8 "Station/../Source/Source.h"

#line 9 "Station/../Source/Source.h"

#line 26 "Station/../Source/Source.h"
compcxx_Source_6::compcxx_Source_6(){
	interarrival.p_compcxx_parent=this /*connect interarrival.to_component,*/;
}


#line 30 "Station/../Source/Source.h"
void compcxx_Source_6 :: Setup(const char *name, int id){
	SourceNumber = id;
	seqNum=0;
	srand(time(0));
	print_source_debugging_points=0;
}
#line 37 "Station/../Source/Source.h"
void compcxx_Source_6 :: Start(){
	InterarrivalTime = 1;	
}
#line 41 "Station/../Source/Source.h"
void compcxx_Source_6 :: Stop(){
}
#line 44 "Station/../Source/Source.h"
void compcxx_Source_6 :: timer_interarrival(trigger_t &){

	

	PACKET packet;
	packet.source = SourceNumber;

	if(n_stations == 1)
		packet.destination =SourceNumber;
	else{

		packet.destination = rand()%n_stations;
		while(packet.destination==SourceNumber){
			packet.destination = rand()%n_stations; 
		}
	}

	packet.type = 100;
	packet.flowId = 0;
	packet.seqNum = seqNum++; 
	packet.num_hops=0;
	packet.current_bridge=station_info[SourceNumber].AttachedBridge;

	if(print_source_debugging_points) printf("Source %d -> Send packet\n \t seqNum: %d from %d to %d at %f\n\n",SourceNumber,packet.seqNum,packet.source,packet.destination, SimTime());
	if(PRINT_CAC)	printf("\n\tSource %d -> Send packet from %d to %d at %f\n",SourceNumber,packet.source,packet.destination, SimTime());
	interarrival.Set(InterarrivalTime + SimTime());
	generated_packets[packet.source][packet.destination]++;
	(p_compcxx_parent->sourceOut(packet));
}
#line 74 "Station/../Source/Source.h"
void compcxx_Source_6 :: in(PACKET &packet)
{
	if(packet.begin == START){
		interarrival.Set(SimTime());
		if(print_source_debugging_points) printf("Source %d -> Received START packet from FlowGen at %f\n",SourceNumber, SimTime());
		seqNum=0;
	}
	if(packet.begin == STOP){
		interarrival.Cancel();
		if(print_source_debugging_points) printf("Source -> Received STOP packet from FlowGen at %f\n", SimTime());
	}
}
#line 11 "Station/Station.h"
inline void compcxx_Station_20::flowGenIn(PACKET& p){return (flowgen.in(p));}
#line 12 "Station/Station.h"
void compcxx_Station_20::flowGenOut(PACKET& p){return (flowGenOut_f(p));}
#line 13 "Station/Station.h"
void compcxx_Station_20::sourceOut(PACKET& p){return (sourceOut_f(p));}
#line 24 "Station/Station.h"
compcxx_Station_20::compcxx_Station_20(){	
	 /*connect ,flowgen.in*/;
	flowgen.p_compcxx_Source_6=&source /*connect flowgen.control_out,source.in*/;
	flowgen.p_compcxx_parent=this /*connect flowgen.out,*/;
	source.p_compcxx_parent=this /*connect source.out,*/;
	mobility.p_compcxx_FlowGen_7=&flowgen /*connect mobility.control_out, flowgen.control_in*/;
}


#line 32 "Station/Station.h"
void compcxx_Station_20 :: Setup(const char *name, int id){
	StationNumber = id;
	source.Setup("source",id);
	flowgen.Setup("FlowGen",id);
	mobility.Setup("Mobility",id);
}
#line 13 "Stats/stats.h"
void compcxx_Stats_23 :: print_global_stats(){
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF RECEIVED PACKETS IN BRIDGING MODULES:\t\t%d\n",total_bridging_received_data_packets);
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF FORWARDED PACKETS IN BRIDGING MODULES:\t\t%d\n",total_bridging_forwarded_data_packets);
	if(PRINT_STP==1)	printf("TOTAL NUMBER OF BROADCASTED PACKETS IN BRIDGING MODULES:\t%d\n\n",total_bridging_broadcasted_data_packets);
}


#line 19 "Stats/stats.h"
void compcxx_Stats_23 :: print_station_stats(int n_stations){
	for(int so=0;so<n_stations;so++){
		if(PRINT_STP==1)	printf("Station %d\n",so);
		for(int si=0;si<n_stations;si++){
			if(PRINT_STP==1)	printf("\tto station %d:\t %d generated packets\n",si,generated_packets[so][si]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
	
	for(int so=0;so<n_stations;so++){
		if(PRINT_STP==1)	printf("Station %d\n",so);
		for(int si=0;si<n_stations;si++){
			if(PRINT_STP==1)	printf("\tto station %d:\t %d received packets\n",si,received_packets[so][si]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
}


#line 37 "Stats/stats.h"
void compcxx_Stats_23 :: print_bridge_port_stats(int n_bridges, int n_ports){
	for(int b=0;b<n_bridges;b++){
		if(PRINT_STP==1)	printf("Bridge %d\n",b);
		for(int p=0;p<n_ports;p++){
			if(PRINT_STP==1)	printf("\tport %d\n",p);
			if(PRINT_STP==1)	printf("\t\tIN:\t\tdata:\t%d\tbpdu:\t%d\n",received_data_packets[b][p],received_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\tOUT:\t\tdata:\t%d\tbpdu:\t%d\n",forwarded_data_packets[b][p],forwarded_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  LOST BACKUP:\tdata:\t%d\tbpdu:\t%d\n",backup_lost_data_packets[b][p],backup_lost_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  LOST QUEUE:\tdata:\t%d\tbpdu:\t%d\n",queue_lost_data_packets[b][p],queue_lost_bpdu_packets[b][p]);
			if(PRINT_STP==1)	printf("\t\t  UTILIZATION:\tdata:\t%0.2f\tbpdu:\t%0.2f\n",utilization_data_packets[b][p],utilization_bpdu_packets[b][p]);
		}
		if(PRINT_STP==1)	printf("\n");
	}
}


#line 52 "Stats/stats.h"
void compcxx_Stats_23 :: init_stats(){
	total_bridging_received_data_packets=0;
	total_bridging_forwarded_data_packets=0;
	total_bridging_broadcasted_data_packets=0;
	
	for(int i=0;i<MAX_STATIONS;i++){
		for(int j=0;j<MAX_STATIONS;j++){
			generated_packets[i][j]=0;
			received_packets[i][j]=0;
		}
	}

	for(int i=0;i<MAX_STATIONS;i++){
		for(int j=0;j<MAX_PORTS;j++){
			received_data_packets[i][j]=0;
			received_bpdu_packets[i][j]=0;
			forwarded_data_packets[i][j]=0;
			forwarded_bpdu_packets[i][j]=0;

			backup_lost_data_packets[i][j]=0;
			backup_lost_bpdu_packets[i][j]=0;

			queue_lost_data_packets[i][j]=0;
			queue_lost_bpdu_packets[i][j]=0;

			utilization_data_packets[i][j]=0.0;
			utilization_bpdu_packets[i][j]=0.0;
		}
	}
}

#line 23 "Network.cc"

#line 27 "Network.cc"
compcxx_Network_26::compcxx_Network_26(){					
	
	n_stations = 100;
	n_sinks = n_stations;
	n_bridges = 19;
	
	station.SetSize(n_stations);
	bridge.SetSize(n_bridges);
	sink.SetSize(n_sinks);

	start_stats.p_compcxx_parent=this /*connect start_stats.to_component,*/;
}


#line 40 "Network.cc"
void compcxx_Network_26 :: Start(){
	printf("\nStart!\n\n");
	start_stats.Set(9); 
}


#line 45 "Network.cc"
void compcxx_Network_26 :: Stop(){
	printf("\nEnd of simulation!\n\n");
	stats.print_global_stats();
	stats.print_station_stats(n_stations);
	stats.print_bridge_port_stats(n_bridges,N_PORTS);
}


#line 52 "Network.cc"
void compcxx_Network_26 :: start_stats_collection(trigger_t& t){
	stats.init_stats();
}


#line 56 "Network.cc"
void compcxx_Network_26 :: Setup(const char *name){	

	printf("\nStart Network Creation\n\n");

	StationsOutOfTopology = 0;

	
	for(int n=0; n < n_stations; n++){
		station[n].Setup("station", n);
	}

	
	for(int n=0; n < n_bridges; n++){
		bridge[n].Setup("bridge",n);	
	}
	
	
	for(int n=0; n < n_sinks; n++){
		sink[n].Setup("sink",n);
	}

	

	for(int i=0; i < n_bridges; i++){
		for(int j=0; j < n_bridges; j++)
			 cac_info[i].AdjacentCells[j] = -1;

	}

	

	
	bridge[0].out1_f.Connect(bridge[1],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out1,bridge[1].in1*/;	
	bridge[1].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in1) /*connect bridge[1].out1,bridge[0].in1*/;

	bridge[0].out2_f.Connect(bridge[2],(compcxx_component::Bridge_out2_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out2,bridge[2].in1*/;	
	bridge[2].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in2) /*connect bridge[2].out1,bridge[0].in2*/;

	bridge[0].out3_f.Connect(bridge[3],(compcxx_component::Bridge_out3_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out3,bridge[3].in1*/;	
	bridge[3].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in3) /*connect bridge[3].out1,bridge[0].in3*/;

	bridge[0].out4_f.Connect(bridge[4],(compcxx_component::Bridge_out4_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out4,bridge[4].in1*/;	
	bridge[4].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in4) /*connect bridge[4].out1,bridge[0].in4*/;

	bridge[0].out5_f.Connect(bridge[5],(compcxx_component::Bridge_out5_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out5,bridge[5].in1*/;	
	bridge[5].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in5) /*connect bridge[5].out1,bridge[0].in5*/;

	bridge[0].out6_f.Connect(bridge[6],(compcxx_component::Bridge_out6_f_t)&compcxx_Bridge_21::in1) /*connect bridge[0].out6,bridge[6].in1*/;	
	bridge[6].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in6) /*connect bridge[6].out1,bridge[0].in6*/;	

	for(int i=0; i<=5; i++){ 	
		 cac_info[0].AdjacentCells[i] = i+1;
		 cac_info[i+1].AdjacentCells[0] = 0;		

	}

	for(int b=0; b<6; b++){	

 		bridge[b+1].out2_f.Connect(bridge[((b+1)%6)+1],(compcxx_component::Bridge_out2_f_t)&compcxx_Bridge_21::in3) /*connect bridge[b+1].out2,bridge[((b+1)%6)+1].in3*/;
		bridge[((b+1)%6)+1].out3_f.Connect(bridge[b+1],(compcxx_component::Bridge_out3_f_t)&compcxx_Bridge_21::in2) /*connect bridge[((b+1)%6)+1].out3,bridge[b+1].in2*/;

		 cac_info[b+1].AdjacentCells[1] = ((b+1)%6)+1;
		 cac_info[((b+1)%6)+1].AdjacentCells[2] = b+1;

	}

	for(int b=7; b<18; b++){	

 		bridge[b].out2_f.Connect(bridge[b+1],(compcxx_component::Bridge_out2_f_t)&compcxx_Bridge_21::in3) /*connect bridge[b].out2,bridge[b+1].in3*/;
		bridge[b+1].out3_f.Connect(bridge[b],(compcxx_component::Bridge_out3_f_t)&compcxx_Bridge_21::in2) /*connect bridge[b+1].out3,bridge[b].in2*/;

		cac_info[b].AdjacentCells[1] = b+1;
		cac_info[b+1].AdjacentCells[2] = b;

	}

	bridge[18].out2_f.Connect(bridge[7],(compcxx_component::Bridge_out2_f_t)&compcxx_Bridge_21::in3) /*connect bridge[18].out2,bridge[7].in3*/;
	bridge[7].out3_f.Connect(bridge[8],(compcxx_component::Bridge_out3_f_t)&compcxx_Bridge_21::in2) /*connect bridge[7].out3,bridge[8].in2*/;		

	cac_info[18].AdjacentCells[1] = 7;
	cac_info[7].AdjacentCells[2] = 18;

	int count_b=8;
	for(int b=2; b<=6; b++){	

 		bridge[b].out4_f.Connect(bridge[count_b],(compcxx_component::Bridge_out4_f_t)&compcxx_Bridge_21::in4) /*connect bridge[b].out4,bridge[count_b].in4*/;
		bridge[count_b].out4_f.Connect(bridge[b],(compcxx_component::Bridge_out4_f_t)&compcxx_Bridge_21::in4) /*connect bridge[count_b].out4,bridge[b].in4*/;

		cac_info[b].AdjacentCells[3] = count_b;
		cac_info[count_b].AdjacentCells[3] = b;

		count_b++;
 		bridge[b].out5_f.Connect(bridge[count_b],(compcxx_component::Bridge_out5_f_t)&compcxx_Bridge_21::in1) /*connect bridge[b].out5,bridge[count_b].in1*/;
		bridge[count_b].out1_f.Connect(bridge[b],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in5) /*connect bridge[count_b].out1,bridge[b].in5*/;

		cac_info[b].AdjacentCells[4] = count_b;
		cac_info[count_b].AdjacentCells[0] = b;

		count_b++;
 		bridge[b].out6_f.Connect(bridge[count_b],(compcxx_component::Bridge_out6_f_t)&compcxx_Bridge_21::in1) /*connect bridge[b].out6,bridge[count_b].in1*/;
		bridge[count_b].out1_f.Connect(bridge[b],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in6) /*connect bridge[count_b].out1,bridge[b].in6*/;

		cac_info[b].AdjacentCells[5] = count_b;
		cac_info[count_b].AdjacentCells[0] = b;

	}
	bridge[1].out4_f.Connect(bridge[18],(compcxx_component::Bridge_out4_f_t)&compcxx_Bridge_21::in4) /*connect bridge[1].out4,bridge[18].in4*/;
	bridge[18].out4_f.Connect(bridge[1],(compcxx_component::Bridge_out4_f_t)&compcxx_Bridge_21::in4) /*connect bridge[18].out4,bridge[1].in4*/;

	cac_info[1].AdjacentCells[3] = 18;
	cac_info[18].AdjacentCells[3] = 1;

	bridge[1].out5_f.Connect(bridge[7],(compcxx_component::Bridge_out5_f_t)&compcxx_Bridge_21::in1) /*connect bridge[1].out5,bridge[7].in1*/;
	bridge[7].out1_f.Connect(bridge[0],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in5) /*connect bridge[7].out1,bridge[0].in5*/;

	cac_info[1].AdjacentCells[4] = 7;
	cac_info[7].AdjacentCells[0] = 1;

	bridge[1].out6_f.Connect(bridge[8],(compcxx_component::Bridge_out6_f_t)&compcxx_Bridge_21::in1) /*connect bridge[1].out6,bridge[8].in1*/;
	bridge[8].out1_f.Connect(bridge[1],(compcxx_component::Bridge_out1_f_t)&compcxx_Bridge_21::in6) /*connect bridge[8].out1,bridge[1].in6*/;

	cac_info[1].AdjacentCells[5] = 8;
	cac_info[8].AdjacentCells[0] = 1;

	for(int i=0; i <=6; i++)
		cac_info[i].numAdjacentCells = 6;
	for(int i=7; i <=18; i++)
		if (i%2 == 0) 
			cac_info[i].numAdjacentCells = 4;
		else
			cac_info[i].numAdjacentCells = 3;

	

	if(PRINT_CAC == 1){
		for(int i=0; i < n_bridges; i++){
			for(int j=0; j < cac_info[i].numAdjacentCells; j++)
				printf("Cell %d adjacent %d\n", i, cac_info[i].AdjacentCells[j]);
			printf("\n");

		}
	}

	
	for(int s=0; s<n_stations ; s++){
		for(int b=0; b<n_bridges ; b++){
			station[s].sourceOut_f.Connect(bridge[b],(compcxx_component::Station_sourceOut_f_t)&compcxx_Bridge_21::in0) /*connect station[s].sourceOut,bridge[b].in0*/;
			bridge[b].out0_f.Connect(sink[s],(compcxx_component::Bridge_out0_f_t)&compcxx_Sink_22::in) /*connect bridge[b].out0,sink[s].in*/;
		}
	}

	
	for(int s=0; s<n_stations ; s++){
		for(int b=0; b<n_bridges ; b++){
			station[s].flowGenOut_f.Connect(bridge[b],(compcxx_component::Station_flowGenOut_f_t)&compcxx_Bridge_21::cac_in) /*connect station[s].flowGenOut,bridge[b].cac_in*/;
			bridge[b].cac_out_f.Connect(station[s],(compcxx_component::Bridge_cac_out_f_t)&compcxx_Station_20::flowGenIn) /*connect bridge[b].cac_out,station[s].flowGenIn*/;
		}
	}

	
	for(int i=7; i<n_bridges; i++){ 
		bridge[i].out5_f.Connect(empty,(compcxx_component::Bridge_out5_f_t)&compcxx_Empty_24::in) /*connect bridge[i].out5,empty.in*/;
		bridge[i].out6_f.Connect(empty,(compcxx_component::Bridge_out6_f_t)&compcxx_Empty_24::in) /*connect bridge[i].out6,empty.in*/;
	}
	bridge[7].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[7].out4,empty.in*/;
	bridge[9].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[9].out4,empty.in*/;
	bridge[11].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[11].out4,empty.in*/;
	bridge[13].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[13].out4,empty.in*/;
	bridge[15].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[15].out4,empty.in*/;
	bridge[17].out4_f.Connect(empty,(compcxx_component::Bridge_out4_f_t)&compcxx_Empty_24::in) /*connect bridge[17].out4,empty.in*/;

	
	printf("End Network Setup\n\n");
	
}int main(int argc, char* argv[]){

	compcxx_Network_26 network;
	network.Seed=1;
	network.StopTime=100000;
	network.Setup("Network");
	network.Run();
	return(0);
};
