
/* Helper functions for 'narrow_bridge' task.
   SPbSTU, IBKS, 2017 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "devices/timer.h"
#include <list.h>
#include "narrow-bridge.h"

// Creates threads, one thread for one car. Entry point is func
void create_vehicles(unsigned int count, thread_func* func);

// Vehicle threads entry points
void thread_normal_left();
void thread_normal_right();
void thread_emergency_left();
void thread_emergency_right();

// Calculates current number of threads
unsigned int threads_count(void);
// Helper function for threads_count
void threads_counter(struct thread *t UNUSED, void *aux);

// Wait for all vehicles threads will be terminated
void wait_threads(void);

// Vehicle entry point
void one_vehicle(enum car_priority prio, enum car_direction dir);

static unsigned int threads_count_on_start = 0;
static struct semaphore sema;
static struct lock gate;

int normal_left_car;
int normal_right_car;
int emergency_left_car;
int emergency_right_car;
void narrow_bridge_init()
{
sema_init(&sema, 2);
lock_init(&gate);
 normal_left_car = 0;
 normal_right_car = 0;
 emergency_left_car = 0;
 emergency_right_car = 0;
}
#define LR 1
#define RL 0
// Test entry point
void test_narrow_bridge(unsigned int num_vehicles_left, unsigned int num_vehicles_right, unsigned int num_emergency_left, unsigned int num_emergency_right)
{  
    printf("SITUATION: CL = %d, CR = %d, EL = %d, ER = %d\n", num_vehicles_left, num_vehicles_right,num_emergency_left,num_emergency_right);
    narrow_bridge_init();
    int direct;
	if(num_emergency_left==num_emergency_right)
	{
	if(num_vehicles_left >=  num_vehicles_right)
		{direct = LR;}
	else
		{direct = RL;}
	}
	else
	{
	if(num_emergency_left >= num_emergency_right)
		{direct = LR;}
	else
		{direct = RL;}
	}
	lock_acquire(&gate);	
        while(num_vehicles_left || num_vehicles_right || num_emergency_left || num_emergency_right)
	{
	if (sema_try_down(&sema))
	{
	if(direct == LR)
			{
			if(num_emergency_left > 0)
				{	
				thread_emergency_left();
				num_emergency_left--;
				}
			else if (num_vehicles_left > 0){
				thread_normal_left();
				num_vehicles_left--;
					}
			}

	else{	
			if(num_emergency_right > 0)
				{	
				thread_emergency_right();
				num_emergency_right--;
				}
			else if (num_vehicles_right > 0){
				thread_normal_right();
				num_vehicles_right--;
					}
			}
	   }	

	else {
		lock_release(&gate);
		thread_yield();
		lock_acquire(&gate);
		sema.value = 2;
		if(num_emergency_left==num_emergency_right){
			if(num_vehicles_left > num_vehicles_right)
			{direct = LR;}
			else
			{direct = RL;}
			}
		else
			{
			if(num_emergency_left > num_emergency_right)
			{direct = LR;}
			else
			{direct = RL;}
			}
		}
	}
lock_release(&gate);
}

void threads_counter(struct thread *t UNUSED, void *aux)
{
   unsigned int* pcnt = (aux);
   (*pcnt)++;
}

unsigned int threads_count(void)
{
    unsigned int cnt = 0;
    enum intr_level old_level = intr_disable();
    thread_foreach(threads_counter, &cnt);
    intr_set_level (old_level);
    return cnt;
}
  
void wait_threads()
{
  while(1)
  {
    if (threads_count() > threads_count_on_start)
      thread_yield();
    else
      return;
  }
}

void create_vehicles(unsigned int count, thread_func* func)
{
     unsigned int i;
     static unsigned int car_id = 0;
     for(i = 0; i < count; i++)
     {
         char name[16];
         snprintf(name, sizeof(name), "%u", ++car_id);
         thread_create(name, PRI_DEFAULT, func, 0);
     }
}

void thread_normal_left()
{
	normal_left_car++;
	char name[16];
	snprintf(name, sizeof(name), "CAR_LR (%d)\n", normal_left_car);
thread_create(name,10,one_vehicle,NULL);
}

void thread_normal_right()
{
    	normal_right_car++;
	char name[16];
	snprintf(name, sizeof(name), "CAR_RL (%d)\n", normal_right_car);
thread_create(name,10,one_vehicle,NULL);
}

void thread_emergency_left()
{
        emergency_left_car++;
	char name[16];
	snprintf(name, sizeof(name), "CAR_LE (%d)\n", emergency_left_car);
thread_create(name,20,one_vehicle,NULL);
}

void thread_emergency_right()
{
    	emergency_right_car++;
	char name[16];
	snprintf(name, sizeof(name), "CAR_RE (%d)\n", emergency_right_car);
thread_create(name,20,one_vehicle,NULL);
}

void one_vehicle(enum car_priority prio, enum car_direction dir)
{
    arrive_bridge();
    cross_bridge();
    exit_bridge();
}
                                                                
void cross_bridge()
{
    printf("Vehicle: %4s,  ticks=%4llu is crossing the bridge\n",thread_current()->name,timer_ticks ());
	
}
void arrive_bridge()
{
	printf("Vehicle %s is coming on the bridge\n", thread_current()->name);
	if(sema.value == 0) printf("Please wait, the bridge  is loaded\n");
	lock_acquire(&gate);
}
void exit_bridge()
{
	printf("Vehicle %s is leaving the bridge\n", thread_current()->name);	
	lock_release(&gate);
	sema_up(&sema);
}
