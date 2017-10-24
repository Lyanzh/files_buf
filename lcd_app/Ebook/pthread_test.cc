#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *thread_function(void *arg);

pthread_mutex_t mutex;

char message[] = "hello world";

int main(int agrc, char **agrv)
{
	int res;
	pthread_t thread;

	char *result;

	res = pthread_mutex_init(&mutex, NULL);
	if (res != 0)
		printf("pthread_mutex_init error\n");

	

	res = pthread_create(&thread, NULL, thread_function, (void *)message);
	if (res != 0)
		printf("pthread_create error\n");

	printf("waiting for thread to finish...\n");
	pthread_mutex_lock(&mutex);

	res = pthread_join(thread, (void *)&result);
	if (res != 0)
		printf("pthread_join error\n");

	printf("thread over, return %s\n", result);

	
	//sleep(1);

	printf("main function\n");

	return 0;
}

void *thread_function(void *arg)
{
	printf("thread_function %s\n", (char *)arg);
	pthread_exit("pthread exit");
}

