#include "local.h"

//the time needed by the storage employee to go to storage room and come back
int carton_box_delivery_time;
//the threshold where the storage area will stop accepting more boxes
int storage_area_max_threshold;
//the threshold where the storage area will start accepting boxes
int storage_area_min_threshold;
//how many trucks do we have
int num_of_trucks;
//number of loading employees
int num_of_loading_employees;
//how many boxes can each truck have
int truck_capacity;
//how much time will each truck take to go and come back
int truck_trip_time;
//the Salary of the CEO
int salary_ceo;
//the salary of the HR
int salary_hr;
//the Salary fo the Technical Team
int salaray_technical;
//the Salary of the Storage Room
int salary_storage;
//the Salary of the truck Loading Employees
int salary_loading;
//the Salary of the Truck Drivers
int salary_drivers;
//the Salary of the Extra Employees
int salary_extra;
//how much does it cost us to manufacture each laptop
int laptop_manufacturing_cost;
//how much does it cost to sell each laptop
int laptop_selling_cost;
//the factory gains - all expenses
int factory_profit;
//the min profit threshold at which we will begin firing employees
int profit_min_threshold;
//the max profit threshold at which we will begin hiring more employees
int profit_max_threshold;
//the maximum profit we aim to reach before ending the simulation
int max_gain_threshold;
//the maximum number of people we aim to suspend before we end the simulation
int percentage_suspend_threshold;
//the range at which we will give random times for each line
int line_time_range[2];
//the random time for each line
int lines_working_times[10];
//the number of workers to work in each line
int num_of_workers_in_line;
//the mutexes that are needed to coordinate the work between workers in each line
pthread_mutex_t line_mutex[10];
//the number of laptop boxes in the storage room
int num_of_boxes_in_storage_room;
//mutext for the storage room
pthread_mutex_t storage_room_mutex;
//semaphores for sequential workers
pthread_mutex_t sequential_mutexes[6];
//condition variable
pthread_cond_t cond_serial_to_random;
//message queue
int q_id;
//the 10 main threads 
pthread_t main_threads[NUMBER_OF_LINES];
//for line 4 to make sure the other lines can still handle more laptops
int wait_line_4;

int main(int argc, char *argv[])
{
    // int i, j, k, l; //dummy variablles for counting and iterating through loops
    set_values(0);

    for (int i = 0; i < NUMBER_OF_LINES; i++)
    {
        //code for Sequential lines
        if (i < 5)
        {

            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&main_threads[i], NULL, &serial_workers_main_thread_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }
        //code for parallel lines
        else
        {
            usleep(1000);
            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&main_threads[i], NULL, &unordered_workers_main_thread_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }

        usleep(1000);
    }
    // sleep(1000);
    //make as amny threads as there are loading employees + 1 for the storage worker
    pthread_t employees[num_of_loading_employees + 1];
    for (int j = 0; j < num_of_loading_employees + 1; j++)
    {
        int *index = malloc(sizeof(int));
        *index = j;
        if (j == 0)
        {
            if (pthread_create(&employees[j], NULL, &storage_worker_function, index) != 0)
            {
                perror("Failed to create a Storage Worker thread");
            }
        }
        else
        {
            if (pthread_create(&employees[j], NULL, &loading_workers_function, index) != 0)
            {
                perror("Failed to create a Storage Filler thread");
            }
        }
    }
    for (int j = 0; j < num_of_loading_employees + 1; j++)
    {
        if (pthread_join(employees[j], NULL) != 0)
        {
            perror("Failed to join a storage working thread");
        }
    }
    for (int j = 0; j < NUMBER_OF_LINES; j++)
    {
        printf("waiting for Line %d to join\n", j);
        if (pthread_join(main_threads[j], NULL) != 0)
        {
            perror("Failed to join a storage working thread");
        }
    }

    for (int k = 0; k < 10; k++)
    {
        pthread_mutex_destroy(&line_mutex[k]);
    }
    pthread_cond_destroy(&cond_serial_to_random);
    return 0;
}

void *sequential_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock((line_mutex+index));
    double time = ((double)lines_working_times[index] / (double)num_of_workers_in_line) * 100000 - (num_of_workers_in_line / (INITIAL_WORKERS_IN_LINE + 1)) * 100000;
    // printf("a sequential worker (%d) working, I have a working time of %lf, and total is %ld\n",index,time,(lines_working_times[index]*1000000));
    usleep(time);
    // printf("a sequential fINISHEDd worker (%d) working, we all have a working time of %d\n",index,lines_working_times[index]);
    pthread_mutex_unlock((line_mutex+index));
    free(arg);
}

void *unordered_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock(&(line_mutex[index]));
    //the -100,000 is there so that we can see the processes being faster when the number of employees has increased
    // printf("\t\tan unordered worker working %d  \t",index);
    usleep((float)(lines_working_times[index] / (float)num_of_workers_in_line) * 1000000 - (num_of_workers_in_line / INITIAL_WORKERS_IN_LINE) * 100000);
    pthread_mutex_unlock(&(line_mutex[index]));
    free(arg);
    // printf("an unordered worker ending %d  \n",index);
}
void *storage_worker_function(void *arg)
{
    int q_id = *(int *)arg;
    message r_msg;
    int should_exit = 0;
    int current_number_of_laptops = 0;
        printf("Storage worker fucntion\n");
    while (1)
    {
        msgrcv(q_id, &r_msg, sizeof(r_msg), 6, 0);
        printf("RECEIVED THE MESSAGE !!!!!!!!\n\n\n");
        //start protocol check for exit
        should_exit = 1;
        for (int s = 0; s < 5; s++)
        {
            if (r_msg.mesg_text[s] == 1)
            {
                should_exit = 0;
                break;
            }
            //if we received a message full of 0s then we should exit, but we can't just kill the thread, we beed to free the argument first
        }
        if (should_exit)
        {
            break;
        }
        current_number_of_laptops++;
        printf("current number of laptops is \t\t\t %d\n",current_number_of_laptops);
        if (current_number_of_laptops == 10)
        {
            sleep(carton_box_delivery_time);
            current_number_of_laptops = 0;
            pthread_mutex_lock(&storage_room_mutex);
            num_of_boxes_in_storage_room++;
            pthread_mutex_unlock(&storage_room_mutex);
        }
    }
    free(arg);
}
//TODO Storage Room Workers Function, + add the trucks to the system
void *loading_workers_function(void *arg)
{
    int q_id = *(int *)arg;
    message r_msg;
    int should_exit = 0;
    int current_number_of_laptops = 0;
        printf("loading worker fucntion\n");
    while (1)
    {
        pthread_mutex_lock(&storage_room_mutex);
        num_of_boxes_in_storage_room++;
        pthread_mutex_unlock(&storage_room_mutex);
    }
    free(arg);
}

void *serial_workers_main_thread_function(void *arg)
{
    int i = *(int *)arg;
    while (1)
    {
        pthread_t workers[num_of_workers_in_line];
        // printf("serial worker fucntion %d ,acquiring lock %d\n", i,((i + 1) % 5));
        
        // if (i == 4)
        // {
        //     printf("serial worker 4 now we will acquire lock 6\n");
        //     pthread_mutex_lock(&(sequential_mutexes[6]));
        // }
        if (pthread_mutex_lock(&(sequential_mutexes[(i + 1) ])) < 0)
        {
            perror("mutex lcok");
            exit(9);
        }
        // printf("serial worker %d ,but now is acquiring lock %d\n", i,i);
        if (pthread_mutex_lock(&(sequential_mutexes[i])) < 0)
        {
            perror("mutex lock");
            exit(9);
        }
        //working with the threads now
        // printf("Now that it is my turn, i (%d) will begin working on the laptop \n",i);
        for (int j = 0; j < num_of_workers_in_line; j++)
        {
            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&(workers[j]), NULL, &sequential_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }
        for (int j = 0; j < num_of_workers_in_line; j++)
        {
            if (pthread_join((workers[j]), NULL) != 0)
            {
                perror("Failed to join a Sequential line thread");
            }
        }
        //finished the threads work
        if (pthread_mutex_unlock(&(sequential_mutexes[(i + 1) ])) < 0)
        {
            perror("mutex unlock");
            exit(9);
        }
        // printf("serial worker %d ,but now released lock %d\n", i,((i%5)+1));
        if (pthread_mutex_unlock(&(sequential_mutexes[i])) < 0)
        {
            perror("mutex unlock");
            exit(9);
        }
        // printf("serial worker %d ,released lock %d\n", i,((i%5)));
        //now we should do special action if we reached the last of the sequential line to give chance for others to take the key
        if (i == 4)
        {
            printf("FINALLY \t MOVING \t TO \t NEXT \t PHASE \t\n");
            // pthread_mutex_unlock(&(sequential_mutexes[6]));
            wait_line_4 = 0;
            while(!wait_line_4);
            
        }
        // usleep(10000);
    }
    free(arg);
}

void *unordered_workers_main_thread_function(void *arg)
{
    int i = *(int *)arg;
    while (1)
    {
        printf("inordered worker fucntion %d\n", i);
        pthread_t workers[num_of_workers_in_line];
        //working with the threads now
        message r_msg;
        // int is_locked;
        //now each process will read until one of them gets a message
        do
        {
            // printf("in do...while loop, this is line %d\n\n\n",i);
            int err = msgrcv(q_id, &r_msg, sizeof(r_msg), i, IPC_NOWAIT);
            if ( err == -1)
            {
                // printf("In my sleep :( %d \n",i);
                usleep(100000);
            }
            //now we got a message from another line withing the last 5 lines
            else
            {
                r_msg.mesg_text[i%5] = 1;
                // printf("Got the message and r_msg.mesg_text[i] = %d for i = %d the error message was %d \n", r_msg.mesg_text[i],i,err);
                for (int j = 0; j < num_of_workers_in_line; j++)
                {
                    int *index = malloc(sizeof(int));
                    *index = i;
                    if (pthread_create(&(workers[j]), NULL, &unordered_function, index) != 0)
                    {
                        perror("Failed to create a Sequential line thread");
                    }
                }
                for (int j = 0; j < num_of_workers_in_line; j++)
                {
                    if (pthread_join((workers[j]), NULL) != 0)
                    {
                        perror("Failed to join a Sequential line thread");
                    }
                }
                printf("Joined all the unordered workers in thread %d\n",i);
                //check if we reached 5 Lines working on this laptop
                int should_continue = 0;
                for (int f = 0; f < 5; f++)
                {
                    if (r_msg.mesg_text[f] == 0)
                    {
                        // printf("%d value is %d and thus we should \t\t\t continue;",f,r_msg.mesg_text[f]);
                        should_continue = 1;
                        break;
                    }
                }
                printf("SHOULD CONTINUE =============================================================== %d\n",should_continue);
                //now we will send it to the next process to process it
                if (should_continue)
                {
                    for (int t = 0;; t = (t + 4 ^ i) % 5)
                    {
                        if (r_msg.mesg_text[t] == 0)
                        {
                            r_msg.mesg_type = t+5;
                            msgsnd(q_id, &r_msg, sizeof(r_msg), IPC_NOWAIT);
                        }
                    }
                }
                //what if we finally covered all the steps? we should send it to the storage man
                else
                {
                    printf("Shou;;;d \t\t Send \t\t Message \t\t%d \n",i);
                    //we finally unveil the 6ths message type, the mailman type
                    r_msg.mesg_type = 6;
                    //won't send with non-blocking, i should wait for the storage carton to be empty again
                    msgsnd(q_id, &r_msg, sizeof(r_msg), 0);

                }
            }
            // is_locked = ;
            // printf("This is the value of is_locked in process %d : %d \n",i,is_locked);
        } while (pthread_mutex_trylock(&(sequential_mutexes[5]))!= 0);
        wait_line_4 = 1;
        //we only locked it to get out of the loop as a way to signal that we finished a laptop
        pthread_mutex_unlock(&(sequential_mutexes[5]));

        for (int r = 0; r < 5; r++)
        {
            r_msg.mesg_text[r] = 0;
        }

        printf("Got a notification and r_msg.mesg_text[i\%5] = %d for worker %d\n", r_msg.mesg_text[i%5],i);
        for (int j = 0; j < num_of_workers_in_line; j++)
        {
            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&(workers[j]), NULL, &unordered_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }
        for (int j = 0; j < num_of_workers_in_line; j++)
        {
            if (pthread_join((workers[j]), NULL) != 0)
            {
                perror("Failed to join a Sequential line thread");
            }
        }
        r_msg.mesg_text[i % 5] = 1;
        //check this formula, it won't give the same value of i, never. Addded 5 in the end to not confuse  with msgtype 0 that is the main one
        //so we have message types : 0 (basic), 5,6,7,8,9
        r_msg.mesg_type = (i - 5 + time(NULL) % 4 + 1) % 5 + 5;
        msgsnd(q_id, &r_msg, sizeof(r_msg), IPC_NOWAIT);
    }
    free(arg);
}

void set_values(int has_file)
{
    srand(time(NULL));
    if (!has_file)
    {
        storage_area_max_threshold = 200;
        storage_area_min_threshold = 140;
        num_of_trucks = 4;
        num_of_loading_employees = 10;
        truck_capacity = 40;
        truck_trip_time = 7;
        salary_ceo = 2000;
        salary_hr = 1500;
        salaray_technical = 1200;
        salary_storage = 900;
        salary_loading = 900;
        salary_drivers = 1000;
        salary_extra = 800;
        laptop_manufacturing_cost = 250;
        laptop_selling_cost = 375;
        factory_profit = 0;
        profit_min_threshold = -1000;
        profit_max_threshold = 5000;
        max_gain_threshold = 20000;
        carton_box_delivery_time = 5;
        percentage_suspend_threshold = 45;
        line_time_range[0] = 1;
        line_time_range[1] = 2;
        num_of_workers_in_line = INITIAL_WORKERS_IN_LINE;
        num_of_boxes_in_storage_room = 0;
    }
    for (int k = 0; k < 10; k++)
    {
        lines_working_times[k] = (rand() % (line_time_range[1] -
                                            line_time_range[0])) +
                                 line_time_range[0];
    }
    for (int k = 0; k < 10; k++)
    {
        pthread_mutex_init(&line_mutex[k], NULL);
    }
    pthread_mutex_init(&storage_room_mutex, NULL);
    pthread_cond_init(&cond_serial_to_random, NULL);
    union semun seq_un;
    seq_un.val = 1;
    for (int i = 0; i < 6; i++)
    {
        if (pthread_mutex_init(&sequential_mutexes[i], NULL) < 0)
        {
            perror("pthread_mutex_init sequential semaphores");
        }
    }
    if ((q_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT)) < 0)
    {
        perror("creating the message queue");
    }
}