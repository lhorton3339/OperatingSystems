#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>





unsigned short xsub1[3];


// sleep between 0.5 - 3 sec
void mySleep()
{
   int s, ms;
   struct timespec t0, t1;

   s = jrand48(xsub1) % 3;
   ms = jrand48(xsub1)  % 1000;
   if (s == 0 && ms < 500)
	ms += 500;
   t0.tv_sec = s;
   t0.tv_nsec = ms * 1000000;
   nanosleep(&t0, &t1);

}

// comment on the pthread_mutex statements to see the different effect
void *runner(void *param)
{
        int myid = *(int *)param;

	int x;
	for (int i = 0; i < 1000000; i++)
		 x = 1;

	for (int i = 0; i < 10; i++)
		{
		   int x = jrand48(xsub1) % 100;
		   printf("Thread %d,  round %d  : %d\n", myid, i, x);
		   if (x % 2)
			{
			   printf("Thread %d sleeps\n", myid);
			   mySleep();
			}
		}
          
 	pthread_exit(0);

}

int main()
{
 
   

   srand(time(0));

   xsub1[0] = (short) (rand() % 256);
   xsub1[1] = (short) (rand() % 256);
   xsub1[2] = (short) (rand() % 256);
   int a[5];
   pthread_t tid[5];
   pthread_attr_t attr;

   pthread_attr_init(&attr);


   for (int i = 0; i < 5;  i++)
	{
		a[i] = i;
		pthread_create(&tid[i], &attr, runner, &(a[i]));
	}

   

   for (int i = 0; i < 5; i++)
	pthread_join(tid[i], NULL);
 
   return(0);
}

