#ifndef __LOCAL_H_
#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <pthread.h>  
#include <errno.h>
#include <sys/msg.h>

#define NUMBER_OF_LINES 10                //number of Palestinian officers at each border
#define INITIAL_WORKERS_IN_LINE 10           //ratio of Palestinians out of 100 of the passengers
#define JOR_PASSENGERS_RATIO 35           //ratio of Jordanians out of 100 of the passengers
#define FRN_PASSENGERS_RATIO 15           //ratio of Foreigners out of 100 of the passengers
#define MAX_HALL_THRESHOLD 50             //maximum amount of people in the hall that we can handle
#define MIN_HALL_THRESHOLD 30             //minimum number that if we got to or below we can allow more people to enter the hall
#define PAL_PROCESSING_TIME 4             //time to process Palestinian papers
#define JOR_PROCESSING_TIME 3             //time to process Jordanian papers
#define FRN_PROCESSING_TIME 5             //time to process Foreigners papers
#define PATIENCE_RANGE(a) ((a) ? 20 : 15) //period of time that if exceeded the process will exit.
// #define min(a, b) ((a) < (b) ? (a) : (b))
#define TOTAL_ENTITIES 5
#define MAX_MSG_SIZE 17
//colors
#define red "\033[0;31m"   /* 0 -> normal ;  31 -> red */
#define cyan "\033[1;36m"  /* 1 -> bold ;  36 -> cyan */
#define green "\033[4;32m" /* 4 -> underline ;  32 -> green */
#define blue "\033[9;34m"  /* 9 -> strike ;  34 -> blue */

#define black "\033[0;30m"
#define brown "\033[0;33m"
#define magenta "\033[0;35m"
#define gray "\033[1;37m"

#define none "\033[0m" /* to flush the previous property */

//the time needed by the storage employee to go to storage room and come back
extern int carton_box_delivery_time;
//the time needed by the storage workers to fill one box in the truck
extern int storage_truck_filling_time;  
//the threshold where the storage area will stop accepting more boxes
extern int storage_area_max_threshold;    
//the threshold where the storage area will start accepting boxes
extern int storage_area_min_threshold;
//how many trucks do we have
extern int num_of_trucks;
//number of loading employees
extern int num_of_loading_employees;
//how many boxes can each truck have
extern int truck_capacity;
//how much time will each truck take to go and come back  
extern int truck_trip_time;      
//the Salary of the CEO
extern int salary_ceo;
//the salary of the HR
extern int salary_hr;
//the Salary fo the Technical Team
extern int salaray_technical;
//the Salary of the Storage Room
extern int salary_storage; 
//the Salary of the truck Loading Employees
extern int salary_loading;
//the Salary of the Truck Drivers 
extern int salary_drivers;
//the Salary of the Extra Employees
extern int salary_extra;   
//how much does it cost us to manufacture each laptop 
extern int laptop_manufacturing_cost;   
//how much does it cost to sell each laptop
extern int laptop_selling_cost;  
//the factory gains - all expenses
extern int factory_profit;       
//the min profit threshold at which we will begin firing employees 
extern int profit_min_threshold;  
//the max profit threshold at which we will begin hiring more employees
extern int profit_max_threshold; 
//the maximum profit we aim to reach before ending the simulation
extern int max_gain_threshold;     
//the maximum number of people we aim to suspend before we end the simulation
extern int percentage_suspend_threshold;    
//the range at which we will give random times for each line
extern int line_time_range[2];
// the random time for each line
extern int lines_working_times[10];
//the number of workers to work in each line
extern int num_of_active_lines;
//the number of laptop boxes in the storage room
extern int num_of_boxes_in_storage_room;
//message queue
extern int q_id[10];
//which line to suspend
extern int suspend_line[10];
//the number of boxes inside the current truck
extern int current_num_of_boxes_inside_current_truck;
//the semaphores needed between the threads
//the mutexes that are needed to coordinate the work between workers in each line
extern pthread_mutex_t line_mutex[10][10];
//mutext for the storage room
extern pthread_mutex_t storage_room_mutex;
//semaphores for sequential workers
extern pthread_mutex_t sequential_mutexes[10][6];
//semaphores for condition signals
extern pthread_mutex_t conditional_mutexes[10][6];
//condition variables for sequential workers
extern pthread_cond_t sequential_cond[10][6];
//condition variable
extern pthread_cond_t cond_serial_to_random;
//the 10 main threads 
extern pthread_t main_threads[NUMBER_OF_LINES];
//the condition for storage workers to start working
extern pthread_cond_t storage_workers_cond;
//the mutex for the storage workers to start working
extern pthread_mutex_t storage_workers_mutex;
//the condition for box workers to start working
extern pthread_cond_t box_worker_cond;
//the mutex for the box workers to start working
extern pthread_mutex_t box_worker_mutex;
//now making variable sized condition variables array for trucks
extern pthread_cond_t *trucks_cond;
//the mutex for the box workers to start working
extern pthread_mutex_t *trucks_mutex;
//the condition variable that tells the loading workers that there is a truck to fill
extern pthread_cond_t fill_boxes_in_truck_cond;
//the mutex for the condition variable that tells storage workers which truch to fill
extern pthread_mutex_t fill_boxes_in_truck_mutex;
//the lock for which truck should we be filling
extern pthread_mutex_t current_filling_truck;
//the condition variable that tells the loading workers that there is a truck to fill
extern pthread_cond_t is_there_a_truck_cond;
//the mutex for the condition variable that tells storage workers which truch to fill
extern pthread_mutex_t is_there_a_truck_mutex;
//the mutex for calculating the profit
extern pthread_mutex_t profit_mutex;
//the mutex for the condition variable for the HR
extern pthread_mutex_t hr_mutex;
//the condition variable for the HR
extern pthread_cond_t hr_cond;
//the mutex for the condition variable for the ceo
extern pthread_mutex_t ceo_mutex;
//the condition variable for the ceo
extern pthread_cond_t ceo_cond;

typedef struct mesg_buffer
{
  long mesg_type;
  int mesg_text[5];
} message;

// set the values for each of the variables 
void set_values(int);
//the function where the sequential workers will work
void *sequential_function(void *);
//main thread function for serial workers
void* serial_workers_main_thread_function(void *);
//function for unordered workers
void* unordered_function(void *);
//main thread function for inordered workers
void* unordered_workers_main_thread_function(void *);
//storage worker
void *storage_worker_function(void *);
//loading worker
void *loading_workers_function(void *);
//the function that organizes the storage workers
void *loading_workers_main_function(void *);
//the function that handles truck threads
void* trucks_function(void*);
//the function for the HR
void* hr_function(void*);
//the function for the ceo
void* ceo_function(void*);
//function that is responsible for each line
void* lines_function(void*);

#endif
