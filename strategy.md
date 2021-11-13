##### So some of the strategies i thought about while reading the project description:

-each manufacturing line can be a seperate process, with each process having multiple threads, the issue in here is each process knowing the PID of the next process so it can signal it.

> `pthread_sigmask()` found in man 3 can be useful to make some threads sensitive to signals.

- since steps 6-10 happen in any order, we can have them operate on the same shared memory with a mutex, and only one of them can acquire that shared memory

- in the shared memory, we can have a counter that counts up once a group of workers work on it, and once it reaches 5 we send a signal to an external thread (or process) that has a counter that increments once a group of processes end working on a laptop.

- when the storage employee (so i guess it must be a thread preferrably or a process [not preferrable]) is absent, then that means that the thread sleeps.  this means that incrementing the storage process needs a mutex and it can be incremented once we get the lock, the counter is reset to 0 when the storage employee takes the  box, and it is locked at 10 whenever we are waiting for the employee to come back.  this might mean that we will need a ==seperate mutext for the storage employee==, and other processes that increment the counter (which increments when we finish a laptop) have their own mutex.

```
//Example of what the code will be:
//when a process wants to increment number of laptops:
acquire(&workers_mutex)
increment the counter
if the counter == 10:
    acquire(&storage_mutex)
    counter -= 10
    release(&workers_mutex)
    sleep(storage worker time)
    release(&storage_mutex)
else:
    release(&workers_mutex)
//simulation of the code above:
counter = 9, we write to it:
we get the mutex
no one can write
we now have 10
we get the storage mutex
we decrement 10
we release the workers mutex
the storage worker sleeps
workers keep adding until it becomes 10 again
we try to acquire storage mutex but he is still sleeping
we wait until he comes back
the mutex gets released from the storage worker
we enter the if statement and decrement the counter by 10 and the storage worker takes it after we release the workers mutex
```
-----

- the time each step takes is calculated through: Predefined time / number of workers. Might need to use usleep and multiply the result by 1000 to make it more precise but this is the idea, since we are going to increase workers if we gained a profit.

- the salary of each step will be the salary of each employee * number of employees in that step

- the way we are going to have a serialised flow of steps, is by having a series of mutex locks (the parallel way requires only one mutex for all of them)
```
step 1 acquires mutex of step 2 and starts working
it releases the mutex once it finishes working
step 2 acqires the mutex of step 3 first, then acquires the mutex of step 2
step 2 releases the mutex of step 2 then step 3 
same for step 3 
(this isn't well calculated but we took something similar in OS)
```
>This way there is no need to know any PID of any process.

> BUT: we need to figure out how can 10 threads take the mutex? 
My solution for this is not to create a mutex, but create a binary semaphore. In here we can use it across processes and the threads will work once we acquire or release
the semaphores
----

> NOTE THAT I REMEMBERED: THERE IS A WAY TO PRINT ON THE SCREEN AND OVERWRITE EVERYTHING ELSE THAT WAS PRINTED PRIOR ON THE SCREEN

- the storage worker can increment the storage area number (which will require a third mutex) and when we reach the maximum, the same thing will happen again (much like the movie inception) so it is a nexted waiting process as the 3 ends of it depend on each other.

- the trucks analogy is similar to the bus analoy in the border control project
----
### SALARIES AND PROFIT (exiting the IT world and entering the Business world)
till now my idea is to have a the salaries of the workers subtracted from the total profit each time the truck delivers a package. of course the profit is controlled using a mutex if there are multiple processes that can modify it.

