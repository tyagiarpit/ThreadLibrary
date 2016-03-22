User Level Thread Library (mythread)
=====================================

The mythread library is a simple Userlevel, non-preemptive threads library for Linux based OS.
It performs the basic thread operations Create, Yield, Join, Exit using the 


Function Details
----------------

1-	mythread_self

		This method returns the thread id of the running/ current thread.

2-	mythread_create

		This method creates a new process/thread and sets its context details
		thread_id - id of the newly created thread. Handle for the thread object.
		attr - to customize the stackbase/stacksize of the newly created process.
		start_func - The function that this thread will start executing .
		arg	- The arguments to the start function start_func.				

3-	mythread_yield

		This method stops the running of the current thread and gives the chance for next
		runnable thread the chance to run.

4-	mythread_join

		This method suspend calling thread till the target_thread has not finished,
		enqueue on the join Q of the target thread, then dispatch ready thread;
		once target_thread finishes, it activates the calling thread / marks it
		as ready.
 

5-	mythread_exit

		This method exits thread that calls the method and awakes joiners on return
		from thread_func, dequeue itself from run Q before dispatching run->next

	

6-	mythread_attr_init
		
		This method initiates the attribute of the mythread- stacksize and stackbase to default values.

7-	mythread_attr_destroy
		
		This method destroys the mythread attibute object.

8-	mythread_attr_setstack
	
		This method initiates the attribute of the mythread- stacksize and stackbase to user defined values.

9-	mythread_attr_getstack
	
		This method returns the attributes of the given mythread.

10-	mythread_attr_setstacksize
		
		This method is used to set the stack size for the mythread attribute to user defined value.

11-	mythread_attr_getstacksize  
		
		This method is used to retrieve the stack size of the given mythread attribute.

12- resumeNext
		
		This method start the execution of the next process in the Queue. 
		It ups the futex of the next process first and then it down the futex of the current executing process.


13- gettid
		
		This method returns the tid of the currently executing mythread.

14-	idle
		
		This method wil be executed by the idle thread when there is no other thread in the Queue.
		Whenever a new mythread is added to the Queue, idle yield to the new mythread.
		

15-	wrapper

		This method is the executed when a new mythread is created. It initializez the futex of the mythread and puts it in a new state.

16-	mythread_create_idle

		This method creates the idle mythread, puts this mythread in the Queue. 

17-	mythread_create

		This method creates a new mythread and puts this mythread in Queue for execution.

18-	addThreadToQueue

		This is the convenience method to put the given mythread in the execution Queue.

19-	removeThreadFromQueue
		
		This is the convenience method to remove the given mythread from the execution Queue.

20-	getThreadFromQueue
		
		This is the convenience method to retrieve the mythread with given tid from the execution Queue.

21-	getNext
		
		This is the convenience method to get the next mythread in the execution queue.

22-	getPrevious

		This is the convenience method to get the previous mythread in the execution queue.


Data Structure/ Objects
------------------------

threadQueue
	- prev		  : Pointer to the previous mythread in the Queue.
	- next		  : Pointer to the next mythread in the Queue.
	- mythread	  : mythead object to be executed.

	The queue of the mythreads to be executed in FIFO order.


mythread_queue_t
	- start		  : The pointer to first myhtread in the threadQueue
	- last		  : The pointer to last myhtread in the threadQueue
	- current	  : The pointer to currently executing mythread in the threadQueue

	The wrapper for the thread queue to maintain the first, last and currently executing mythread.
	

mythread_attr 
	-	stacksize : stack size in bytes 
	-	stackbase : pointer to bottom of stack (high address)

	Thread attributes that can be set while creating the new thread.


mythread         
	-	tid
	-	start_func : Function to be called by the thread
	-	arg : arguments of start_func */
	-	joinq	: Queue of threads waiting for this thread to finish execution
	-	returnValue : Pointer to the detached function's return value
	-	futex : the system object that controls the execution state of the thread
	-	state : state of execution
	-	attr : attributes of the thread object that can be set hwile starting the thread

	This is the thread object also known as thread control block -that controls 

