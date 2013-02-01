
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Station/../FlowGen/FlowGen.h"



#line 35 "Station/../FlowGen/FlowGen.h"
;


#line 42 "Station/../FlowGen/FlowGen.h"
;


#line 45 "Station/../FlowGen/FlowGen.h"
;


#line 56 "Station/../FlowGen/FlowGen.h"
;


#line 75 "Station/../FlowGen/FlowGen.h"
;


#line 96 "Station/../FlowGen/FlowGen.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Station/../Source/Source.h"



#line 31 "Station/../Source/Source.h"
;


#line 35 "Station/../Source/Source.h"
;


#line 38 "Station/../Source/Source.h"
;


#line 52 "Station/../Source/Source.h"
;


#line 65 "Station/../Source/Source.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Station/../Mobility/Mobility.h"



#line 26 "Station/../Mobility/Mobility.h"
;



#line 44 "Station/../Mobility/Mobility.h"
;

#line 3 "Station/Station.h"



#line 49 "Station/Station.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Sink/Sink.h"



#line 19 "Sink/Sink.h"
;


#line 23 "Sink/Sink.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Bridge/../Scheduler/../Fifo/Fifo.h"



#line 25 "Bridge/../Scheduler/../Fifo/Fifo.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 3 "Bridge/../Scheduler/Scheduler.h"



#line 48 "Bridge/../Scheduler/Scheduler.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Bridge/../Bridging/Bridging.h"



#line 19 "Bridge/../Bridging/Bridging.h"
;


#line 24 "Bridge/../Bridging/Bridging.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Bridge/../Driver/Driver.h"



#line 23 "Bridge/../Driver/Driver.h"
;

#line 26 "Bridge/../Driver/Driver.h"
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

#define CellBW 2e6 		//BW of the Cells -> consider equal to all cells
#define Tobs 2			//Observation time, timer to change cell or not
#define ProbHandoff 10		//Probability to change to another cell (10%)

#define FlowRate 64e3		//Rate of each flow

struct PACKET{

	
	int source;
	int destination;
	int flowId;
	int seqNum;
	int type;		
	int bit_length;		

	
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

struct CAC_INFO{
	
	int CACId;		
	int ActiveFlows;	
	int AdjacentCells;	
	double BW;			
	int N; 			

};

CAC_INFO cac_info[1];

#endif 


#line 1 "Bridge/../CAC/CAC.h"



#line 24 "Bridge/../CAC/CAC.h"
;


#line 33 "Bridge/../CAC/CAC.h"
;


#line 36 "Bridge/../CAC/CAC.h"
;


#line 63 "Bridge/../CAC/CAC.h"
;


#line 4 "Bridge/Bridge.h"



#line 78 "Bridge/Bridge.h"
;
#line 5 "Network.cc"



#line 69 "Network.cc"
;

#include "compcxx_Network.h"

#line 3 "Bridge/../Bridging/Bridging.h"
class compcxx_Bridging_12 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in(PACKET& p);	

		class my_Bridging_out_f_t:public compcxx_functor<Bridging_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};compcxx_array<my_Bridging_out_f_t > out;/*outportvoid out(PACKET& p)*/;

		compcxx_Bridging_12(); 
		virtual ~compcxx_Bridging_12(){}; 
		void Setup(const char *, int ){};

	private:
};

class compcxx_Bridge_17;
#line 3 "Bridge/../CAC/CAC.h"
class compcxx_CAC_15 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in(PACKET& request);
		/*outport void out(PACKET& response)*/;

		compcxx_CAC_15(){}; 
		virtual ~compcxx_CAC_15(){}; 
		void Setup(const char *, int);
		void Start();
		void Stop();

	private:
		int CACNumber;	

public:compcxx_Bridge_17* p_compcxx_parent;};

class compcxx_Bridge_17;
#line 3 "Bridge/../Driver/Driver.h"
class compcxx_Driver_14 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in0(PACKET& p);	
		/*inport */inline void in1(PACKET& p);	
		
		

		/*outport void out0(PACKET& p)*/;
		/*outport void out1(PACKET& p)*/;

		compcxx_Driver_14(){}; 
		virtual ~compcxx_Driver_14(){}; 
		void Setup(const char *, int ){};

	private:
public:compcxx_Bridge_17* p_compcxx_parent;};

class compcxx_Server_11;
#line 3 "Bridge/../Scheduler/../Fifo/Fifo.h"
class compcxx_Fifo_10 : public compcxx_component, public TypeII{
  public:
	compcxx_Fifo_10(){};
    virtual ~compcxx_Fifo_10(){};
	void Setup(const char *, int);
    void Start();

    unsigned int queue_length;

    /*inport */inline void in(PACKET& p);	
    /*outport void out(PACKET& p)*/;		
    /*inport */inline void next();			

  private:
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

class compcxx_Fifo_10;class compcxx_Scheduler_13;
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
public:compcxx_Fifo_10* p_compcxx_Fifo_10;public:compcxx_Scheduler_13* p_compcxx_parent;};


#line 5 "Bridge/../Scheduler/Scheduler.h"
class compcxx_Scheduler_13 : public compcxx_component, public TypeII{
	public: 
		/*inport */inline void in(PACKET& p);			
		class my_Scheduler_out_f_t:public compcxx_functor<Scheduler_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Scheduler_out_f_t out_f;/*outport void out(PACKET& p)*/;				
		/*inport */inline void to_out(PACKET& p);		
														
														
														
														
		
		compcxx_Fifo_10 fifo;
		compcxx_Server_11 server;

		compcxx_Scheduler_13(); 
		virtual ~compcxx_Scheduler_13(){}; 
		void Setup(const char *, int);

	private:
		int SchedulerNumber;
};


#line 6 "Bridge/Bridge.h"
class compcxx_Bridge_17 : public compcxx_component, public TypeII{
	public: 
		/*inport */inline void in(PACKET& p);		
		class my_Bridge_out_f_t:public compcxx_functor<Bridge_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};compcxx_array<my_Bridge_out_f_t > out;/*outportvoid out(PACKET& p)*/;			
		/*inport */inline void to_out0(PACKET& p);		
		/*inport */inline void to_out1(PACKET& p);		
		
		
		/*inport */inline void cac_in(PACKET& p);		
		class my_Bridge_cac_out_f_t:public compcxx_functor<Bridge_cac_out_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Bridge_cac_out_f_t cac_out_f;/*outport void cac_out(PACKET& p)*/;		
		/*inport */inline void to_cac_out(PACKET& p);	

		compcxx_Bridging_12 bridging;	
		compcxx_array<compcxx_Scheduler_13 >scheduler;
		compcxx_Driver_14 driver;
		compcxx_CAC_15 cac;

		compcxx_Bridge_17();
		virtual ~compcxx_Bridge_17(){}; 
		void Setup(const char *, int);

	private:
		int BridgeNumber;
};


#line 3 "Sink/Sink.h"
class compcxx_Sink_18 : public compcxx_component, public TypeII
{
	public: 
		/*inport */inline void in(PACKET& p);

		void Setup(const char *, int);

	private:
		int SinkNumber;

};

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

class compcxx_Station_16;class compcxx_Source_6;
#line 3 "Station/../FlowGen/FlowGen.h"
class compcxx_FlowGen_7 : public compcxx_component, public TypeII{
	public: 
		/*outport void out(PACKET& request)*/;		
		/*outport void control_out(PACKET& p)*/;
		/*inport */inline void in(PACKET& response);

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
public:compcxx_Station_16* p_compcxx_parent;public:compcxx_Source_6* p_compcxx_Source_6;};

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


#line 3 "Station/../Mobility/Mobility.h"
class compcxx_Mobility_8 : public compcxx_component, public TypeII{

	public: 
		compcxx_Mobility_8();
		virtual ~compcxx_Mobility_8(){}; 
		void Setup(const char *, int);
		compcxx_Timer_5 /*<trigger_t> */ComputeNewPosition;
		/*inport */inline void timer_ComputeNewPosition(trigger_t& t);


	private:
		int StationNumber;
};

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

class compcxx_Station_16;
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

	private:
		int SourceNumber;
		int InterarrivalTime;
public:compcxx_Station_16* p_compcxx_parent;};


#line 5 "Station/Station.h"
class compcxx_Station_16 : public compcxx_component, public TypeII{
	public: 
		/*inport */inline void flowGenIn(PACKET& p);	
		class my_Station_flowGenOut_f_t:public compcxx_functor<Station_flowGenOut_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Station_flowGenOut_f_t flowGenOut_f;/*outport void flowGenOut(PACKET& p)*/;		
		class my_Station_sourceOut_f_t:public compcxx_functor<Station_sourceOut_f_t>{ public:void  operator() (PACKET& p) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(p); return (c[0]->*f[0])(p);};};my_Station_sourceOut_f_t sourceOut_f;/*outport void sourceOut(PACKET& p)*/;		
		/*inport */inline void to_flowGenOut(PACKET& p);	
		/*inport */inline void to_sourceOut(PACKET& p);	

		compcxx_Source_6 source;
		compcxx_FlowGen_7 flowgen;
		compcxx_Mobility_8 mobility;

		compcxx_Station_16();
		virtual ~compcxx_Station_16(){}; 
		void Setup(const char *, int);

	private:
		int StationNumber;
};


#line 7 "Network.cc"
class compcxx_Network_19 : public compcxx_component, public CostSimEng
{
	private: 
		int n_stations;
		int n_bridges;
		int n_sinks;

		compcxx_array<compcxx_Station_16 >station;
		compcxx_array<compcxx_Bridge_17 >bridge;
		compcxx_array<compcxx_Sink_18 >sink;

	public:
		compcxx_Network_19();
		void Setup(const char *);
		void Stop(){};
		void Start(){};
};


#line 17 "Bridge/../Bridging/Bridging.h"
compcxx_Bridging_12 :: compcxx_Bridging_12(){
	out.SetSize(2);
}
#line 21 "Bridge/../Bridging/Bridging.h"
void compcxx_Bridging_12 :: in(PACKET &packet){
	out[0](packet);
	out[1](packet);
}
#line 20 "Bridge/../CAC/CAC.h"
void compcxx_CAC_15 :: Setup(const char *name, int id)
{
	CACNumber = id;

}
#line 26 "Bridge/../CAC/CAC.h"
void compcxx_CAC_15 :: Start(){

	cac_info[CACNumber].CACId = CACNumber;	
	cac_info[CACNumber].ActiveFlows = 0;
	cac_info[CACNumber].AdjacentCells = 0;		
	cac_info[CACNumber].BW = CellBW;
	cac_info[CACNumber].N = (int)(CellBW / FlowRate);	
}
#line 35 "Bridge/../CAC/CAC.h"
void compcxx_CAC_15 :: Stop(){
}
#line 38 "Bridge/../CAC/CAC.h"
void compcxx_CAC_15 :: in(PACKET &request)
{

PACKET response;

	if (request.destination == CACNumber){

	if(request.type == 300){			
		response.source = CACNumber;
		response.destination = request.source;
	
		if (cac_info[CACNumber].ActiveFlows < cac_info[CACNumber].N){

			response.status = ACCEPT;
			cac_info[CACNumber].ActiveFlows++;
				
		}
		else
			response.status = REJECT;

		(p_compcxx_parent->to_cac_out(response));
	}
	if (request.type == 400)			
		cac_info[CACNumber].ActiveFlows--;
	}
}
#line 21 "Bridge/../Driver/Driver.h"
void compcxx_Driver_14 :: in0(PACKET &packet){
	(p_compcxx_parent->to_out0(packet));
}
#line 24 "Bridge/../Driver/Driver.h"
void compcxx_Driver_14 :: in1(PACKET &packet){
	(p_compcxx_parent->to_out1(packet));
}
#line 22 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10 :: Setup(const char *name, int id){
	FifoNumber = id;
	queue_length=100;
}
#line 27 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10 :: Start(){
	busy_server=false;					
}


#line 31 "Bridge/../Scheduler/../Fifo/Fifo.h"
void compcxx_Fifo_10::in(PACKET& packet){			
	
    if (!busy_server){					
        (p_compcxx_Server_11->in(packet));					
        busy_server=true;				
    } 
    else
	if (queue.size() < queue_length){	
		queue.push_back(packet);		
    }
	else{								
	}
    return;
}


#line 46 "Bridge/../Scheduler/../Fifo/Fifo.h"
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
	serving.Set(SimTime() + Exponential(service_time));				
	return;
}


#line 39 "Bridge/../Scheduler/../Server/Server.h"
void compcxx_Server_11 :: packet_served(trigger_t& t){
	
    (p_compcxx_parent->to_out(packet_to_serve));							
    (p_compcxx_Fifo_10->next());											
    return;
}

#line 26 "Bridge/../Scheduler/Scheduler.h"
compcxx_Scheduler_13::compcxx_Scheduler_13(){
	 /*connect ,fifo.in*/;
	fifo.p_compcxx_Server_11=&server /*connect fifo.out,server.in*/;
	server.p_compcxx_Fifo_10=&fifo /*connect server.next,fifo.next*/;
	server.p_compcxx_parent=this /*connect server.out,*/;						
}


#line 33 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_13 :: to_out(PACKET& packet){			
	
	(out_f(packet));
}


#line 38 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_13 :: in(PACKET& packet){									
	
	fifo.in(packet);
}


#line 43 "Bridge/../Scheduler/Scheduler.h"
void compcxx_Scheduler_13 :: Setup(const char *name, int id){
	SchedulerNumber = id;
	
	fifo.Setup("fifo",id);	
	server.Setup("server",id);
}
#line 31 "Bridge/Bridge.h"
compcxx_Bridge_17::compcxx_Bridge_17(){
	out.SetSize(2);
	scheduler.SetSize(2);
	
	 /*connect ,bridging.in*/;
	bridging.out[0].Connect(scheduler[0],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_13::in) /*connect bridging.out[0],scheduler[0].in*/;
	bridging.out[1].Connect(scheduler[1],(compcxx_component::Bridging_out_f_t)&compcxx_Scheduler_13::in) /*connect bridging.out[1],scheduler[1].in*/;
	scheduler[0].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_14::in0) /*connect scheduler[0].out,driver.in0*/;
	scheduler[1].out_f.Connect(driver,(compcxx_component::Scheduler_out_f_t)&compcxx_Driver_14::in1) /*connect scheduler[1].out,driver.in1*/;
	driver.p_compcxx_parent=this /*connect driver.out0,*/;
	driver.p_compcxx_parent=this /*connect driver.out1,*/;

	cac.p_compcxx_parent=this /*connect cac.out,*/;
	 /*connect ,cac.in*/;
}


#line 47 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: to_out0(PACKET& packet){			
	
	out[0](packet);
}


#line 52 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: to_out1(PACKET& packet){			
	
	out[1](packet);
}


#line 57 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: to_cac_out(PACKET& packet){
	(cac_out_f(packet));
}


#line 61 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: in(PACKET& packet){
	
	scheduler[0].in(packet);
	scheduler[1].in(packet);
}


#line 67 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: cac_in(PACKET& packet){
	cac.in(packet);
}


#line 71 "Bridge/Bridge.h"
void compcxx_Bridge_17 :: Setup(const char *name, int id){
	BridgeNumber = id;

	scheduler[0].Setup("scheduler",id);
	scheduler[1].Setup("scheduler",id);

	cac.Setup("cac",id);
}
#line 15 "Sink/Sink.h"
void compcxx_Sink_18 :: Setup(const char *name, int id)
{
	SinkNumber = id;

}
#line 21 "Sink/Sink.h"
void compcxx_Sink_18 :: in(PACKET& packet){
	printf("\t\t\t\tSink %d -> Received packet\n \t\t\t\t\t seqNum: %d from %d to %d at %f\n\n",SinkNumber,packet.seqNum,packet.source,packet.destination, SimTime());
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




#line 11 "Station/../FlowGen/FlowGen.h"

#line 12 "Station/../FlowGen/FlowGen.h"

#line 26 "Station/../FlowGen/FlowGen.h"
compcxx_FlowGen_7::compcxx_FlowGen_7(){

	interFlowRequest.p_compcxx_parent=this /*connect interFlowRequest.to_component,*/;
	FlowDuration.p_compcxx_parent=this /*connect FlowDuration.to_component,*/;
}


#line 32 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Setup(const char *name, int id){

	SourceNumber = id;
}
#line 37 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Start(){
	interFlowRequestTime = 10;	
	FlowDurationTime = 5;
	interFlowRequest.Set(0);
	
}
#line 44 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: Stop(){
}
#line 47 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: timer_interFlowRequest(trigger_t &){

	PACKET request;
	request.source = SourceNumber;
	request.destination = 0;
	request.type = 300;		
	
	(p_compcxx_parent->to_flowGenOut(request));

}
#line 58 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: timer_FlowDuration(trigger_t &){

	PACKET stoppacketGen;
	PACKET notifyend;
	
	

	stoppacketGen.begin = STOP;
	interFlowRequest.Set(interFlowRequestTime + SimTime());
	(p_compcxx_Source_6->in(stoppacketGen));	
	
	

	notifyend.source = SourceNumber;
	notifyend.destination = 0;
	notifyend.type = 400;
	(p_compcxx_parent->to_flowGenOut(notifyend));
}
#line 77 "Station/../FlowGen/FlowGen.h"
void compcxx_FlowGen_7 :: in(PACKET &response)
{
	

	PACKET startpacketGen;

	if (response.destination == SourceNumber)
		

	if(response.status == ACCEPT){

		startpacketGen.begin = START;
		FlowDuration.Set(FlowDurationTime + SimTime());
		(p_compcxx_Source_6->in(startpacketGen));

	}
	else 		

		interFlowRequest.Set(interFlowRequestTime + SimTime());
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




#line 10 "Station/../Mobility/Mobility.h"

#line 17 "Station/../Mobility/Mobility.h"
compcxx_Mobility_8::compcxx_Mobility_8(){		
	ComputeNewPosition.p_compcxx_parent=this /*connect ComputeNewPosition.to_component,*/;
}


#line 21 "Station/../Mobility/Mobility.h"
void compcxx_Mobility_8 :: Setup(const char *name, int id){

	StationNumber = id;
	ComputeNewPosition.Set(Tobs);

}
#line 29 "Station/../Mobility/Mobility.h"
void compcxx_Mobility_8 :: timer_ComputeNewPosition(trigger_t &){

int change = rand() % 101;

	printf("change: %d\n", change);

	if(change < ProbHandoff){	

		printf("Change Cell\n");

	}
	else
		printf("Continue in the same cell\n");
		
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

#line 24 "Station/../Source/Source.h"
compcxx_Source_6::compcxx_Source_6(){
	interarrival.p_compcxx_parent=this /*connect interarrival.to_component,*/;
}


#line 28 "Station/../Source/Source.h"
void compcxx_Source_6 :: Setup(const char *name, int id){
	SourceNumber = id;
	seqNum=0;
}
#line 33 "Station/../Source/Source.h"
void compcxx_Source_6 :: Start(){
	InterarrivalTime = 1;	
}
#line 37 "Station/../Source/Source.h"
void compcxx_Source_6 :: Stop(){
}
#line 40 "Station/../Source/Source.h"
void compcxx_Source_6 :: timer_interarrival(trigger_t &){

	

	PACKET packet;
	packet.source = SourceNumber;
	packet.destination = 1;
	packet.flowId = 0;
	packet.seqNum = seqNum++; 
	printf("Source %d -> Send packet\n \t seqNum: %d from %d to %d at %f\n\n",SourceNumber,packet.seqNum,packet.source,packet.destination, SimTime());
	interarrival.Set(InterarrivalTime + SimTime());
	(p_compcxx_parent->to_sourceOut(packet));
}
#line 54 "Station/../Source/Source.h"
void compcxx_Source_6 :: in(PACKET &packet)
{
	if(packet.begin == START){
		interarrival.Set(SimTime());
		printf("Source %d -> Received START packet from FlowGen at %f\n\n",SourceNumber, SimTime());
		seqNum=0;
	}
	if(packet.begin == STOP){
		interarrival.Cancel();
		printf("Source -> Received STOP packet from FlowGen at %f\n\n", SimTime());
	}
}
#line 25 "Station/Station.h"
compcxx_Station_16::compcxx_Station_16(){	
	flowgen.p_compcxx_parent=this /*connect flowgen.out,*/;
	source.p_compcxx_parent=this /*connect source.out,*/;
	 /*connect ,flowgen.in*/;
	flowgen.p_compcxx_Source_6=&source /*connect flowgen.control_out,source.in*/;
}


#line 32 "Station/Station.h"
void compcxx_Station_16 :: flowGenIn(PACKET& packet){
	flowgen.in(packet);
}


#line 36 "Station/Station.h"
void compcxx_Station_16 :: to_flowGenOut(PACKET& packet){
	(flowGenOut_f(packet));
}


#line 40 "Station/Station.h"
void compcxx_Station_16 :: to_sourceOut(PACKET& packet){
	(sourceOut_f(packet));
}


#line 44 "Station/Station.h"
void compcxx_Station_16 :: Setup(const char *name, int id){
	StationNumber = id;
	source.Setup("source",id);
	flowgen.Setup("FlowGen",id);
	mobility.Setup("Mobility",id);
}
#line 25 "Network.cc"
compcxx_Network_19::compcxx_Network_19(){							
	n_stations = 1;
	n_sinks = 4;
	n_bridges = 3;
	
	station.SetSize(n_stations);
	bridge.SetSize(n_bridges);
	sink.SetSize(n_sinks);

}


#line 36 "Network.cc"
void compcxx_Network_19 :: Setup(const char *name){	

	printf("Start Network Creation\n");

	
	for(int n=0; n < n_stations; n++){
		station[n].Setup("station", n);
	}

	
	for(int n=0; n < n_bridges; n++){
		bridge[n].Setup("bridge",n);
	}
	
	
	for(int n=0; n < n_sinks; n++){
		sink[n].Setup("sink",n);
	}

	
	station[0].flowGenOut_f.Connect(bridge[0],(compcxx_component::Station_flowGenOut_f_t)&compcxx_Bridge_17::cac_in) /*connect station[0].flowGenOut,bridge[0].cac_in*/;
	bridge[0].cac_out_f.Connect(station[0],(compcxx_component::Bridge_cac_out_f_t)&compcxx_Station_16::flowGenIn) /*connect bridge[0].cac_out,station[0].flowGenIn*/;

	station[0].sourceOut_f.Connect(bridge[0],(compcxx_component::Station_sourceOut_f_t)&compcxx_Bridge_17::in) /*connect station[0].sourceOut,bridge[0].in*/;
	bridge[0].out[0].Connect(bridge[1],(compcxx_component::Bridge_out_f_t)&compcxx_Bridge_17::in) /*connect bridge[0].out[0],bridge[1].in*/;
	bridge[0].out[1].Connect(bridge[2],(compcxx_component::Bridge_out_f_t)&compcxx_Bridge_17::in) /*connect bridge[0].out[1],bridge[2].in*/;
	bridge[1].out[0].Connect(sink[0],(compcxx_component::Bridge_out_f_t)&compcxx_Sink_18::in) /*connect bridge[1].out[0],sink[0].in*/;
	bridge[1].out[1].Connect(sink[1],(compcxx_component::Bridge_out_f_t)&compcxx_Sink_18::in) /*connect bridge[1].out[1],sink[1].in*/;
	bridge[2].out[0].Connect(sink[2],(compcxx_component::Bridge_out_f_t)&compcxx_Sink_18::in) /*connect bridge[2].out[0],sink[2].in*/;
	bridge[2].out[1].Connect(sink[3],(compcxx_component::Bridge_out_f_t)&compcxx_Sink_18::in) /*connect bridge[2].out[1],sink[3].in*/;
		
	printf("End Network Setup.\n");
	
}int main(int argc, char* argv[]){

	compcxx_Network_19 network;
	network.Seed=1;
	network.StopTime=10;
	network.Setup("Network");
	printf("Start!\n\n");
	network.Run();
	return(0);
};
