#include "local.h"

//the time needed by the storage employee to go to storage room and come back
int carton_box_delivery_time;
//the time needed by the storage workers to fill one box in the truck
int storage_truck_filling_time;
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
//the number of workers to work in each sequential line
int num_of_active_lines;
//the number of laptop boxes in the storage room
int num_of_boxes_in_storage_room;
//message queue
int q_id;
//for line 4 to make sure the other lines can still handle more laptops
int wait_line_4;
//the number of boxes inside the current truck
int current_num_of_boxes_inside_current_truck;
//the mutexes that are needed to coordinate the work between workers in each line
pthread_mutex_t line_mutex[10];
//mutext for the storage room
pthread_mutex_t storage_room_mutex;
//semaphores for sequential workers
pthread_mutex_t sequential_mutexes[6];
//semaphores for condition signals
pthread_mutex_t conditional_mutexes[6];
//condition variables for sequential workers
pthread_cond_t sequential_cond[6];
//condition variable
pthread_cond_t cond_serial_to_random;
//the 10 main threads
pthread_t main_threads[NUMBER_OF_LINES];
//the condition for storage workers to start working
pthread_cond_t storage_workers_cond = PTHREAD_COND_INITIALIZER;
//the mutex for the storage workers to start working
pthread_mutex_t storage_workers_mutex = PTHREAD_MUTEX_INITIALIZER;
//the condition for box workers to start working
pthread_cond_t box_worker_cond = PTHREAD_COND_INITIALIZER;
//the mutex for the box workers to start working
pthread_mutex_t box_worker_mutex = PTHREAD_MUTEX_INITIALIZER;
//now making variable sized condition variables array for trucks
pthread_cond_t *trucks_cond;
//the mutex for the box workers to start working
pthread_mutex_t *trucks_mutex;
//the condition variable that tells the loading workers that there is a truck to fill
pthread_cond_t fill_boxes_in_truck_cond = PTHREAD_COND_INITIALIZER;
//the mutex for the condition variable that tells storage workers which truch to fill
pthread_mutex_t fill_boxes_in_truck_mutex = PTHREAD_MUTEX_INITIALIZER;
//the lock for which truck should we be filling
pthread_mutex_t current_filling_truck = PTHREAD_MUTEX_INITIALIZER;
//the condition variable that tells the loading workers that there is a truck to fill
pthread_cond_t is_there_a_truck_cond = PTHREAD_COND_INITIALIZER;
//the mutex for the condition variable that tells storage workers which truch to fill
pthread_mutex_t is_there_a_truck_mutex = PTHREAD_MUTEX_INITIALIZER;
//the mutex for calculating the profit
pthread_mutex_t profit_mutex = PTHREAD_MUTEX_INITIALIZER;
//the mutex for the condition variable for the HR
pthread_mutex_t hr_mutex = PTHREAD_MUTEX_INITIALIZER;
//the condition variable for the HR
pthread_cond_t hr_cond = PTHREAD_COND_INITIALIZER;
//the mutex for the condition variable for the ceo
pthread_mutex_t ceo_mutex = PTHREAD_MUTEX_INITIALIZER;
//the condition variable for the ceo
pthread_cond_t ceo_cond = PTHREAD_COND_INITIALIZER;
//TODO Figure Out How To Print Everything on the same page and not create inifinte number of lines
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

    //*make as amny threads as there are loading employees + 1 for the storage worker
    pthread_t employees[NUMBER_OF_LINES + 1];

    int *q_id_m = malloc(sizeof(int));
    *q_id_m = q_id;
    //*creating the box workers that move boxes to storage rooms
    for (int j = 0; j < NUMBER_OF_LINES; j++)
    {
        if (pthread_create(&employees[j], NULL, &storage_worker_function, q_id_m) != 0)
        {
            perror("Failed to create a Storage Worker thread");
        }
    }
    //*creating the storage employees that load trucks main function
    if (pthread_create(&employees[NUMBER_OF_LINES], NULL, &loading_workers_main_function, q_id_m) != 0)
    {
        perror("Failed to create a Storage Filler thread");
    }
    //*creating Trucks
    pthread_t trucks_threads[num_of_trucks];
    for (int j = 0; j < num_of_trucks; j++)
    {
        int *index = malloc(sizeof(int));
        *index = j;
        if (pthread_create(&trucks_threads[j], NULL, &trucks_function, index) != 0)
        {
            perror("Failed to create a Storage Worker thread");
        }
    }
    //*creating the HR and the CEO
    pthread_t hr_and_ceo_threads[2];
    if (pthread_create(&hr_and_ceo_threads[0], NULL, &hr_function, NULL) != 0)
    {
        perror("Failed to create a Storage Worker thread");
    }
    if (pthread_create(&hr_and_ceo_threads[1], NULL, &ceo_function, NULL) != 0)
    {
        perror("Failed to create a Storage Worker thread");
    }

    //*JOINING
    //loading employees and storage workers
    for (int j = 0; j < NUMBER_OF_LINES + 1; j++)
    {
        if (pthread_join(employees[j], NULL) != 0)
        {
            perror("Failed to join a storage working thread");
        }
    }
    //the 10 main threads representing lines
    for (int j = 0; j < NUMBER_OF_LINES; j++)
    {
        printf("waiting for Line %d to join\n", j);
        if (pthread_join(main_threads[j], NULL) != 0)
        {
            perror("Failed to join a storage working thread");
        }
    }
    //joining trucks
    for (int j = 0; j < num_of_trucks; j++)
    {
        printf("waiting for Truck %d to join\n", j);
        if (pthread_join(trucks_threads[j], NULL) != 0)
        {
            perror("Failed to join a Truck thread");
        }
    }
    //joining the hr and ceo
    for (int j = 0; j < 2; j++)
    {
        printf("waiting for HR // CEO %d to join\n", j);
        if (pthread_join(hr_and_ceo_threads[j], NULL) != 0)
        {
            perror("Failed to join an HR // CEO thread");
        }
    }
    //*Destroying Mutexesand Conditions
    for (int k = 0; k < 10; k++)
    {
        if (k == 0)
        {
            //mutexes
            pthread_mutex_destroy(&storage_workers_mutex);
            pthread_mutex_destroy(&box_worker_mutex);
            pthread_mutex_destroy(&(*trucks_mutex));
            pthread_mutex_destroy(&fill_boxes_in_truck_mutex);
            pthread_mutex_destroy(&current_filling_truck);
            pthread_mutex_destroy(&is_there_a_truck_mutex);
            pthread_mutex_destroy(&profit_mutex);
            pthread_mutex_destroy(&hr_mutex);
            pthread_mutex_destroy(&ceo_mutex);
            pthread_mutex_destroy(&storage_room_mutex);
            //condition variables
            pthread_cond_destroy(&cond_serial_to_random);
            pthread_cond_destroy(&storage_workers_cond);
            pthread_cond_destroy(&box_worker_cond);
            pthread_cond_destroy(&(*trucks_cond));
            pthread_cond_destroy(&fill_boxes_in_truck_cond);
            pthread_cond_destroy(&is_there_a_truck_cond);
            pthread_cond_destroy(&hr_cond);
            pthread_cond_destroy(&ceo_cond);
        }
        if (k < 6)
        {
            //mutexes
            pthread_mutex_destroy(&sequential_mutexes[k]);
            pthread_mutex_destroy(&conditional_mutexes[k]);
            //condition variables
            pthread_cond_destroy(&sequential_cond[k]);
        }
        //mutexes
        pthread_mutex_destroy(&line_mutex[k]);
        //conditional variables
    }

    free(trucks_cond);
    free(trucks_mutex);
    free(q_id_m);
    return 0;
}

void *serial_workers_main_thread_function(void *arg)
{
    int i = *(int *)arg;
    //converting num_of_workers_in_line to a local variable
    int lnum_of_workers_in_line = num_of_active_lines;
    while (1)
    {
        if (lnum_of_workers_in_line != num_of_active_lines)
        {
            lnum_of_workers_in_line = num_of_active_lines;
        }
        pthread_t workers[lnum_of_workers_in_line];
        if (pthread_mutex_lock(&(sequential_mutexes[(i + 1)])) < 0)
        {
            perror("mutex lcok");
            exit(9);
        }
        if (pthread_mutex_lock(&(sequential_mutexes[i])) < 0)
        {
            perror("mutex lock");
            exit(9);
        }
        //working with the threads now
        for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
        {
            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&(workers[j]), NULL, &sequential_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }
        for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
        {
            if (pthread_join((workers[j]), NULL) != 0)
            {
                perror("Failed to join a Sequential line thread");
            }
        }

        //finished the threads work
        if (pthread_mutex_unlock(&(sequential_mutexes[i])) < 0)
        {
            perror("mutex unlock");
            exit(9);
        }
        //if i > 0
        if (i)
        {
            pthread_cond_signal(&(sequential_cond[i - 1]));
        }
        // printf("serial worker %d ,but now released lock %d\n", i,((i%5)+1));
        if (pthread_mutex_unlock(&(sequential_mutexes[(i + 1)])) < 0)
        {
            perror("mutex unlock");
            exit(9);
        }
        /*cond wait:
        * unlocks the mutex
        * waits for signal
        * locks the mutex 
        */
        //so we need to lock and unlock properly becauuse unlocking an unlocked mutex will cause undefined behaviour (see the manual)
        pthread_mutex_lock(&(conditional_mutexes[i]));
        pthread_cond_wait(&(sequential_cond[i]), &(conditional_mutexes[i]));
        pthread_mutex_unlock(&(conditional_mutexes[i]));
    }
    free(arg);
}

void *sequential_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock((line_mutex + index));
    double time = ((double)lines_working_times[index] / (double)num_of_active_lines) * 100000 -
                  (num_of_active_lines * 100000 * lines_working_times[index] - INITIAL_WORKERS_IN_LINE * 100000 * lines_working_times[index]);
    // printf("a sequential worker (%d) working, I have a working time of %lf, and total is %ld\n",index,time,(lines_working_times[index]*1000000));
    usleep(time);
    pthread_mutex_unlock((line_mutex + index));
    free(arg);
}

void *unordered_workers_main_thread_function(void *arg)
{
    int i = *(int *)arg;
    //converting this to a local variable
    int lnum_of_workers_in_line = num_of_active_lines;
    while (1)
    {
        if (lnum_of_workers_in_line != num_of_active_lines)
        {
            lnum_of_workers_in_line = num_of_active_lines;
        }

        pthread_t workers[lnum_of_workers_in_line];

        //working with the threads now
        message r_msg;
        // int is_locked;
        //now each process will read until one of them gets a message
        do
        {
            message r_msg;
            if (lnum_of_workers_in_line != num_of_active_lines)
            {
                lnum_of_workers_in_line = num_of_active_lines;
            }
            // pthread_t workers[lnum_of_workers_in_line];
            int err = msgrcv(q_id, &r_msg, sizeof(r_msg), i, IPC_NOWAIT);
            if (err == -1)
            {
                // printf("In my sleep :( %d \n",i);
                usleep(100000);
            }
            //now we got a message from another line withing the last 5 lines
            else
            {
                r_msg.mesg_text[i % 5] = 1;

                for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
                {
                    int *index = malloc(sizeof(int));
                    *index = i;
                    if (pthread_create(&(workers[j]), NULL, &unordered_function, index) != 0)
                    {

                        perror("Failed to create a Sequential line thread");
                    }
                }
                for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
                {
                    if (pthread_join((workers[j]), NULL) != 0)
                    {
                        perror("Failed to join a Sequential line thread");
                    }
                }

                //check if we reached 5 Lines working on this laptop
                int should_continue = 0;

                for (int f = 0; f < 5; f++)
                {

                    if (r_msg.mesg_text[f] == 0)
                    {
                        // printf("%d value is %d and thus we should \t\t\t continue; We are Line %d\n",f,r_msg.mesg_text[f],i);
                        should_continue = 1;
                        break;
                    }
                }
                //now we will send it to the next process to process it
                if (should_continue)
                {
                    for (int t = 0; t < 5; t++)
                    {
                        if (r_msg.mesg_text[(t + i) % 5] == 0)
                        {
                            r_msg.mesg_type = (t + i) % 5 + 5;
                            msgsnd(q_id, &r_msg, sizeof(r_msg), IPC_NOWAIT);
                            break;
                        }
                    }
                }
                //what if we finally covered all the steps? we should send it to the storage man
                else
                {
                    //we finally unveil the 6ths message type, the mailman type
                    r_msg.mesg_type = 10;
                    //won't send with non-blocking, i should wait for the storage carton to be empty again
                    msgsnd(q_id, &r_msg, sizeof(r_msg), 0);
                }
            }
        } while (pthread_mutex_trylock(&(sequential_mutexes[5])) != 0);
        if (lnum_of_workers_in_line != num_of_active_lines)
        {
            lnum_of_workers_in_line = num_of_active_lines;
        }
        for (int r = 0; r < 5; r++)
        {
            r_msg.mesg_text[r] = 0;
        }

        for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
        {
            printf("%d\t\t", lnum_of_workers_in_line);
            int *index = malloc(sizeof(int));
            *index = i;
            if (pthread_create(&(workers[j]), NULL, &unordered_function, index) != 0)
            {
                perror("Failed to create a Sequential line thread");
            }
        }
        for (int j = 0; j < sizeof(workers)/sizeof(pthread_t); j++)
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
        msgsnd(q_id, &r_msg, sizeof(r_msg), 0);
        // printf("I am Line %d and have sent the message to line %ld for them to continue working on this laptop\n", i, r_msg.mesg_type);
        pthread_cond_signal(&(sequential_cond[4]));
        //we only locked it to get out of the loop as a way to signal that we finished a laptop
        pthread_mutex_unlock(&(sequential_mutexes[5]));
    }
    free(arg);
}

void *unordered_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock(&(line_mutex[index]));
    //the second part of the equation is there so that we can see the processes being faster when the number of employees has increased,
    //it also affects the system negatively if number of workers decreased
    usleep((float)(lines_working_times[index] / (float)num_of_active_lines) * 1000000 -
           (num_of_active_lines * 100000 * lines_working_times[index] - INITIAL_WORKERS_IN_LINE * 100000 * lines_working_times[index]));
    pthread_mutex_unlock(&(line_mutex[index]));
    free(arg);
}

void *loading_workers_main_function(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&storage_workers_mutex);
        /*cond wait:
        * unlocks the mutex
        * waits for signal
        * locks the mutex 
        */
        //so we need to lock and unlock properly becauuse unlocking an unlocked mutex will cause undefined behaviour (see the manual)
        pthread_cond_wait(&storage_workers_cond, &storage_workers_mutex);
        // int *load_truck =
        printf("Got the Signal, Lets Get Those Trucks Fillin Up Boys!!\n\n\n");
        pthread_t employees[num_of_loading_employees];
        for (int j = 0; j < num_of_loading_employees; j++)
        {
            if (pthread_create(&employees[j], NULL, &loading_workers_function, NULL) != 0)
            {
                perror("Failed to create a Storage Filler thread");
            }
        }
        for (int j = 0; j < num_of_loading_employees + 1; j++)
        {
            if (pthread_join(employees[j], NULL) != 0)
            {
                perror("Failed to join a storage working thread");
            }
        }
        pthread_mutex_unlock(&storage_workers_mutex);
    }
    // free(arg);
}

void *loading_workers_function(void *arg)
{
    //TODO Utilise this Variable
    int should_exit = 0;
    // printf("loading worker fucntion\n");
    while (1)
    {
        pthread_mutex_lock(&storage_room_mutex);
        // printf("[loading_workers_function] Acquired the storage room mutex %d\n",current_num_of_boxes_inside_current_truck);
        pthread_mutex_lock(&is_there_a_truck_mutex);
        // printf("[loading_workers_function] Acquired the is there a truck mutex %d\n",current_num_of_boxes_inside_current_truck);
        usleep((long)((float)storage_truck_filling_time / (float)num_of_loading_employees) * 1000000);
        num_of_boxes_in_storage_room--;
        // printf("[loading_workers_function] taking boxes from the storage room, there remains %d \tboxes\n",num_of_boxes_in_storage_room);
        pthread_mutex_lock(&fill_boxes_in_truck_mutex);
        current_num_of_boxes_inside_current_truck++;
        // printf("[loading_workers_function] Filling the truck,Also Acquired the fill boxes in truch mutex current capacity inside the truck is %d\n",current_num_of_boxes_inside_current_truck);
        if (current_num_of_boxes_inside_current_truck == truck_capacity)
        {
            pthread_mutex_unlock(&fill_boxes_in_truck_mutex);
            // printf("[loading_workers_function] The Truck IS Full, lets send him a signal%d\n",current_num_of_boxes_inside_current_truck);
            pthread_cond_signal(&fill_boxes_in_truck_cond);
            //now wait till there is a truck that we can fill
            pthread_cond_wait(&is_there_a_truck_cond, &is_there_a_truck_mutex);
        }
        else
        {
            pthread_mutex_unlock(&fill_boxes_in_truck_mutex);
            // printf("[loading_workers_function] Unlocked the Fill Box in Truck Mutex, now anyone else can fill boxes in the truck\n");
        }
        if (num_of_boxes_in_storage_room == storage_area_min_threshold)
        {
            pthread_cond_signal(&box_worker_cond);
        }
        if (num_of_boxes_in_storage_room <= num_of_loading_employees)
        {
            pthread_mutex_unlock(&is_there_a_truck_mutex);
            pthread_mutex_unlock(&storage_room_mutex);
            break;
        }
        pthread_mutex_unlock(&is_there_a_truck_mutex);
        // printf("[loading_workers_function] Unlocked the is there a truck mutex\n");
        pthread_mutex_unlock(&storage_room_mutex);
        // printf("[loading_workers_function] Unlocked the storage room mutex\n");
    }
    // printf("Broke out of the loop since there are not many boxes left, i'm free again\n");
    // free(arg);
}

void *storage_worker_function(void *arg)
{
    int q_id = *(int *)arg;
    message r_msg;
    int should_exit = 0;
    int current_number_of_laptops = 0;
    int should_wait = 0;
    int print = 0;
    // printf("Storage worker fucntion\n");
    while (1)
    {
        // printf("in storage room waiting\n");
        msgrcv(q_id, &r_msg, sizeof(r_msg), 10, 0);
        if (print++ % 5 == 0)
            printf("[storage_worker_function] RECEIVED THE MESSAGE !!!!!!!!\n\n\n");
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
        // printf("current number of laptops is \t\t\t %d\n", current_number_of_laptops);
        if (current_number_of_laptops == 10)
        {
            if (should_wait)
            {
                should_wait = 0;
                /*cond wait:
                * unlocks the mutex
                * waits for signal
                * locks the mutex 
                */
                //so we need to lock and unlock properly becauuse unlocking an unlocked mutex will cause undefined behaviour (see the manual)
                pthread_mutex_lock(&box_worker_mutex);
                pthread_cond_wait(&box_worker_cond, &box_worker_mutex);
                pthread_mutex_unlock(&box_worker_mutex);
            }
            sleep(carton_box_delivery_time);
            current_number_of_laptops = 0;
            pthread_mutex_lock(&storage_room_mutex);
            num_of_boxes_in_storage_room++;
            printf("[storage_worker_function] \t\tnumber of boxes in storage room is %d\n\n", num_of_boxes_in_storage_room);
            if (num_of_boxes_in_storage_room >= storage_area_max_threshold - NUMBER_OF_LINES)
            {
                should_wait = 1;
                //we only need to signal one time, the rest of the times we want to wait
                //* double free or corruption (fasttop) Check This Error!!
                //* Figured the message out, it occurs when we free a variable more than one time. Had to remanage where each dynamically allocated variable is being freed
                if (num_of_boxes_in_storage_room == storage_area_max_threshold - NUMBER_OF_LINES)
                {
                    pthread_cond_signal(&storage_workers_cond);
                }
            }
            else if (num_of_boxes_in_storage_room >= truck_capacity + 2)
            {
                pthread_cond_signal(&storage_workers_cond);
            }
            pthread_mutex_unlock(&storage_room_mutex);
        }
    }
    // free(arg);
}

void *trucks_function(void *arg)
{

    while (1)
    {
        pthread_mutex_lock(&current_filling_truck);
        pthread_cond_broadcast(&is_there_a_truck_cond);
        pthread_mutex_lock(&fill_boxes_in_truck_mutex);
        if (current_num_of_boxes_inside_current_truck != truck_capacity)
        {
            printf("[trucks_function] waiting for boxes to be filled\n");
            pthread_cond_wait(&fill_boxes_in_truck_cond, &fill_boxes_in_truck_mutex);
            printf("[trucks_function] THE BOXES ARE FILLED IN THE TRUCK, LETS GET GOING\n");
        }
        current_num_of_boxes_inside_current_truck = 0;
        pthread_mutex_unlock(&fill_boxes_in_truck_mutex);
        pthread_mutex_unlock(&current_filling_truck);
        sleep(truck_trip_time);
        // pthread_mutex_lock(&profit_mutex);
        // factory_profit += truck_capacity * 10 * (laptop_selling_cost - laptop_manufacturing_cost) -
        //                   (salary_hr + salary_ceo + NUMBER_OF_LINES * num_of_workers_in_line * salaray_technical + NUMBER_OF_LINES * salary_storage +
        //                    num_of_loading_employees * salary_loading + salary_extra + salary_drivers);
        // printf("[trucks_function] The Current Profit for this round is %d\n", factory_profit);
        // pthread_mutex_unlock(&profit_mutex);
        pthread_cond_signal(&hr_cond);
    }
    free(arg);
}

void *hr_function(void *arg)
{
    // int num_of_sent_truck = 0;
    while (1)
    {
        pthread_mutex_lock(&hr_mutex);
        pthread_cond_wait(&hr_cond, &hr_mutex);
        // num_of_sent_truck++;
        pthread_mutex_unlock(&hr_mutex);
        //* now calculating the profit that is calculated once every successful truck delivery
        // num_of_sent_truck = 0;
        pthread_mutex_lock(&profit_mutex);
        factory_profit += truck_capacity * 10 * (laptop_selling_cost - laptop_manufacturing_cost) -
                          (salary_hr + salary_ceo + NUMBER_OF_LINES * num_of_active_lines * salaray_technical + NUMBER_OF_LINES * salary_storage +
                           num_of_loading_employees * salary_loading + salary_extra + salary_drivers);
        printf("[hr_function] The Total Profit is %d\n", factory_profit);
        if (factory_profit >= profit_max_threshold || factory_profit <= profit_min_threshold || factory_profit >= max_gain_threshold)
        {
            pthread_cond_signal(&ceo_cond);
        }
        pthread_mutex_unlock(&profit_mutex);
    }
    free(arg);
}

void *ceo_function(void *arg)
{
    //so we can see if we suspended more than a certain barrier
    int number_of_suspended = 0;
    while (1)
    {
        printf("CEO WAITING\n");
        pthread_mutex_lock(&ceo_mutex);
        pthread_cond_wait(&ceo_cond, &ceo_mutex);
        pthread_mutex_unlock(&ceo_mutex);
        printf("CEO WAITING2\n");

        //* now checking why we were called
        //TODO Think whether we should keep the mutex in here
        // pthread_mutex_lock(&profit_mutex);
        if (factory_profit >= profit_max_threshold)
        {
            //! ERROR, Segmentation Fault, it seems we need to aquire all mutexes before doing so
            printf("[ceo_function] The Total Profit now is %d,which is above %d, so we are going to increase the number of workers by 1 on all Lines\n", factory_profit, profit_max_threshold);

            num_of_active_lines++;

            printf("unlocked the mutex all right\n");
        }
        else if (factory_profit <= profit_min_threshold)
        {
            printf("[ceo_function] Unfortunately, The Total Profit now is %d, which is below %d, so we are going to SUSPEND one worker from each line\n", factory_profit, profit_min_threshold);

            num_of_active_lines--;

            number_of_suspended++;
            if (((float)number_of_suspended / (float)INITIAL_WORKERS_IN_LINE) >= ((float)percentage_suspend_threshold / 100.0))
            {
                //TODO end everything with a depressing failure
            }
        }
        // pthread_mutex_unlock(&profit_mutex);
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
        truck_capacity = 8;
        truck_trip_time = 7;
        salary_ceo = 2000;
        salary_hr = 1500;
        salaray_technical = 1200;
        salary_storage = 900;
        salary_loading = 900;
        salary_drivers = 1000;
        salary_extra = 800;
        laptop_manufacturing_cost = 150;
        laptop_selling_cost = 5250;
        factory_profit = 0;
        profit_min_threshold = -1000;
        profit_max_threshold = 5000;
        max_gain_threshold = 20000;
        carton_box_delivery_time = 1;
        storage_truck_filling_time = 1;
        percentage_suspend_threshold = 45;
        line_time_range[0] = 1;
        line_time_range[1] = 2;
        num_of_active_lines = INITIAL_WORKERS_IN_LINE;
        num_of_boxes_in_storage_room = 0;
    }
    current_num_of_boxes_inside_current_truck = 0;
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
    for (int i = 0; i < 6; i++)
    {
        if (pthread_mutex_init(&sequential_mutexes[i], NULL) < 0)
        {
            perror("pthread_mutex_init sequential semaphores");
        }
        if (pthread_mutex_init(&conditional_mutexes[i], NULL) < 0)
        {
            perror("pthread_mutex_init conditional semaphores");
        }
        if (pthread_cond_init(&sequential_cond[i], NULL) < 0)
        {
            perror("pthread_mutex_init sequential conditions");
        }
    }
    if ((q_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT)) < 0)
    {
        perror("creating the message queue");
    }
    trucks_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * num_of_trucks);
    trucks_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_of_trucks);
}