#include "mythread.h"
#include "myqueue.h"
#include <unistd.h>
#include <signal.h>

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

/*Queue maintaining threads*/
static mythread_queue_t *mythread_queue = (mythread_queue_t*)malloc(sizeof(mythread_queue_t));
/*idle thread's pid*/
static pid_t idle_id;

/*
 * Initilize attr with default values
 */
int mythread_attr_init(mythread_attr_t *attr){
	if(attr==NULL)
	{
		attr = (mythread_attr_t*)malloc(sizeof(mythread_attr_t));
		if(attr==NULL)
			return ERROR_ALLOCATING_MEM;
	}
	attr->stacksize = 0;
	attr->stackbase = NULL;
	return RETURN_OK;
}
/*
 * Resets mythread_attr
 */
int mythread_attr_destroy(mythread_attr_t *attr){
	attr->stacksize = 0;
	attr->stackbase = NULL;
	return RETURN_OK;
}
/*
 * Returns stack size and base pointer
 */
int mythread_attr_getstack(const mythread_attr_t *attr, void **stackaddr, size_t *stacksize){
	if (attr==NULL)
		return NO_MYTHREAD_ATTR;
	if(stackaddr==NULL)
		return NO_STACK_ADDRESS;
	if(stacksize==NULL)
		return NO_STACK_SIZE;
	*stackaddr = attr->stackbase-attr->stacksize;
	*stacksize = attr->stacksize;
	return RETURN_OK;
}
/*
 * sets stack size and base pointer
 */
int mythread_attr_setstack(mythread_attr_t *attr, void *stackaddr, size_t stacksize){
	if (attr==NULL)
		return NO_MYTHREAD_ATTR;
	if(stackaddr==NULL)
		return NO_STACK_ADDRESS;
	if(stacksize<=0)
		return NO_STACK_SIZE;
	attr->stackbase = stackaddr+stacksize;
	attr->stacksize = stacksize;
	return RETURN_OK;
}
/*
 * Returns stack size
 */
int mythread_attr_getstacksize(const mythread_attr_t *attr, size_t *stacksize){
	if (attr==NULL)
		return NO_MYTHREAD_ATTR;
	if(stacksize==NULL)
		return NO_STACK_SIZE;
	*stacksize = attr->stacksize;
	return RETURN_OK;
}
/*
 * Sets stack size
 */
int mythread_attr_setstacksize(mythread_attr_t *attr, size_t stacksize){
	if (attr==NULL)
		return NO_MYTHREAD_ATTR;
	if(stacksize<=0)
		return NO_STACK_SIZE;
	attr->stacksize = stacksize;
	return RETURN_OK;
}

/*
 * returns thread id of current running thread
 */
static pid_t gettid(void) {
	return (pid_t) syscall(SYS_gettid);
}
/*
 * returns mythred_t object for current running thread (thread from which this method is called)
 */
mythread_t mythread_self(){
	return getThreadFromQueue(gettid());
}
/*
 * Idle method, scans if there are any threads in the queue.
 * If yes, it yields to next thread, else it keeps on checking
 */
void *idle(void *arg) {
	mythread_t* thread = (mythread_t*)arg;
	int freeCycleCount = 0;
	while(1){
		//sleep(1);
		if(mythread_queue->start->next==mythread_queue->start){
			// Can be removed... just added for display purpose
			sleep(1);
			write(1,"Idle Thread...\n",15);
			freeCycleCount++;
			if(freeCycleCount==MAX_WAIT_TIME_IDLE){
				write(1,"No New Thread Added...\nExiting Idle Thread",42);
				break;
			}
		}
		else{
			/*
			 * There is threads in queue waiting to be scheduled
			 */
			freeCycleCount = 0;
			mythread_yield();
		}
	}
}

/*
 * Wrapper method...
 * Calls futex down initially, so that thread can be kept in queue as suspended
 */
int wrapper(void *arg) {
	mythread_t* thread = (mythread_t*)arg;
	struct threadData *data = (struct threadData*)((*thread)->arg);
	int idlet = (*thread)->tid==idle_id?1:0;
	futex_init(&((*thread)->futex),0);
	(*thread)->state = STATE_NEW;
	if(!idlet)
		futex_down(&((*thread)->futex));
	(*thread)->start_func((*thread)->arg);
	mythread_exit(NULL);
}

/*
 * This method creates idle thread...
 */
int mythread_create_idle(mythread_t *new_thread_ID, mythread_attr_t *attr, void * (*start_func)(void *), void *arg){
	*new_thread_ID = (mythread_t)malloc(sizeof(mythread_t));
	void* new_stack;
	void* stackbase = NULL;
	size_t stackSize = SIGSTKSZ;
	/*
	 * If attr is defined, get stack size and stack addr from attr
	 *
	 */
	if(attr){
		if(attr->stacksize>0)
			stackSize = attr->stacksize;
		else
			return NO_STACK_SIZE;
		if(attr->stackbase)
			stackbase = attr->stackbase;
		(*new_thread_ID)->attr = *attr;
	}
	/*allocate memory for stack if necessary*/
	if(stackbase==NULL){
		new_stack = malloc(stackSize);
		stackbase = new_stack+stackSize;
	}
	if(new_stack == 0||stackbase == 0){
		return ERROR_CREATING_THREAD;
	}
	if(!attr){
		mythread_attr_t attr;
		attr.stackbase = stackbase;
		attr.stacksize = stackSize;
		(*new_thread_ID)->attr = attr;
	}
	(*new_thread_ID)->start_func = start_func;
	(*new_thread_ID)->arg = arg;
	int tid;
	/*add thread to queue of threads*/
	addThreadToQueue(*new_thread_ID);
	/*create new thread*/
	if(((*new_thread_ID)->tid = tid = idle_id = clone(&wrapper, stackbase-sizeof(sigset_t), SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, new_thread_ID))==-1)
	{
		printf("Could Not create thread...");
		exit(1);
	}

	return RETURN_OK;
}

int mythread_create(mythread_t *new_thread_ID, mythread_attr_t *attr, void * (*start_func)(void *), void *arg){
	*new_thread_ID = (mythread_t)malloc(sizeof(mythread_t));
	void* new_stack;
	void* stackbase = NULL;
	size_t stackSize = SIGSTKSZ;
	/*
	 * If attr is defined, get stack size and stack addr from attr
	 *
	 */
	if(attr){
		if(attr->stacksize>0)
			stackSize = attr->stacksize;
		else
			return NO_STACK_SIZE;
		if(attr->stackbase)
			stackbase = attr->stackbase;
	}
	if(stackbase==NULL){
		new_stack = malloc(stackSize);
		stackbase = new_stack+stackSize;
	}
	if(new_stack == 0||stackbase == 0){
		return ERROR_CREATING_THREAD;
	}
	(*new_thread_ID)->attr.stackbase = stackbase;
	(*new_thread_ID)->attr.stacksize = stackSize;

	(*new_thread_ID)->start_func = start_func;
	(*new_thread_ID)->arg = arg;
	/*add thread to queue of threads*/
	addThreadToQueue(*new_thread_ID);
	/*create new thread*/
	int tid;
	if(((*new_thread_ID)->tid = tid = clone(&wrapper, stackbase-sizeof(sigset_t), CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, new_thread_ID))==-1)
	{
		printf("Could Not create thread...");
		exit(1);
	}
	return RETURN_OK;
}

/*
 * mythread_yield - switch from running thread to the next ready one
 */


int mythread_yield(){
	mythread_t thread = mythread_self();
	thread->state = STATE_READY;
	resumeNext(thread);
	return RETURN_OK;
}
/*
 * resumes next available thread in queue
 */
void resumeNext(mythread_t thread){
	//get next thread from queue
	mythread_t nextThread = getNext();
	/*if no other thread in queue, return*/
	if(nextThread==NULL){
		return;
	}
	/*
	 * if there is other thread in queue and it is new or ready
	 * resume next thread's execution and suspend current thread's execution
	 */
	if(nextThread->state==STATE_READY||nextThread->state==STATE_NEW){
		nextThread->state = STATE_RUNNING;
		futex_up(&(nextThread->futex));
		futex_down(&(thread->futex));
		return;
	}
	/*
	 * if there is other thread in queue and it is new or ready
	 * resume next thread's execution and suspend current thread's execution
	 */

	resumeNext(thread);
}

int mythread_join(mythread_t target_thread, void **status)
{
	waitpid(target_thread->tid, 0, 0 );
	return RETURN_OK;
}
/*
 * mythread_exit - exit thread, awakes joiners on return
 * from thread_func and dequeue itself from run Q before dispatching run->next
 */
void mythread_exit(void *retval){
	mythread_t thread = mythread_self();
	thread->state = STATE_FINISHED;
	futex_up(&(mythread_queue->start->thread->futex));
	if(idle_id==thread->tid)
		mythread_queue->start=NULL;
	else
		removeThreadFromQueue(thread);
	free(thread->attr.stackbase-thread->attr.stacksize);
	return;
}


/*
 * add a thread to queue
 */



void addThreadToQueue(struct mythread* thread){
	if(mythread_queue->start==NULL){
		mythread_queue->last = mythread_queue->current = mythread_queue->start = (threadQueue*)malloc(sizeof(threadQueue));
		(mythread_queue->start)->thread = thread;
		(mythread_queue->start)->next = mythread_queue->start;
		(mythread_queue->start)->prev = mythread_queue->start;

	}
	else
	{
		threadQueue *temp;
		temp = (threadQueue*)malloc(sizeof(threadQueue));
		temp->thread = thread;
		temp->prev = mythread_queue->last;
		temp->next = mythread_queue->start;
		(mythread_queue->last)->next = temp;
		(mythread_queue->start)->prev = temp;
		mythread_queue->last = temp;
	}
}
/*
 * Remove a thread from queue
 */

void removeThreadFromQueue(struct mythread* thread){
	if(mythread_queue->start==NULL){
		return;
	}
	threadQueue *temp;
	temp = mythread_queue->start;
	while(temp->thread!=thread&&temp!=mythread_queue->last){
		temp = temp->next;
	}
	if(temp->thread==thread){
		if(temp==mythread_queue->start && temp==mythread_queue->last){
			mythread_queue->start = mythread_queue->last = mythread_queue->current = NULL;
			free(temp);
			return;
		}
		else if(temp==mythread_queue->last)
			mythread_queue->last = temp->prev;
		else if(temp==mythread_queue->start)
			mythread_queue->start = temp->next;
		mythread_queue->current = temp->next;
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
		free(temp);
	}
}

/*
 * get a thread from queue based of its tid
 *
 */

struct mythread* getThreadFromQueue(pid_t tid){
	if(mythread_queue->start==NULL){
		return NULL;
	}
	threadQueue *temp;
	temp = mythread_queue->start;
	while(temp->thread->tid!=tid&&temp!=mythread_queue->last){
		temp = temp->next;
	}
	if(temp->thread->tid==tid){
		mythread_queue->current = temp->next;
		return temp->thread;

	}
	else
		return NULL;
}
/*
 * get next thread from queue
 */
struct mythread* getNext(){
	struct mythread *thread = mythread_queue->current->thread;
	mythread_queue->current = mythread_queue->current->next;
	return thread;
}

/*
 * Get last thread from queue
 */
struct mythread* getPrevious(){
	mythread_queue->current = mythread_queue->current->prev;
	struct mythread* thread = mythread_queue->current->thread;
	return thread;
}
