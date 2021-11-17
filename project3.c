#include "local.h"

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
//the random time for each line
extern int lines_working_times[10];
//the number of workers to work in each line
extern int num_of_workers_in_line;
//the mutexes that are needed to coordinate the work between workers in each line
extern pthread_mutex_t line_mutex[10];
//the number of laptop boxes in the storage room
extern int num_of_boxes_in_storage_room;
//mutext for the storage room
extern pthread_mutex_t storage_room_mutex;
int main(int argc, char *argv[])
{
    int i, j, k, l; //dummy variablles for counting and iterating through loops
    set_values(0);
    int lines_pid[NUMBER_OF_LINES];
    union semun seq_un;
    seq_un.val = 1;
    int sequential_semaphores[5];
    //initialising the semaohores
    for (i = 0; i < 5; i++)
    {
        sequential_semaphores[i] = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        if (sequential_semaphores[i] < 0)
        {
            perror("semget sequential semaphores");
        }
        if (semctl(sequential_semaphores[i], 0, SETVAL, seq_un) < 0)
        {
            perror("semctl hall");
            exit(12);
        }
    }
    int q_id;
    if ((q_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT)) < 0)
    {
        perror("creating the message queue");
    }
    for (i = 0; i < NUMBER_OF_LINES; i++)
    {
        if ((lines_pid[i] = fork()) < 0)
        {
            perror("Forking a Line");
        }
        else if (lines_pid[i] == 0)
        {
            //code for Sequential lines
            if (i < 5)
            {

                while (1)
                {
                    pthread_t workers[num_of_workers_in_line];
                    if (semop(sequential_semaphores[(i + 1) % 5], &acquire, 1) < 0)
                    {
                        perror("semop acquire");
                        exit(9);
                    }
                    if (semop(sequential_semaphores[i], &acquire, 1) < 0)
                    {
                        perror("semop acquire");
                        exit(9);
                    }
                    //working with the threads now
                    for (int j = 0; j < num_of_workers_in_line; j++)
                    {
                        int *index = malloc(sizeof(int));
                        *index = i;
                        if (pthread_create(&workers[j], NULL, &sequential_function, index) != 0)
                        {
                            perror("Failed to create a Sequential line thread");
                        }
                    }
                    for (int j = 0; j < num_of_workers_in_line; j++)
                    {
                        if (pthread_join(workers[j], NULL) != 0)
                        {
                            perror("Failed to join a Sequential line thread");
                        }
                    }
                    //now we should do special action if we reached the last of the sequential line
                    if (i == 4)
                    {
                        //this will get filled as each process takes it later
                        message msg;
                        for (int i = 0; i < 5; i++)
                        {
                            msg.mesg_text[i] = 0;
                        }
                        msgsnd(q_id, &msg, sizeof(msg), IPC_NOWAIT);
                    }
                    //finished the threads work
                    if (semop(sequential_semaphores[i % 5], &release, 1) < 0)
                    {
                        perror("semop release");
                        exit(9);
                    }
                    if (semop(sequential_semaphores[(i + 1) % 5], &release, 1) < 0)
                    {
                        perror("semop release");
                        exit(9);
                    }
                }
            }
            //code for parallel lines
            else
            {

                while (1)
                {
                    pthread_t workers[num_of_workers_in_line];
                    // if (semop(sequential_semaphores[(i + 1) % 5], &acquire, 1) < 0)
                    // {
                    //     perror("semop acquire");
                    //     exit(9);
                    // }
                    // if (semop(sequential_semaphores[i], &acquire, 1) < 0)
                    // {
                    //     perror("semop acquire");
                    //     exit(9);
                    // }
                    //working with the threads now
                    message r_msg;
                    //now each process will read until one of them gets a message
                    do
                    {
                        if (msgrcv(q_id, &r_msg, sizeof(r_msg), i, IPC_NOWAIT) == ENOMSG)
                        {
                            usleep(1000);
                        }
                        //now we got a message from another line withing the last 5 lines
                        else
                        {
                            r_msg.mesg_text[i] = 1;
                            for (int j = 0; j < num_of_workers_in_line; j++)
                            {
                                int *index = malloc(sizeof(int));
                                *index = i;
                                if (pthread_create(&workers[j], NULL, &unordered_function, index) != 0)
                                {
                                    perror("Failed to create a Sequential line thread");
                                }
                            }
                            for (int j = 0; j < num_of_workers_in_line; j++)
                            {
                                if (pthread_join(workers[j], NULL) != 0)
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
                                    should_continue = 1;
                                    break;
                                }
                            }
                            //now we will send it to the next process to process it
                            if (should_continue)
                            {
                                for (int t = 0;; t = (t + 4 ^ i) % 5)
                                {
                                    if (r_msg.mesg_text[t] == 0)
                                    {
                                        r_msg.mesg_type = t;
                                        msgsnd(q_id, &r_msg, sizeof(r_msg), IPC_NOWAIT);
                                    }
                                }
                            }
                            //what if we finally covered all the steps? we should send it to the storage man
                            else
                            {
                                //we finally unveil the 6ths message type, the mailman type
                                r_msg.mesg_type = 6;
                                //won't send with non-blocking, i should wait for the storage carton to be empty again
                                msgsnd(q_id, &r_msg, sizeof(r_msg), 0);
                            }
                        }

                    } while (msgrcv(q_id, &r_msg, sizeof(r_msg), 0, IPC_NOWAIT) == ENOMSG);
                    r_msg.mesg_text[i % 5] = 1;

                    for (int j = 0; j < num_of_workers_in_line; j++)
                    {
                        int *index = malloc(sizeof(int));
                        *index = i;
                        if (pthread_create(&workers[j], NULL, &unordered_function, index) != 0)
                        {
                            perror("Failed to create a Sequential line thread");
                        }
                    }
                    for (int j = 0; j < num_of_workers_in_line; j++)
                    {
                        if (pthread_join(workers[j], NULL) != 0)
                        {
                            perror("Failed to join a Sequential line thread");
                        }
                    }
                    //check this formula, it won't give the same value of i, never. Addded 5 in the end to not confuse  with msgtype 0 that is the main one
                    //so we have message types : 0 (basic), 5,6,7,8,9
                    r_msg.mesg_type = (i - 5 + time(NULL) % 4 + 1) % 5 + 5;
                    msgsnd(q_id, &r_msg, sizeof(r_msg), IPC_NOWAIT);
                    // //finished the threads work
                    // if (semop(sequential_semaphores[i % 5], &release, 1) < 0)
                    // {
                    //     perror("semop release");
                    //     exit(9);
                    // }
                    // if (semop(sequential_semaphores[(i + 1) % 5], &release, 1) < 0)
                    // {
                    //     perror("semop release");
                    //     exit(9);
                    // }
                }
            }
            return 0;
        }
        usleep(1000);
    }

    //make as amny threads as there are loading employees + 1 for the storage worker
    pthread_t employees[num_of_loading_employees + 1];
    for (int j = 0; j < num_of_loading_employees + 1; j++)
    {
        int *index = malloc(sizeof(int));
        *index = j;
        if (j == 0)
        {
            if (pthread_create(&employees[j], NULL, &storage_worker_function, q_id) != 0)
            {
                perror("Failed to create a Storage Worker thread");
            }
        }
        if (pthread_create(&employees[j], NULL, &loading_workers_function, q_id) != 0)
        {
            perror("Failed to create a truck loading employee thread");
        }
    }
    for (int j = 0; j < num_of_loading_employees + 1; j++)
    {
        if (pthread_join(employees[j], NULL) != 0)
        {
            perror("Failed to join a storage working thread");
        }
    }

    for (i = 0; i < NUMBER_OF_LINES; i++)
    {
        waitpid(lines_pid[i], 0, 0);
    }
    //destroy all the mutexes used before exiting
    for (int k = 0; k < 10; k++)
    {
        pthread_mutex_destroy(&line_mutex[k]);
    }
    return 0;
}

void *sequential_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock(line_mutex + index);
    usleep((lines_working_times[index] / num_of_workers_in_line) * 1000000 - (num_of_workers_in_line / INITIAL_WORKERS_IN_LINE) * 100000);
    pthread_mutex_unlock(line_mutex + index);
    free(arg);
}

void *unordered_function(void *arg)
{
    int index = *(int *)arg;
    // *(int*)arg = sum //indicator to how we can set the value in the argument
    pthread_mutex_lock(line_mutex + index);
    //the -100,000 is there so that we can see the processes being faster when the number of employees has increased
    usleep((float)(lines_working_times[index] / (float)num_of_workers_in_line) * 1000000 - (num_of_workers_in_line / INITIAL_WORKERS_IN_LINE) * 100000);
    pthread_mutex_unlock(line_mutex + index);
    free(arg);
}
void *storage_worker_function(void *arg)
{
    int q_id = *(int *)arg;
    message r_msg;
    int should_exit = 0;
    int current_number_of_laptops = 0;
    while (1)
    {
        msgrcv(q_id, &r_msg, sizeof(r_msg), 6, 0);
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
    while (1)
    {
        pthread_mutex_lock(&storage_room_mutex);
        num_of_boxes_in_storage_room++;
        pthread_mutex_unlock(&storage_room_mutex);
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
        line_time_range[0] = 5;
        line_time_range[1] = 10;
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
        pthread_mutex_init(line_mutex + k, NULL);
    }
    pthread_mutex_init(&storage_room_mutex, NULL);
}