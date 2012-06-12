/*
 *      Author: tommy.wiklund
 *
 *      Revision information
 *      2012-02   twik    Initial draft
 *
 *
 */

#ifndef _MEMSHARE_H
#define _MEMSHARE_H

#include <pthread.h>
#include "memshare_api.h"

#define  CTRL_SIZE   1024
#define  SHM_CTRL_KEY  42
#define  SEM_CTRL_KEY  43

#define  MAJOR 0
#define  MINOR 1

#define DATA     1
#define SIGNAL1  2
#define SIGNAL2  3
#define SIGNAL3  4

#define ADATA    5
#define ASIGNAL1 6
#define ASIGNAL2 7
#define ASIGNAL3 8

#define ACK      100

#define NUMBER_OF_PROCS 10
#define QUEUE_SIZE 500

/* struct that is passed along with the data/signal between procs */
typedef struct {
	char proc_name[PROC_NAME_SIZE];	/* orginating proc */
	char msg_type;
	int msg_len;
	int msg_prio;
	int seq;
} header;
#define SIZEOF_HEADER sizeof(header)

typedef struct {
	int signal1;
	int signal2;
	int signal3;
} signal;
#define SIZEOF_SIGNAL sizeof(signal)

/* Every process will scan the ctrl area and map every proc entry */
/* to a mem_proc_entry for quick access to that process */
typedef struct {
	char proc_name[PROC_NAME_SIZE];
	char *shm;
	int rlock;
	int wlock;
	int active;
} mem_proc_entry;
#define SIZEOF_MEM_PROC_ENTRY sizeof(mem_proc_entry)

/* Every process will register one of these in the ctrl area */
typedef struct {
	int size;		/* size of this struct?  */
	char version_major;	/* version major         */
	char version_minor;	/* version minor         */
	int key_shm;		/* key = index (in ctrl area) * 4 + 43 */
	int size_shm;		/* size of the shared memory allocated */
	int key_rlock;		/* key to hold the read lock for the shared mem */
	int key_wlock;		/* key to hold the write lock for the shared mem */
	int key_active;		/* key which will go zero if process terminates */
	int active;		/* flagg for the process to signal active */
	char proc_name[PROC_NAME_SIZE];	/* process name */
	char ready;
	int received;		/* counter for received data/signals */
	int sent;		/* counter for sent data/signals */
	int sent_acked;		/* future improvements */
	int recevied_acked;	/* future improvements */
} proc_entry;
#define SIZEOF_PROC_ENTRY sizeof(proc_entry)

int get_proc_info(int, int*, int*, int*, char**);
int get_proc_index(char*);
int check_proc_entry(int);

#endif				/* _MEMSHARE_H */
