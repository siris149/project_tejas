/****************************************************************************************************************
Desc :

		Each node in graph represented by process.
		Each process maintains a graph structure and a queue.
		
		Details of current node and adjacent node obtained from user.
		
Assumption : static graph.
			constant number of nodes.
			nodes can go down in which case all edge lengths made 0 in graph.
			Addition of new nodes not possible. 
		
TODO : create timers and send status to adjacent nodes when timer expires.		
*****************************************************************************************************************/

#include<cstdio>
#include<cstdlib>
#include<semaphore.h>
#include<pthread.h>
#include<mqueue.h>
#inlcude<queue>

#define MAXNODES 50  //static graph - num of nodes const and known
#define INF 10000
#define MAX_SIZE 30    //queue size

using namespace std;

// definitions

struct adjnode        // representation of each adj node
{
	int dest;
	int cost;
};

struct adjlist {                    // structure to maintain adj list for each node -- adjacency list
	int v;
	adjnode nodes[MAX_NODES];
};


struct edge             // representation of a edge
{
	int sor;
	int dest;
	int cost;

};

struct edgelist                   //edgelist representation
{
	int e;
	edge edges[MAX_NODES];
};

struct queuebuffer    //buffer to be used in queue
{
	//buffertype type;
	int sending_node;
	adjlist alist[MAX_NODES];	

};

/***************************************************************************************************************/
//global variables
adjlist alist[MAX_NODES] = {0};    // adjacency list -- array of all adj lists
int total_nodes;

char *queuename[MAX_NODES];   // store the queuenames of all nodes

sem_t s;                 // to protect access to alist
edgelist mst;

int node;              //current node num
int process_alive = 0;


/***************************************************************************************************************/
//forward declarations
class Compare;
void configure();
void addAdj();
void shortest();
void mst();
void events_thread(void *param);
int updateadjlist(int node, int adj, int cost);
int addedgetomst(int s, int d);
int updategraph();
int send_updates_adjlist(int node);

/***************************************************************************************************************/
int main()
{
	application();
	return 0;
}
/***************************************************************************************************************/

int application()
{
	int choice;
	
	
	printf("MENU\n"):
	printf("1.Configure node details \n2. Add adj node details \n3.find shortest path \n4.MST \n5.Exit\n");
	
		printf("Enter choice:"):
	scanf("%d", &choice);
	
	switch(choice)
	{
		case 1:
				configure();
				break;
		case 2 :
				if(process_alive ==1)
					addAdj();
					break;
		case 3:
				if(process_alive ==1)
					shortest();
					break;
		case 4:
				if(process_alive ==1)
					mst();
					break;
		case 5:
				process_alive = 0;
				printf("Bye\n");
				exit();
				break;
		default:
				
	}
	
}
/***************************************************************************************************************/
// gets current node and queuename from user, initalizes graph, queue and sem
// seperate thread is created which monitors events for other processes(nodes)

//TODO : create a timer for each process and the status to adj nodes when timer expires and reset timer.
//TODO : seperate thread whith monitors status from adj nodes and updates graph according and share details to other processes.

void configure()
{	
	char queue[20];
	pthread_t events_thread;
	mqd_t mqdesc;
    struct mq_attr *attr=(struct mq_attr *)malloc(sizeof(struct mq_attr));

	 
    printf("Enter node identifier:");
	scanf("%d", &node);
	printf("Enter queue name:);         // TODO : other way, store in a shared file and retrieve
	scanf("%d", queue);
	
	attr->mq_flags=0;
   attr->mq_maxmsg=sizeof(queuebuffer) +1;     
   attr->mq_msgsize=MAX_SIZE;                 // queue size
	mqdesc=mq_open(queue,O_RDWR|O_CREAT,0664,attr);          // create the queue for current node
	alist[node].v =0;    //initialize adj list count to node
	
	//TODO : write node# queuename to file
	sem_init(&s, 0, 1);   //initiate sem with 1
	
	//send_updates_adjlist(int node);           // send updated graph to all adj nodes
	
	pthread_create(&events_thread, NULL, processing_thrd ,NULL);
	// create_timer();
	//update_status();
	
	pthread_join(&events_thread);
	sem_destroy(&s);
	mq_close(mqdesc);	
	
}
/***************************************************************************************************************/
// gets the details of neibouring nodes from user and updates the graph
// sends the updated graph to all neighbouring nodes.

void addAdj()
{
	int nodes;
	printf("Enter no of adj nodes:");
	scanf("%d", &nodes);
	
	for(int i=0; i< nodes; i++)
	{
		printf("Enter cost of adj node identifier and cost of a node:");
	    scanf("%d %d", &adj, &cost);
		
		updateadjlist(node, adj, cost);           
		//TODO : take the queue name and add to array
	
	}
	
}
/***************************************************************************************************************/

void shortest()  //dikstras algo
{
	int s, d;
	enum lbl { permanent, tentative };
	
	printf("Enter source:");
	scanf("%d", &s);
	
	printf('Enter destination:");
	scanf("%d", &d);
	
	int path[MAX_NODES];
	int weight[MAX_NODES];
	int predecessor[MAX_NODES];
	lbl label[MAX_NODES];
	
	int node;
	int i, min;	
	
	for(int i=0; i<total_nodes; i++)
	{
		label[i] = lbl::tentative;
		weight[i] = INF;
		predecessor[i] = -1;
		
	}
		
	weight[s] = 0;
	node =s;
	
	sem_wait(s1); // lock throughout process to avoid updation in between	
	
	while(node!= d)             // iterate till destination is found
	{ 
        for(int i =0; i < alist[node].v; i++)   // iterate through the adjacency list of vertex represented by node
		{			
			if(label[i] == lbl::tentative)                 // vertex not visited
			{
				cost = alist[node].adjnode[i].cost;
				if(weight[i] < weight[node] + cost)
				{
					weight[i] = weight[node] + cost;
					predecessor[i] = node;
				}
			}
		}
		min = INF;
		for(int i =0 ; i< total_nodes; i++)      //find the vertex with least weight
		{
			if(weight[i] < min)
			{
				min = weight[i];
				node = i;				
			}
		}
		
		label[node] = lbl::permanent;             // update the node as permanent	    
	}		
	sem_post(s1); //release the lock
	
	i =0;
	while(node != s)
	{
		path[i++] = node;               // store the path in array from dest to source obtained from predecessor array
		node = predecessor[node];
	}
	
	 //path stored in rev order
	
	printf("Shortest Path from %d to %d :\n", s, d);
	for(int j = i-1; i>=0; i--)
		printf("%d ", path[j]);
	
}
/***************************************************************************************************************/
	
void mst()    //prims algo
{	
	int r;
		
	printf("Enter the starting node:");
	scanf("%d", &r);
	
	mst.e =0 ;   //nodes in mst is 0 intitally
	
	int s[MAX_NODES];         //s[i] =1 if node present in set
	priority_queue<edge, std::vector<edge>, Compare> minheap;
	struct edge *e;
	int vertex, sor, dest;
	
	for(int i=0; i<total_nodes; i++)
	{
		s[i] = 0; 
	}
	
	s[r] = 1;          // add strating node to set
	

	for(int i =0; i< alist[r].v; i++)      //insert all adj element of r to heap
	{
		e = new edge;
		e->sor = r;
		e->dest = alist[r].nodes[i].dest;
		e->cost = alist[r].nodes[i].cost;
		minheap.push(e);
	}
	
	while(!minheap.empty())
	{
		do {
			e = minheap.top();
			sor = e->sor;
			dest = e->dest
		} while(s[sor] != 1 && s[dest] !=1);
		
		
		addedgetomst(sor, dest);                 // add the min eedge obtained above to mst
		
		if(s[sor] !=1)
			vertex = sor;
		else
			vertex = dest;						// obtain the vertex of edge such that vertex not in set
		
		int index;
		for(int i =0; i<alist[vertex].v; i++)   // check for all adj node from e which are not already in s
		{
			index = alist[vertex].nodes[i];
			if( s[index] == 0)
			{
				e = new edge;
				e->sor = alist[vertex].nodes[i].sor;
				e->dest = alist[vertex].nodes[i].dest;
				e->cost = alist[vertex].nodes[i].cost;
				minheap.push(e);
			}	
		}
		
		s[vertex] = 1;                   //add the obtained vertex to set
		minheap.pop()
	}
}
/***************************************************************************************************************/

void events_thread(void *param)
{
	int rec_bytes = 0;
	mqd_t mqdesc;
	unsigned prio=10;
	
	queuebuffer *buffer;
	
	mqdesc=mq_open(queue,O_WRONLY);
	
	while(process_alive ==1)
	{
		if(rec_bytes != 0)
		{
			rec_bytes=mq_receive(mqdesc,buffer,sizeof(queuebuffer),&prio);
			
			// read the graph details and update all edges whose len is 0.
			
			//updategraph(buffer);
		}	
	}
}	
/***************************************************************************************************************/

int updateadjlist(int node, int adj, int cost)
{
	sem_wait(&s);
	int count = alist[node].v;
	
	alist[node].nodes[count].dest = adj;
	alist[node].nodes[count].cost = cost;
	
	alist[node].v++;
	sem_post(&s);
	
	return 0;

}

/***************************************************************************************************************/

int addedgetomst(int s, int d)
{
	mst.edges[e].sor = s;
	mst.edges[e].dest = d;
	
	mst.e++;	

}
/***************************************************************************************************************/

int updategraph()
{
	
}
/***************************************************************************************************************/
	
int send_updates_adjlist(int node)
{
	string queue;
	int sen_status;
	   mqd_t mqdesc;
   unsigned prio=10;
   
	for(int i=0; i< alist[node].v, i++)
	{
		queue = queuename[i];              // TODO :
		mqdesc=mq_open(queue,O_WRONLY);
	    sen_status=mq_send(mqdesc,buffer,size,prio);
        if(sen_status==-1)
			return 0;
		mq_close(mqdesc);    //TODO : check return value
	}	
}	
/***************************************************************************************************************/

class Compare
{
public:
    bool operator() (edge e1, edge e2)
	{
		return e1.cost > e2.cost;
	}
};

/**********************************************END**************************************************************/





 