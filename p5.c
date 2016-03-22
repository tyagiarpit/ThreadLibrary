#include <pthread.h>
#include <stdio.h>

#include "mythread.h"
#include "futex.h"
#include "futex.c"
#include "myatomic.h"
#include "mythread.c"

/*Data to be passed to thread function as argument*/
struct threadData{
	int *x;
	int threadNumber;
};

/*
 * Function to be invoked by thread
 * It increments x by 10
 */
void *inc_x1(void *args)
{
	struct threadData *data = (struct threadData*)args;
	int i = 10;
	int threadNo = ((*(data->x))/i);
	char threadNoStr[] = {'0','\n','\0'};
	threadNoStr[0] = threadNo+48;
	//sleep(1);
	write(1,"Thread Number: ",15);
	write(1, threadNoStr, 2);

	while(i--){
		(*(data->x))++;
	}
	write(1,"Exiting From Thread's Method\n",29);
}

int main()
{

	int x = 0;
	printf("x: %d From Main\n", x);
	mythread_t m1;
	mythread_t m2;
	mythread_t m3;
	mythread_t m4;

	struct threadData arg1 = {&x,0};
	struct threadData arg2 = {&x,1};

	mythread_attr_t attr;

	int stackSize = 8096;

	if(!mythread_attr_init(&attr)){
		printf("Error while initializing mythread_attr");
		exit(1);
	}
	//Creating idle thread...
	mythread_create_idle(&m1, NULL, idle, &arg1);

	mythread_attr_setstacksize(&attr,stackSize);

	/*
	 * Create first thread
	 */
	mythread_create(&m2, NULL, inc_x1, &arg2);

	/*
	 * Create second thread
	 */

	mythread_create(&m3, NULL, inc_x1, &arg2);

	
	/*Thread with custom stack size*/
	mythread_create(&m4, &attr, inc_x1, &arg2);

	size_t size=0;

	mythread_attr_getstacksize(&attr,&size);

	if(size!=stackSize){
		write(1,"Stack size Incorrect\n",21);
	}

	/*Wait for idle thread to join*/
	mythread_join(m1,NULL);// Works Fine :)

	printf("\nBack in main thread\n\nupdated x : %d\n\n",x);

	return 0;
}
