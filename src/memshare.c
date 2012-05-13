/* Memshare, quick and easy IPC.                                                   */
/* Copyright (C) 2012  Tommy Wiklund                                               */

/* This program is free software; you can redistribute it and/or                   */
/* modify it under the terms of the GNU General Public License                     */
/* as published by the Free Software Foundation; either version 2                  */
/* of the License, or (at your option) any later version.                          */

/* This program is distributed in the hope that it will be useful,                 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */

/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU General Public License for more details.                                    */

/* You should have received a copy of the GNU General Public License               */
/* along with this program; if not, write to the Free Software                     */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

#include "memshare.h"
#include "memshare_api.h"
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "queue.h"

static pthread_t recthread1_t, recthread2_t;

union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};

/* global pointer to the ctrl area, initialized by get_shm() */
char *shm_ctrl_ptr = NULL;
/* global mutex protecting the ctrl area */
int lock_ctrl_sem = 0;
/* internal memory view of the ctrl area */
mem_proc_entry mem_entry[NUMBER_OF_PROCS];
/* initialized flag */
int initialized = 0;
/* My own process */
char my_proc[PROC_NAME_SIZE];

int my_index, queue_index, sequence = 0;

static int create_lock(int key, int value);
static void lock(int sem);
static void unlock(int sem);
static int try_lock1(int sem);

static void init_mem_proc(void);
static void populate_mem_proc(void);
static int clear_shm(int key, int size);
static char *get_shm(int key, int size, int *mode);

static proc_entry *get_proc_at(int index);
static int clear_proc_entry(int index);
static int get_next_free_index(void);
static void populate_mem_proc_single(int index);
static int print(int level, const char *format, ...);
static int inc_sent(void);
static int inc_received(void);

callback_1 callback1 = NULL;
callback_2 callback2 = NULL;
callback_3 callback3 = NULL;
callback_data callbackdata = NULL;

/* print functions either syslog, printf of user specific */
int current_level = CH_ERROR;

static int print(int level, const char *format, ...)
{
	va_list ap;
	int retval = 0;

	/* add syslog and user specific TODO */
	va_start(ap, format);
	if (level <= current_level) {
		retval = vprintf(format, ap);
	}
	va_end(ap);
	return retval;
}

static void *recthread2(void *arg)
{
	header *hdr;
	signal *sig;
	char *msg;
	int value1;

	for (;;) {
		msg = qget(queue_index);
		hdr = (header *) msg;
		switch (hdr->msg_type) {
		case DATA:
			if (callbackdata != NULL) {
				callbackdata(hdr->proc_name,
					     (msg + SIZEOF_HEADER),
					     hdr->msg_len);
				free(msg);
			} else {
				print(CH_INFO,
				      "memshare: No callback for msg_type %d\n",
				      DATA);
			}
			break;

		case SIGNAL1:
			sig = (signal *) (msg + SIZEOF_HEADER);
			if (callback1 != NULL) {
				callback1(hdr->proc_name, sig->signal1);
				free(msg);
			} else {
				print(CH_INFO,
				      "memshare: No callback for msg_type %d\n",
				      SIGNAL1);
			}
			break;

		case SIGNAL2:
			sig = (signal *) (msg + SIZEOF_HEADER);
			if (callback2 != NULL) {
				callback2(hdr->proc_name, sig->signal1,
					  sig->signal2);
				free(msg);
			} else {
				print(CH_INFO,
				      "memshare: No callback for msg_type %d\n",
				      SIGNAL2);
			}
			break;

		case SIGNAL3:
			sig = (signal *) (msg + SIZEOF_HEADER);
			if (callback3 != NULL) {
				callback3(hdr->proc_name, sig->signal1,
					  sig->signal2, sig->signal3);
				free(msg);
			} else {
				print(CH_INFO,
				      "memshare: No callback for msg_type %d\n",
				      SIGNAL3);
			}
			break;

		default:
			print(CH_ERROR, "memshare: Illeagal msg_type %d\n",
			      hdr->msg_type);
			free(msg);
			break;
		}
	}
}

static void *recthread1(void *arg)
{
	header *hdr;
	char *msg;

	for (;;) {
		/* Add check for prio, TODO */
		print(CH_DEBUG, "recthread going to lock\n");
		lock(mem_entry[my_index].rlock);
		print(CH_DEBUG, "Entry inserted in shm for my process\n");
		hdr = (header *) mem_entry[my_index].shm;
		/* check return of allocation TODO */
		msg = malloc(hdr->msg_len + SIZEOF_HEADER);
		memcpy(msg, mem_entry[my_index].shm,
		       (hdr->msg_len + SIZEOF_HEADER));
		if (lo_qadd(queue_index, &msg)) {
			/* Failed to put in queue, msg lost */
			print(CH_ERROR,
			      "memshare: Failed to put msg in queue, msg with seq nr %d is lost!\n",
			      hdr->seq);
			free(msg);
		} else {
			/* msg concidered received */
			inc_received();
		}
		unlock(mem_entry[my_index].wlock);
	}
}

/********** ipc semaphore functions
            to be used as mutexes    ***********/
/* This function will try to create a semaphore for an index, exclusively    */
/* If it already has been created it will fail and we will open it normally  */
/* If the exclusive open succede we know we are the first process and we so  */
/* we will initialize it to work as a mutex                                  */
static int create_lock(int key, int value)
{
	/*struct sembuf op[1]; */
	union semun ctrl;
	int sem, mode = 0;

	print(CH_DEBUG, "create_lock key=%d, value=%d\n", key, value);
	if ((sem = semget(key, 1, IPC_EXCL | IPC_CREAT | 0666)) == -1) {
		/* Trying to open it exclusively failed, try normal */
		print(CH_DEBUG, "create_lock Key=%d already open\n", key);
		mode = 1;
		if ((sem = semget(key, 1, IPC_CREAT | 0666)) == -1) {
			print(CH_ERROR, "Unable to create semaphore\n");
			return 0;
		}
	}

	/*op[0].sem_num = 0;
	   op[0].sem_flg = 0; */
	ctrl.val = value;

	if (mode == 0) {
		print(CH_DEBUG, "create_lock Init sem=%d to %d\n", sem, value);
		/* Its the first time for this key, init it to work as a mutex */
		if (semctl(sem, 0, SETVAL, ctrl) == -1) {
			print(CH_ERROR,
			      "memshare: Unable to initialize semaphore\n");
			return 0;
		}
	}
	return sem;
}

static int destroy_lock(int key)
{
	union semun ctrl;
	int sem;

	print(CH_DEBUG, "destroy_lock key=%d\n", key);
	if ((sem = semget(key, 1, IPC_CREAT | 0600)) == -1) {
		print(CH_ERROR, "memshare: Unable to create semaphore\n");
		return 1;
	}
	if (semctl(sem, 0, IPC_RMID, ctrl) == -1) {
		print(CH_ERROR, "memshare: Unable to initialize semaphore\n");
		return 1;
	}
	return 0;
}

static void lock(int sem)
{
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = -1;
	op[0].sem_flg = SEM_UNDO;
	semop(sem, op, 1);
}

static void unlock(int sem)
{
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 1;
	op[0].sem_flg = 0;
	semop(sem, op, 1);
}

static void set_active(int sem)
{
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 1;
	op[0].sem_flg = SEM_UNDO;
	semop(sem, op, 1);
}

static void clear_active(int sem)
{
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = -1;
	op[0].sem_flg = 0;
	semop(sem, op, 1);
}

static int try_lock1(int sem)
{
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 0;
	op[0].sem_flg = IPC_NOWAIT;
	if (semop(sem, op, 1) == -1 && errno == EAGAIN) {
		return 1;
	}
	return 0;
}

/********** mem_proc functions **********/
static void init_mem_proc(void)
{
	int i;

	for (i = 0; i < NUMBER_OF_PROCS; i++) {
		mem_entry[i].shm = NULL;
		mem_entry[i].rlock = 0;
		mem_entry[i].wlock = 0;
		mem_entry[i].active = 0;
	}
}

/* Check a specific index int the ctrl area and copy to local memory  */

static void populate_mem_proc_single(int index)
{
	proc_entry *entry;
	int sem, mode = 0;

	entry = (proc_entry *) get_proc_at(index);
	if (entry->active) {
		/* this entry should be active, if not it has crached and should be garbage collected */
		if ((sem = create_lock(entry->key_active, 0)) != 0) {
			if (try_lock1(sem)) {
				print(CH_DEBUG, "Index %d is active by %s\n",
				      index, entry->proc_name);
				/* active lets store the data */
				mem_entry[index].active = sem;
				if ((mem_entry[index].shm =
				     (char *)get_shm(entry->key_shm,
						     entry->size_shm,
						     &mode)) == 0) {
					/* garbage collect, they should have valid keys */
					print(CH_ERROR,
					      "memshare: Unable to alloc shared mem\n");
					clear_proc_entry(index);
					return;
				}
				if ((mem_entry[index].rlock =
				     create_lock(entry->key_rlock, 0)) == 0) {
					/* garbage collect, they should have valid keys */
					print(CH_ERROR,
					      "memshare: Unable to create rlock\n");
					clear_proc_entry(index);
					return;
				}
				if ((mem_entry[index].wlock =
				     create_lock(entry->key_wlock, 0)) == 0) {
					/* garbage collect, they should have valid keys */
					print(CH_ERROR,
					      "memshare: Unable to create wlock\n");
					clear_proc_entry(index);
					return;
				}
			} else {
				print(CH_INFO,
				      "memshare: Index %d is active in shared mem but has no process\n",
				      index);
				clear_proc_entry(index);
				/* garbage collect */
			}
			memcpy(mem_entry[index].proc_name, entry->proc_name,
			       PROC_NAME_SIZE);
		}
	} else {
		print(CH_INFO, "memshare: Index %d is not active\n", index);
	}
}

/* scan through the ctrl area and copy to local memory */
static void populate_mem_proc(void)
{
	int i;
	proc_entry *entry;
	int sem, mode = 0;

	for (i = 0; i < NUMBER_OF_PROCS; i++) {
		populate_mem_proc_single(i);
	}
}

int check_proc_entry(int index)
{
	/* compare mem_entry with shared memory and se if process is active */
	proc_entry *entry;

	entry = (proc_entry *) get_proc_at(index);

	populate_mem_proc_single(index);

	print(CH_DEBUG, "Check proc entry index %d active1 %d active2 %d\n",
	      index, try_lock1(mem_entry[index].active), entry->active);
	if (try_lock1(mem_entry[index].active)
	    &&
	    (!memcmp
	     (mem_entry[index].proc_name, entry->proc_name, PROC_NAME_SIZE))
	    && entry->active) {
		return 1;
	}
	return 0;
}

static int clear_proc_entry(int index)
{
	proc_entry *entry;
	print(CH_INFO, "memshare: Removes proc from entry and memlist\n");
	entry = (proc_entry *) get_proc_at(index);
	clear_shm(entry->key_shm, entry->size_shm);
	entry->key_shm = 0;
	entry->size_shm = 0;
	mem_entry[index].shm = 0;
	destroy_lock(entry->key_rlock);
	entry->key_rlock = 0;
	mem_entry[index].rlock = 0;
	destroy_lock(entry->key_wlock);
	entry->key_wlock = 0;
	mem_entry[index].wlock = 0;
	destroy_lock(entry->key_active);
	entry->key_active = 0;
	mem_entry[index].active = 0;
	entry->active = 0;
	return 0;
}

/* This function will serach for a free entry in the ctrl area        */
/* there it will fill all the keys that can be used to map shared mem */
static int add_proc(char *name, int size)
{
	proc_entry *entry;
	int index, key_base = 0, mode = 0;

	if ((index = get_next_free_index()) == NUMBER_OF_PROCS) {
		return 1;
	}
	print(CH_DEBUG, "Addin my proc %s to index %d\n", name, index);
	entry = get_proc_at(index);
	key_base = index * 4 + SEM_CTRL_KEY;
	entry->key_shm = key_base + 1;
	entry->key_rlock = key_base + 2;
	entry->key_wlock = key_base + 3;
	entry->key_active = key_base + 4;
	entry->size_shm = size;
	entry->sent = 0;
	entry->received = 0;
	my_index = index;
	print(CH_DEBUG,
	      "Allocating shared memory allocated for key %d size %d\n",
	      entry->key_shm, entry->size_shm);
	/* Map up yourself in the local memory map with pointers instead */
	/* of keys                                                       */
	if ((mem_entry[index].shm =
	     get_shm(entry->key_shm, entry->size_shm, &mode)) == NULL) {
		clear_proc_entry(index);
		return 1;
	}
	print(CH_DEBUG, "Shared memory allocated for key %d\n", entry->key_shm);
	if ((mem_entry[index].rlock = create_lock(entry->key_rlock, 0)) == 0) {
		clear_proc_entry(index);
		return 1;
	}
	if ((mem_entry[index].wlock = create_lock(entry->key_wlock, 1)) == 0) {
		clear_proc_entry(index);
		return 1;
	}
	if ((mem_entry[index].active = create_lock(entry->key_active, 0)) == 0) {
		clear_proc_entry(index);
		return 1;
	}
	set_active(mem_entry[index].active);
	if (try_lock1(mem_entry[index].active))
		print(CH_DEBUG, "Proc %s is now active at index %d\n", name,
		      index);
	strncpy(entry->proc_name, name, PROC_NAME_SIZE);
	memcpy(mem_entry[index].proc_name, entry->proc_name, PROC_NAME_SIZE);
	/* signal the entry active to use */
	entry->active = 1;
}

static int start_listen_thread(void)
{
	pthread_attr_t tattr;

	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setinheritsched(&tattr, PTHREAD_INHERIT_SCHED);

	if (pthread_create(&recthread1_t, &tattr, recthread1, (void *)NULL) !=
	    0) {
		print(CH_ERROR, "memshare: Unable to create worker thread1\n");
		return 1;
	}

	if (pthread_create(&recthread2_t, &tattr, recthread2, (void *)NULL) !=
	    0) {
		print(CH_ERROR, "memshare: Unable to create worker thread2\n");
		return 1;
	}
}

/********** the shared memory functions ***********/
static int clear_shm(int key, int size)
{
	int shmid;

	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
		print(CH_ERROR, "memshare: Unable get shared mem for key\n",
		      key);
		return 1;
	}
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}

/* This function will try to map shared memory for an index(key)  */
/* The return value is a pointer to the shared memory or NULL if  */
/* the function fail.                                             */
/* If mode is set to 1 the shared memory will be created if it    */
/* not has been done before.                                      */
/* With mode = 0 the function will fail if the index has been     */
/* created before                                                 */
static char *get_shm(int key, int size, int *mode)
{
	int shmid;
	char *data;

	if (*mode) {
		if ((shmid =
		     shmget(key, size, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
			/* already open */
			*mode = 0;
		} else {
			print(CH_DEBUG, "Creates shared mem control area\n");
		}
	}
	if (*mode == 0) {
		if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
			print(CH_ERROR, "memshare, Unable get shared mem\n");
			return NULL;
		}
	}

	print(CH_INFO, "memshare: Key %d allocated with shmid %d\n", key,
	      shmid);
	if ((data = shmat(shmid, NULL, 0)) == (char *)-1) {
		print(CH_ERROR, "memshare: Unable to alloc shared mem\n");
		return NULL;
	}
	return data;
}

static int get_next_free_index()
{
	int i;

	for (i = 0; i < NUMBER_OF_PROCS; i++) {
		if (!check_proc_entry(i)) {
			return i;
		}
	}
	return NUMBER_OF_PROCS;
}

static proc_entry *get_proc_at(int index)
{
	char *tmp_ptr;
	proc_entry *entry;
	tmp_ptr = shm_ctrl_ptr + (SIZEOF_PROC_ENTRY * index);
	entry = (proc_entry *) tmp_ptr;
	return entry;
}

int get_proc_index(char *proc)
{
	int i;

	for (i = 0; i < NUMBER_OF_PROCS; i++) {
		if (check_proc_entry(i)) {
			if (!strcmp(mem_entry[i].proc_name, proc)) {
				return i;
			}
		}
	}
	return -1;
}

static int inc_sent(void)
{
	proc_entry *entry;

	/* The locks might not be needed TODO */
	/*lock(lock_ctrl_sem); */
	entry = (proc_entry *) get_proc_at(my_index);
	entry->sent++;
	/*unlock(lock_ctrl_sem); */
	return 0;
}

static int inc_received(void)
{
	proc_entry *entry;

	/* The locks might not be needed TODO */
	/*lock(lock_ctrl_sem); */
	entry = (proc_entry *) get_proc_at(my_index);
	entry->received++;
	/*unlock(lock_ctrl_sem); */
	return 0;
}

int send_ack(int seq)
{
	return 0;
}

/****************API***********************/

int set_print_level(int level)
{
	if ((level == CH_ERROR) || (level == CH_INFO) || (level == CH_DEBUG)) {
		current_level = level;
		return 0;
	}
	return 1;
}

void data_register(callback_data cbd)
{
	callbackdata = cbd;
}

void signal1_register(callback_1 cb1)
{
	callback1 = cb1;
}

void signal2_register(callback_2 cb2)
{
	callback2 = cb2;
}

void signal3_register(callback_3 cb3)
{
	callback3 = cb3;
}

int init_memshare(char *proc_name, int size)
{
	int ctrl_mode = 1, src_process = 1;
	print(CH_DEBUG, "init_memshare start\n");

	if (initialized)
		return 1;
	/* a source proc is a must */
	if (proc_name == NULL)
		return 2;

	memcpy(my_proc, proc_name, PROC_NAME_SIZE);

	if (size) {
		init_queues();
		seize_queue(&queue_index, "memshare", QUEUE_SIZE);
	}

	/* clear the memory view */
	init_mem_proc();

	/* start off by locking the ctrl lock */
	if ((lock_ctrl_sem = create_lock(SEM_CTRL_KEY, 1)) == 0) {
		print(CH_ERROR, "memshare: Unable to create semaphore\n");
		return 1;
	}
	print(CH_DEBUG, "Created ctrl lock\n");

	lock(lock_ctrl_sem);
	print(CH_DEBUG, "init_memshare ctrl\n");

	/* map up the ctrl area */
	if ((shm_ctrl_ptr = get_shm(SHM_CTRL_KEY, CTRL_SIZE, &ctrl_mode)) == 0) {
		print(CH_ERROR, "memshare: Unable to alloc shared mem\n");
		return 3;
	}
	print(CH_DEBUG, "init_memshare: populate memproc\n");
	populate_mem_proc();

	if (size)
		add_proc(proc_name, size);

	unlock(lock_ctrl_sem);
	print(CH_DEBUG, "init_memshare:unlock ctrl\n");

	if (size)
		start_listen_thread();
	print(CH_DEBUG, "init_memshare done\n");
	initialized = 1;
}

int data(char *proc, char *data, int len)
{
	int index;
	header hdr;
	if (!initialized)
		return 2;
	if ((index = get_proc_index(proc)) < 0) {
		print(CH_INFO, "memshare: No such process %s\n", proc);
		return 1;
	}
	/*populate_mem_proc_single(index); */
	print(CH_DEBUG, "Sending data to %s at index %d\n", proc, index);
	lock(mem_entry[index].wlock);
	hdr.msg_type = DATA;
	hdr.msg_len = len;
	hdr.seq = sequence++;
	memcpy(hdr.proc_name, my_proc, PROC_NAME_SIZE);
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	memcpy((mem_entry[index].shm + SIZEOF_HEADER), data, len);
	inc_sent();
	unlock(mem_entry[index].rlock);
	return 0;
}

/* return 1 the process isn't connected */
int signal1(char *proc, int data1)
{
	int index;
	header hdr;
	signal sig;

	print(CH_DEBUG, "Sending signal to %s at index %d\n", proc, index);
	if (!initialized)
		return 2;
	if ((index = get_proc_index(proc)) < 0) {
		print(CH_INFO, "memshare: No such process %s\n", proc);
		return 1;
	}
	/*populate_mem_proc_single(index); */
	print(CH_DEBUG, "Sending signal to %s at index %d\n", proc, index);
	lock(mem_entry[index].wlock);
	hdr.msg_type = SIGNAL1;
	hdr.msg_len = SIZEOF_SIGNAL;
	hdr.seq = sequence++;
	memcpy(hdr.proc_name, my_proc, PROC_NAME_SIZE);
	sig.signal1 = data1;
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	memcpy((mem_entry[index].shm + SIZEOF_HEADER), &sig, SIZEOF_SIGNAL);
	inc_sent();
	unlock(mem_entry[index].rlock);
	return 0;
}

int signal2(char *proc, int data1, int data2)
{
	int index;
	header hdr;
	signal sig;

	if (!initialized)
		return 2;
	if ((index = get_proc_index(proc)) < 0) {
		print(CH_INFO, "memshare: No such process %s\n", proc);
		return 1;
	}
	/*populate_mem_proc_single(index); */
	print(CH_DEBUG, "Sending signal to %s at index %d\n", proc, index);
	lock(mem_entry[index].wlock);
	hdr.msg_type = SIGNAL2;
	hdr.msg_len = SIZEOF_SIGNAL;
	hdr.seq = sequence++;
	memcpy(hdr.proc_name, my_proc, PROC_NAME_SIZE);
	sig.signal1 = data1;
	sig.signal2 = data2;
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	memcpy((mem_entry[index].shm + SIZEOF_HEADER), &sig, SIZEOF_SIGNAL);
	inc_sent();
	unlock(mem_entry[index].rlock);
	return 0;
}

int signal3(char *proc, int data1, int data2, int data3)
{
	int index;
	header hdr;
	signal sig;

	if (!initialized)
		return 2;
	if ((index = get_proc_index(proc)) < 0) {
		print(CH_INFO, "memshare: No such process %s\n", proc);
		return 1;
	}
	/*populate_mem_proc_single(index); */
	print(CH_DEBUG, "Sending signal to %s at index %d\n", proc, index);
	lock(mem_entry[index].wlock);
	hdr.msg_type = SIGNAL3;
	hdr.msg_len = SIZEOF_SIGNAL;
	hdr.seq = sequence++;
	memcpy(hdr.proc_name, my_proc, PROC_NAME_SIZE);
	sig.signal1 = data1;
	sig.signal2 = data2;
	sig.signal3 = data3;
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	memcpy((mem_entry[index].shm + SIZEOF_HEADER), &sig, SIZEOF_SIGNAL);
	inc_sent();
	unlock(mem_entry[index].rlock);
	return 0;
}

int get_datasize(char *proc)
{
	int index;
	proc_entry *entry;

	if ((index = get_proc_index(proc)) < 0) {
		print(CH_INFO, "memshare: No such process %s\n", proc);
		return 0;
	}
	entry = get_proc_at(index);
	return (entry->size_shm);
}

int get_proc_info(int index, int *send_count, int *rec_count, int *data_size,
		  char **procptr)
{
	proc_entry *entry;

	/* this call will not check if the entry is active */
	entry = get_proc_at(index);
	*send_count = entry->sent;
	*rec_count = entry->received;
	*data_size = entry->size_shm;
	*procptr = entry->proc_name;
	return 0;
}

 /*
    int
    s_data(int index, char *data, int len)
    {
    return 0;
    }

    int
    s_signal1(int index, int data)
    {
    return 0;
    }

    int
    s_signal2(int index, int data1, int data2)
    {
    return 0;
    }

    int
    s_signal3(int index, int data1, int data2, int data3)
    {
    return 0;
    } */
