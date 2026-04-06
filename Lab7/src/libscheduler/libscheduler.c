/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
    int jobID;
    int arrivalTime;
    int runningTime;
    int remainingTime;
    int priority;

    int startTime;
    int lastStartTime;
    int finishTime;
} job_t;

static priqueue_t queue; // priority queue using the priority scheme defined in initialisation
static scheme_t currentScheme; // the current scheme being used

static job_t **coresArray; // array to hold cores
static int numCores; // total number of cores in the system

// used for statistics (averages)
static float totalWaitingTime = 0;
static float totalResponseTime = 0;
static float totalTurnaroundTime = 0;
static int numJobs = 0; // number of currently active jobs

// each priority function a priorityqueue should be able to use
int fcfs(const void *a, const void *b)
{
  job_t *job1 = (job_t *)a;
  job_t *job2 = (job_t *)b;

  return job1->arrivalTime - job2->arrivalTime;
}

int sjf(const void *a, const void *b)
{
  job_t *job1 = (job_t *)a;
  job_t *job2 = (job_t *)b;

  return job1->runningTime - job2->runningTime;
}

int psjf(const void *a, const void *b)
{
  job_t *job1 = (job_t *)a;
  job_t *job2 = (job_t *)b;

  return job1->remainingTime - job2->remainingTime;
}

int pri(const void *a, const void *b)
{
  job_t *job1 = (job_t *)a;
  job_t *job2 = (job_t *)b;

  int prio = job1->priority - job2->priority;
  if(prio != 0)
    return prio;

  return job1->arrivalTime - job2->arrivalTime;
}

int fifo(const void *a, const void *b)
{
  return 0;
}

// find the first idling core
int findCore()
{
  // check every core
  for(int i = 0; i < numCores; i++)
  {
    // core is idle
    if(coresArray[i] == NULL)
    {
      return i;
    }
  }

  // no cores available. return -1
  return -1;
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  numCores = cores;
  currentScheme = scheme;

  // allocate memory to job array
  coresArray = malloc(sizeof(job_t *) * numCores);
  for(int i = 0; i < numCores; i++)
    coresArray[i] = NULL; // initialize jobs to NULL

  // define which scheme to use
  // PPRI and RR use PRI and FCFS priorities respectively.
  // Their differences appear in scheduling new jobs.
  switch(scheme)
  {
    case FCFS:
      priqueue_init(&queue, fcfs); 
      break;
    case SJF:
      priqueue_init(&queue, sjf); 
      break;
    case PSJF:
      priqueue_init(&queue, psjf); 
      break;
    case PRI:
      priqueue_init(&queue, pri); 
      break;
    case PPRI:
      priqueue_init(&queue, pri); 
      break;
    case RR:
      priqueue_init(&queue, fifo); 
      break;
  }
}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  // make a new job
  job_t *job = malloc(sizeof(job_t));
  
  job->jobID = job_number;
  job->arrivalTime = time;
  job->runningTime = running_time;
  job->remainingTime = running_time;
  job->priority = priority;
  // job has not started yet
  job->startTime = -1;

  numJobs++;

  // attempt to find a core to run the job immediately
  int availableCore = findCore();
  
  // a core is available/idle
  if(availableCore != -1)
  {
    // start the job after creation
    coresArray[availableCore] = job;
    if (job->startTime == -1)
    {
      job->startTime = time;
    }
    job->lastStartTime = time;
    return availableCore;
  }
  
  // holds the core number whose job should be replaced with the new incoming job
  int replacedCore = -1;

  // preemption for PSJF
  // if a job arrives and is the new shortest job,
  // preemption is required to ensure that PSJF is followed properly
  if(currentScheme == PSJF)
  {
    // attempt to find the first violator of PSJF
    for(int i = 0; i < numCores; i++)
    {
      // the current job running at the given core's index
      job_t *running = coresArray[i];
      
      // check if the current running job should be replaced by checking the remainingTime
      if(job->remainingTime < running->remainingTime - (time - running->lastStartTime)){
        if(replacedCore == -1 ||coresArray[replacedCore]->remainingTime - (time - coresArray[replacedCore]->lastStartTime) < (running->remainingTime - (time - running->lastStartTime)))
        {
          replacedCore = i;
        }
      }
    }
  }

  // preemption for PPRI
  // if a job arrives and is the new shortest job,
  // preemption is required to ensure that PSJF is followed properly
  if(currentScheme == PPRI)
  {
    // attempt to find the first violator of PSJF
    for(int i = 0; i < numCores; i++)
    {
      // the current job running at the given core's index
      job_t *running = coresArray[i];
      
      // check if the current running job should be replaced by checking the priority
      if(job->priority < running->priority){
        if(replacedCore == -1 || coresArray[replacedCore]->priority < running->priority)
        {
          replacedCore = i;
        }
      }
    }
  }

  // one of the cores' jobs should be replaced
  if(replacedCore != -1)
  {
    // the current job running at the given core's index
    job_t *running = coresArray[replacedCore];
    
    // decrement the remainingTime of the currently running job
    running->remainingTime -= time - running->lastStartTime;
    
    // move the current job back into the 'line'
    priqueue_offer(&queue, coresArray[replacedCore]);
    
    // start the new job
    coresArray[replacedCore] = job;
    if (job->startTime == -1)
    {
      job->startTime = time;
    }
    job->lastStartTime = time;
    return replacedCore;
  }

  // no scheduling changes should be made
	priqueue_offer(&queue, job);
  return -1;
}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	// get the finished job from the cores array
  job_t *job = coresArray[core_id];

  // update the finish time
  job->finishTime = time;

  // increase statistics
  totalWaitingTime += job->finishTime - job->runningTime - job->arrivalTime;
  totalResponseTime += job->startTime - job->arrivalTime;
  totalTurnaroundTime += job->finishTime - job->arrivalTime;

  // free memory from finished job
  free(job);
  coresArray[core_id] = NULL;

  // get the next job 
  job_t *nextJob = priqueue_poll(&queue);

  // no jobs left in queue
  if(nextJob == NULL)
  {
    // continue idling
    return -1;
  }

  // insert nextJob into the freed core
  coresArray[core_id] = nextJob;
  nextJob->lastStartTime = time;

  // set the startTime if the job has not started execution yet
  if(nextJob->startTime == -1)
  {
    nextJob->startTime = time;
  }

  // job_number of the new job scheduled to run on core core_id
  return nextJob->jobID;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  // find the currently running job
  job_t *job = coresArray[core_id];

  // job is NULL. core remains idle
  if(job == NULL)
  {
    return -1;
  }

  // update remaining time
  job->remainingTime -= (time - job->lastStartTime);

  // no jobs waiting in the queue. Continue the current job
  if(priqueue_size(&queue) == 0)
  {
    job->lastStartTime = time;
    return job->jobID;
  }

  // put the current job at the back of queue
  priqueue_offer(&queue, job);

  // get next job
  job_t *nextJob = priqueue_poll(&queue);
  coresArray[core_id] = nextJob;

  // update new job's start times
  nextJob->lastStartTime = time;
  if(nextJob->startTime == -1)
    nextJob->startTime = time;

  return nextJob->jobID;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return totalWaitingTime / numJobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return totalTurnaroundTime / numJobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return totalResponseTime / numJobs;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  // destroy the queue
  priqueue_destroy(&queue);
  
  // free memory in the cores
  free(coresArray);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
