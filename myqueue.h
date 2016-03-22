#include "mythread.h"
#include <stdlib.h>
#ifndef MYQUEUE_H_
#define MYQUEUE_H_

typedef struct threadQueue{
	threadQueue *prev;
	threadQueue *next;
	struct mythread* thread;
} threadQueue;


typedef struct mythread_queue_t{
	threadQueue *start;
	threadQueue *last;
	threadQueue *current;
} mythread_queue_t;

void addThreadToQueue(struct mythread* thread);
void removeThreadFromQueue(struct mythread* thread);
void removeThreadFromQueue(pid_t tid);
struct mythread* getThreadFromQueue(pid_t tid);
struct mythread* getThreadFromQueueUsingTid(pid_t tid);
struct mythread* getNext();
struct mythread* getPrevious();

#endif /* MYQUEUE_H_ */
