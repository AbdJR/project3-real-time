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

enum
{
  PAL,
  JOR,
  FRN
};
enum
{
  TRUE,
  FALSE
};

//the time needed by the storage employee to go to storage room and come back
extern int carton_box_delivery_time;  
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
extern int num_of_workers_in_line;
//the semaphores needed between the threads
//the mutexes that are needed to coordinate the work between workers in each line
extern pthread_mutex_t line_mutex[10];
//the number of laptop boxes in the storage room
extern int num_of_boxes_in_storage_room;
//mutext for the storage room
extern pthread_mutex_t storage_room_mutex;
//semaphores for sequential workers
extern pthread_mutex_t sequential_mutexes[6];
//condition variable
extern pthread_cond_t cond_serial_to_random;
//message queue
extern int q_id;
//the 10 main threads 
extern pthread_t main_threads[NUMBER_OF_LINES];

union semun
{
  int val;
  struct semid_ds *buf;
  ushort *array;
};



// typedef struct passenger_t
// {
//   int pid;
//   unsigned char nationality;         //Palestinian 0, Jordanian 1, Foreign 3+
//   unsigned short has_passport;       //to check wether we have the passports or not
//   unsigned short expiray_date_day;   //at which day does the current passport expire, only short required for all fields here
//   unsigned short expiray_date_month; //at which month does the current passport expire
//   unsigned short expiray_date_year;  //at which year does the current passport expire
//   // unsigned int passport_id;       //the id of this specific passoport
//   unsigned short tolerance; //for how long can this person wait in line
//   //used for stopping the officers when hall gets filled later
//   unsigned short my_officer;
// } passenger_t;

// typedef struct officer_t
// {
//   unsigned short processing_time; //the amount of time taken by officer to handle passengers
//   unsigned short type;            //which crossing point does this officer work at (PAL/JOR or FRN)
//   unsigned short crossing_point;  //at which crossing point will this officer serve at
// } officer_t;

// typedef struct crossing_point_t
// {
//   struct officer_t officers[OFFICERS_IN_BORDER]; //We have NUM_OF_OFFICERS amount of Palestinian officers in each border
//   unsigned short type;                           //type of crossing point (PAL/JOR or FRN)
// } crossing_point_t;

// typedef struct bus_t
// {
//   unsigned short max_capacity;    //how many passengers can the bus handle each trip
//   unsigned short trip_time;       //the time it takes to go to the jordanian side and come back
//   unsigned int num_of_passengers; //current number of passengers
// } bus_t;

// typedef struct hall_t
// {
//   unsigned int max_threshold; //how much can we have in the hall before stopping operations
//   unsigned int min_threshold; //if we reached the max, how much do we have to have in order to resume operations

// } hall_t;

typedef struct mesg_buffer
{
  long mesg_type;
  int mesg_text[5];
} message;

struct sembuf acquire = {0, -1, SEM_UNDO},
              release = {0, 1, SEM_UNDO};
//               acquire_all = {0, -1 * TOTAL_ENTITIES, SEM_UNDO},
//               release_all = {0, TOTAL_ENTITIES, SEM_UNDO};

// extern passenger_t *PASSENGERS;
// extern officer_t *OFFICERS;
// extern crossing_point_t *CROSSING_POINTS;
// extern bus_t *BUSSES;
// extern hall_t HALL;

// set the values for each of the variables 
void set_values(int);
void handle_sigusr1(int);
void free_all();
//the function where the sequential workers will work
void *sequential_function(void *);
//main thread function for serial workers
void* serial_workers_main_thread_function(void *);
//main thread function for inordered workers
void* unordered_workers_main_thread_function(void *);
//storage worker
void *storage_worker_function(void *);
//loading worker
void *loading_workers_function(void *);
//the function where the parallel workers will work
// void *unordered_function(void *);
// passenger_t create_passenger(int);
// officer_t create_officer();
// crossing_point_t create_crossing_point(int, int, int *, int);
// void initialise_semaphores(key_t, union semun, ushort);
// bus_t create_bus(int);
// char *get_info(int);
// int find_length(char *);
// //set the current date for officers to know how to check
// void set_date();
// void sit_in_hall(passenger_t, int, int *, int *, int *, int *, int);
// void go_to_bus(passenger_t, int, int *, int *, int[]);
// void increment_pass(int, int *);
// void increment_reject(int, int *);
// void increment_return(int, int *);
// int kill_the_process(int, int, int);
#endif
