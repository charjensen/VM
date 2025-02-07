///////////////////////////////////////////////////////
// Mutex implementation
// This implementation provides MUTEX_MAXNUMBER mutex locks
// for use by user processes; the lock number is specified 
// using an 8-bit number (called key), which restricts us to 
// have up to 256 mutex variables
// DO NOT use a mutex object that has not been created; the functions
// always return in such cases
// TODO: Allow user process to create own mutex object

//Author: Charles Jensen

#include "kernel_only.h"

MUTEX mx[MUTEX_MAXNUMBER];	// the mutex locks; maximum 256 of them

/*** Initialize all mutex objects ***/
void init_mutexes() {
	int i;
	for (i=0; i<MUTEX_MAXNUMBER; i++) {
		mx[i].available = TRUE; // the mutex is available for use
		mx[i].lock_with = NULL;
		init_queue(&(mx[i].waitq));
	}
	mx[0].available = FALSE;
}

/*** Create a mutex object ***/
// At least one of the cooperating processes (typically
// the main process) should create the mutex before use
// The function returns 0 if no mutex objects are available;
// otherwise the mutex object number is returned
mutex_t mutex_create(PCB *p) {
	// TODO: see background material on what this function should do

	//start at the 2nd mx spot and loop until we find an available mutex
	unsigned char i = 1;
	while (mx[i].available != TRUE) {
		
		i++;

		//if we went through all of them and none are available return 0
		if (i > 256) return 0;

	}

	//set proper variables for chosen mutex
	mx[i].available = FALSE;
	mx[i].creator = p->pid;
	mx[i].lock_with = NULL;
	mx[i].waitq.head = 0;
	mx[i].waitq.count = 0;

	// TODO: comment the following line before you start working

	//return array index
	return i;
}

/*** Destroy a mutex with a given key ***/
// This should be called by the process who created the mutex
// using mutex_create; the function makes the mutex key available
// for use by other creators.
// Mutex is automatically destroyed if creator process dies; creator
// process should always destroy a mutex when no other process is
// holding a lock on it; otherwise the behavior is undefined
void mutex_destroy(mutex_t key, PCB *p) {
	// TODO: see background material on what this function should do

	//if the mutex we have is not created then return
	if (mx[key].available == TRUE) return;

	//if the process created the mutex then it can destroy it
	if (mx[key].creator == p->pid) mx[key].available = TRUE;

}

/*** Obtain lock on mutex ***/
// Return true if process p is able to obtain a lock on mutex
// number <key>; otherwise the process is queued and FALSE is
// returned.
// Non-recursive: if the process holding the lock tries
// to obtain the lock again, it will cause a deadlock
bool mutex_lock(mutex_t key, PCB *p) {
	// TODO: see background material on what this function should do

	//if the mutex we have is not created then return
	if (mx[key].available == TRUE) return FALSE;

	//if the mutex is not locked with a process then give the process the mutex
	if (mx[key].lock_with == NULL) {

		mx[key].lock_with = p;
		return TRUE;

	}

	//put the process in the queue and save the spot
	int ret = enqueue(&(mx[key].waitq), p);

	//update the process pcb
	p->mutex.wait_on = key;
	p->mutex.queue_index = ret;

	// TODO: comment the following line before you start working
	return FALSE;
}

/*** Release a previously obtained lock ***/
// Returns FALSE if lock is not owned by process p;
// otherwise the lock is given to a waiting process and TRUE
// is returned
bool mutex_unlock(mutex_t key, PCB *p) {
	// TODO: see background material on what this function should do

	//only unlock if the same process that has the lock is attempting to unlock
	if (mx[key].available == TRUE || mx[key].lock_with != p) return FALSE;
	else mx[key].lock_with = NULL;

	//if the queue is not empty then give a waiting process in the queue the mutex
	if (mx[key].waitq.count > 0) {
	
		PCB* process = dequeue(&(mx[key].waitq));
		mx[key].lock_with = process;
		process->mutex.wait_on = -1;
		process->state = READY;
			
	}

	// TODO: comment the following line before you start working
	return TRUE;
}

/*** Cleanup mutexes for a process ***/
void free_mutex_locks(PCB *p) {
	int i;

	for (i=1; i<MUTEX_MAXNUMBER; i++) {
		// see if process is creator of the mutex
		if (p->pid == mx[i].creator) mutex_destroy((mutex_t)i,p);
	}

	// remove from wait queue, if any
	if (p->mutex.wait_on != -1) 
		remove_queue_item(&mx[p->mutex.wait_on].waitq, p->mutex.queue_index);	
}



