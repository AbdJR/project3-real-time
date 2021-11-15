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

int main(int argc, char *argv[])
{
    int i, j, k, l; //dummy variablles for counting and iterating through loops
    set_values(0);

    return 0;
}

void set_values(int has_file)
{
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
    }
}