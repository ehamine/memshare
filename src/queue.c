/*
 *      Author: tommy.wiklund
 *
 *      Revision information
 *      2012-02   twik    Initial draft
 *
 *
 */

#include "queue.h"

sem_t qlock[NUM_OF_QUEUES], seize_lock;
pthread_mutex_t condition_mutex[NUM_OF_QUEUES] = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond[NUM_OF_QUEUES]  = PTHREAD_COND_INITIALIZER;
int low_bottom[NUM_OF_QUEUES], low_top[NUM_OF_QUEUES];
int low_totalsize[NUM_OF_QUEUES], low_state[NUM_OF_QUEUES];
int hi_bottom[NUM_OF_QUEUES], hi_top[NUM_OF_QUEUES];
int hi_totalsize[NUM_OF_QUEUES], hi_state[NUM_OF_QUEUES];
int seize_queue_state[NUM_OF_QUEUES];
char *seize_queue_name[NUM_OF_QUEUES];
char **low_queuebase[NUM_OF_QUEUES], **hi_queuebase[NUM_OF_QUEUES];

int queue_state = 0;

static int lo_qinit(int index, int size);
static int hi_qinit(int index, int size);

/*****************************************************************************/
/* Function Name      : init_queues                                          */
/* Description        : This function initialises the queue seize env        */
/* Input(s)           : None                                                 */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 meening memoru allocation failure                  */
/*****************************************************************************/

int init_queues()
{
  int i;
  for (i=0;i<NUM_OF_QUEUES;i++)
    {
      seize_queue_state[i] = 0;
      if ((seize_queue_name[i] = malloc(100)) == 0)
	{
	  return 1;
	}
    }
  queue_state = 1;
  return 0;
}

/*****************************************************************************/
/* Function Name      : seize_queue                                          */
/* Description        : This function reserves a queue by a name and sets    */
/*                      the depth(size)                                      */
/* Input(s)           : name for identification(max 100 char), size of queue */
/* Output(s)          : index to be used for quick reference                 */
/* Return Value(s)    : 0 ok (trust the index)                               */
/*                      1 name already seized,size ignored (trust the index) */
/*                  (for rest of the return codes the index can't be trusted */
/*                      2 queue not initialized                              */
/*                      3 no queues avaliable                                */
/*                      4 sanity check of parameters failed                  */
/*****************************************************************************/

int seize_queue(int *index, char *name, int size)
{
  int i;
  if (queue_state == 0)
    {
      return 2;
    }

  if (index == 0 || name == 0 || size == 0)
    {
      return 4;
    }

  /* search for already seized */
  for (i=0;i<NUM_OF_QUEUES;i++)
    {
      if ((seize_queue_state[i] == 1) && (!strncmp(name, seize_queue_name[i], NUM_OF_QUEUES)))
	{
	  *index = i;
	  return 1;
	}
    }
  /* search for a free entry */
  for (i=0;i<NUM_OF_QUEUES;i++)
   {
     if (seize_queue_state[i] == 0)
       {
	 seize_queue_state[i] = 1;
	 /* Found an index initalize the queues for that index */
	 lo_qinit(i, size);
	 hi_qinit(i, size);
	 strncpy(seize_queue_name[i], name, NUM_OF_QUEUES);
	 *index = i;
	  return 0;
	}
    }
  return 3;
}

/*****************************************************************************/
/* Function Name      : lo_qinit                                             */
/* Description        : This function initializes a queue and sets           */
/*                      the depth(size), called from seize_queus             */
/* Input(s)           : index and size                                       */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 memory allocation failed                           */
/*****************************************************************************/

static int lo_qinit(int index, int size)
{
  if ((low_queuebase[index] = malloc(sizeof(char*) * size)) != NULL)
    {
      /* initalize all stuff */
      low_bottom[index] = 0;
      low_top[index] = 0;
      low_totalsize[index] = size;
      low_state[index] = EMPTY;
      sem_init(&qlock[index], 0, 1);
      return 0;
    }
  return 1;
}


/*****************************************************************************/
/* Function Name      : hi_qinit                                             */
/* Description        : This function initializes a queue and sets           */
/*                      the depth(size), called from seize_queus             */
/* Input(s)           : index and size                                       */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 memory allocation failed                           */
/*****************************************************************************/

static int hi_qinit(int index, int size)
{
    if ((hi_queuebase[index] = malloc(sizeof(char*) * size)) != NULL)
    {
	hi_bottom[index] = 0;
	hi_top[index] = 0;
	hi_totalsize[index] = size;
	hi_state[index] = EMPTY;
	sem_init(&qlock[index], 0, 1);
	return 0;
    }
    return 1;
}


/*****************************************************************************/
/* Function Name      : lo_qadd                                              */
/* Description        : This function adds a pointer to a low prio queue     */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/

int lo_qadd(int index, char** entry)
{
  if (entry == NULL)
    {
      return 1;
    }
  sem_wait(&qlock[index]);
  int prev_state = low_state[index];
  if (low_state[index] == FULL)
    {
      sem_post(&qlock[index]);
      return 2;
    }
  low_queuebase[index][low_top[index]] = (char*)*entry;
  low_top[index]++;
  low_state[index] = NORMAL;
  if (low_top[index] == low_totalsize[index])
    {
      low_top[index] = 0;
    }
  if (low_top[index] == low_bottom[index])
    {
      low_state[index] = FULL;
    }
  pthread_mutex_lock( &condition_mutex[index] );
  if ((prev_state == EMPTY) && (hi_state[index] == EMPTY))
    {
      pthread_cond_signal( &condition_cond[index] );
    }
  pthread_mutex_unlock( &condition_mutex[index] );
  sem_post(&qlock[index]);
  return 0;
}


/*****************************************************************************/
/* Function Name      : hi_qadd                                              */
/* Description        : This function adds a pointer to a high prio queue    */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/

int hi_qadd(int index, char** entry)
{
    if (entry == NULL)
    {
	return 1;
    }
    sem_wait(&qlock[index]);
    int prev_state = hi_state[index];
    if (hi_state[index] == FULL)
    {
	sem_post(&qlock[index]);
	return 2;
    }
    hi_queuebase[index][hi_top[index]] = (char*)*entry;
    hi_top[index]++;
    hi_state[index] = NORMAL;
    if (hi_top[index] == hi_totalsize[index])
    {
	hi_top[index] = 0;
    }
    if (hi_top[index] == hi_bottom[index])
    {
	hi_state[index] = FULL;
    }
    pthread_mutex_lock( &condition_mutex[index] );
    if ((prev_state == EMPTY) && (low_state[index] == EMPTY))
    {
	pthread_cond_signal( &condition_cond[index] );
    }
    pthread_mutex_unlock( &condition_mutex[index] );
    sem_post(&qlock[index]);
    return 0;
}


/*****************************************************************************/
/* Function Name      : gethiq                                               */
/* Description        : Internal function                                    */
/* Input(s)           : index                                                */
/* Return Value(s)    :                                                      */
/*****************************************************************************/

static char *gethiq(int index)
{
  char *retval;
  retval = hi_queuebase[index][hi_bottom[index]];
  hi_bottom[index]++;
  if (hi_bottom[index] == hi_totalsize[index])
    {
      hi_bottom[index] = 0;
    }
  if (hi_bottom[index] == hi_top[index])
    {
      hi_state[index] = EMPTY;
    }
  return retval;
}


/*****************************************************************************/
/* Function Name      : getliq                                               */
/* Description        : Internal function                                    */
/* Input(s)           : index                                                */
/* Return Value(s)    :                                                      */
/*****************************************************************************/

static char *getloq(int index)
{
  char *retval;
  retval = low_queuebase[index][low_bottom[index]];
  low_bottom[index]++;
  if (low_bottom[index] == low_totalsize[index])
    {
      low_bottom[index] = 0;
    }
  if (low_bottom[index] == low_top[index])
    {
      low_state[index] = EMPTY;
    }
  return retval;
}


/*****************************************************************************/
/* Function Name      : qget                                                 */
/* Description        : This function picks the next entry in the indexed    */
/*                      queue, preferably the hi_queue.                      */
/*                      The call will block until an entry is available      */
/* Input(s)           : index                                                */
/* Return Value(s)    : A pointer to a buffer                                */
/*****************************************************************************/

char *qget(int index)
{
  char *retval;
  static int count;
 
  pthread_mutex_lock( &condition_mutex[index] );
  if ((low_state[index] == EMPTY) && (hi_state[index] == EMPTY))
    {
      pthread_cond_wait( &condition_cond[index], &condition_mutex[index] );
    }
  pthread_mutex_unlock( &condition_mutex[index] );
  sem_wait(&qlock[index]);

  /* pick every 10th packet first from loq*/
  count++;
  if (count == 10) 
    {
      if (low_state[index] != EMPTY)
	{
	  retval = getloq(index);
	}
      else if (hi_state[index] != EMPTY)
	{
	  retval = gethiq(index);
	}
      count = 0;
    } 
  else
    { 
      if (hi_state[index] != EMPTY)
	{
	  retval = gethiq(index);
	}
      else if (low_state[index] != EMPTY)
	{
	  retval = getloq(index);
	}
    }
 
  sem_post(&qlock[index]);
  return retval;
}


/*****************************************************************************/
/* Function Name      : qpeek                                                */
/* Description        : This function peeks for the next entry in the        */
/*                      indexed queue, preferably the hi_queue.              */
/* Input(s)           : index                                                */
/* Return Value(s)    : 0 no entry available in any (hi or lo) queue         */
/*                      1 An entry returned in buff                          */
/*****************************************************************************/

int qpeek(int index, char **buff)
{
    pthread_mutex_lock( &condition_mutex[index] );
    if (hi_state[index] != EMPTY)
    {
	(*buff) = hi_queuebase[index][hi_bottom[index]];
	pthread_mutex_unlock( &condition_mutex[index] );
	return 1;
    }
    else if (low_state[index] != EMPTY)
    {
	(*buff) = low_queuebase[index][low_bottom[index]];
	pthread_mutex_unlock( &condition_mutex[index] );
	return 1;
    }
    pthread_mutex_unlock( &condition_mutex[index] );
    return 0;
}

