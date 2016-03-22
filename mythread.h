/*
 * mythread.h -- interface of user threads library
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <sys/types.h>
#include "myqueue.h"
#include "futex.h"

typedef int mythread_key_t;

typedef struct mythread_attr {    /* thread attributes */
	int stacksize;                  /* stack size in bytes */
	void* stackbase;                /* pointer to bottom of stack (high address) */
} mythread_attr_t;

typedef struct mythread {         /* thread control block */
  pid_t tid;
  void * (*start_func)(void*);    /* thread_func to be called */
  void *arg;                      /* thread_arg arguments of thread_func */
  mythread_queue_t joinq;         /* Q of threads waiting for my return */
  void *returnValue;              /* Pointer to the detached function's return value */
  struct futex futex;
  int state;                      /* state of execution */
  mythread_attr_t attr;
} *mythread_t;

/*
 * mythread_self - thread id of running thread
 */
mythread_t mythread_self(void);

/*
 * mythread_create - prepares context of new_thread_ID as start_func(arg),
 * attr is ignored right now.
 * Threads are activated (run) according to the number of available LWPs
 * or are marked as ready.
 */
int mythread_create(mythread_t *new_thread_ID,
		    mythread_attr_t *attr,
		    void * (*start_func)(void *),
		    void *arg);

/*
 * mythread_yield - switch from running thread to the next ready one
 */
int mythread_yield(void);

/*
 * mythread_join - suspend calling thread if target_thread has not finished,
 * enqueue on the join Q of the target thread, then dispatch ready thread;
 * once target_thread finishes, it activates the calling thread / marks it
 * as ready.
 */
int mythread_join(mythread_t target_thread, void **status);

/*
 * mythread_exit - exit thread, awakes joiners on return
 * from thread_func and dequeue itself from run Q before dispatching run->next
 */
void mythread_exit(void *retval);

int mythread_attr_init(mythread_attr_t *attr);
int mythread_attr_destroy(mythread_attr_t *attr);
int mythread_attr_getstack(const mythread_attr_t *attr,
			   void **stackaddr,
			   size_t *stacksize);
int mythread_attr_setstack(mythread_attr_t *attr, void *stackaddr,
			   size_t stacksize);
int mythread_attr_getstacksize(const mythread_attr_t *attr, 
			       size_t *stacksize);
int mythread_attr_setstacksize(mythread_attr_t *attr, size_t stacksize);


/*
 * ===================================================================================
 * 						DO NOT MODIFY ABOVE THIS LINE
 * ===================================================================================
 */

void resumeNext(mythread_t thread);
int create_idle_thread();

#define RETURN_OK 1
#define NO_MYTHREAD_ATTR 0
#define NO_STACK_ADDRESS 0
#define NO_STACK_SIZE 0
#define INVALID_HANDLE 0
#define ERROR_CREATING_THREAD 0
#define ERROR_ALLOCATING_MEM 0

#define STATE_NEW 0
#define STATE_READY 1
#define STATE_SUSPENDED 2
#define STATE_BLOCKED 3
#define STATE_RUNNING 4
#define STATE_FINISHED 5
/*How Long Idle thread should wait for other threads to join before terminating itself*/
#define MAX_WAIT_TIME_IDLE 5

#endif /* MYTHREAD_H */
