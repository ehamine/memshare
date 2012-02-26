/*
 *      Author: tommy.wiklund
 *
 *      Revision information
 *      2012-02   twik    Initial draft
 *
 *
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

#define EMPTY   0
#define NORMAL  1
#define FULL    2
#define NUM_OF_QUEUES 5

/*****************************************************************************/
/* Function Name      : init_queues                                          */
/* Description        : This function initialises the queue seize env        */
/* Input(s)           : None                                                 */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 meaning memory allocation failure                  */
/*****************************************************************************/
int init_queues();

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
int seize_queue(int *index, char *name, int size);

/*****************************************************************************/
/* Function Name      : lo_qadd                                              */
/* Description        : This function adds a pointer to a low prio queue     */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/
int lo_qadd(int index, char** entry);

/*****************************************************************************/
/* Function Name      : hi_qadd                                              */
/* Description        : This function adds a pointer to a high prio queue    */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/
int hi_qadd(int index, char** entry);

/*****************************************************************************/
/* Function Name      : qget                                                 */
/* Description        : This function picks the next entry in the indexed    */
/*                      queue, preferably the hi_queue.                      */
/*                      The call will block until an entry is available      */
/* Input(s)           : index                                                */
/* Return Value(s)    : A pointer to a buffer                                */
/*****************************************************************************/
char *qget(int index);

/*****************************************************************************/
/* Function Name      : qpeek                                                */
/* Description        : This function peeks for the next entry in the        */
/*                      indexed queue, preferably the hi_queue.              */
/* Input(s)           : index                                                */
/* Return Value(s)    : 0 no entry available in any (hi or lo) queue         */
/*                      1 An entry returned in buff                          */
/*****************************************************************************/
int qpeek(int index, char **buff);

#endif
