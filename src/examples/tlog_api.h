/*
 *
 * Tlog, syslog wrapper with IPC control.
 * Copyright (C) 2012  Tommy Wiklund
 *
 */

/*****************************************************************************/
/* Function Name      : tsyslog_init                                         */
/* Description        : Initializes tlog, sets the syslog tag name           */
/* Input(s)           : char* (string 20 char)                               */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                    : 1 a name has to be present                           */
/*                    : 2 unable to initialize memshare                      */
/*****************************************************************************/
int tsyslog_init(char *name);

/*****************************************************************************/
/* Function Name      : tsyslog                                              */
/* Description        : Push a string to syslog depending on priority mask   */
/* Input(s)           : int priority                                         */
/*                    : char* (string) to be sent to syslog                  */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                    : 1 it has to be initalized first                      */
/*****************************************************************************/
void tsyslog(int, const char *, ...);

/*****************************************************************************/
/* Function Name      : tsyslog_set                                          */
/* Description        : Set a bit in the mask for syslog output              */
/* Input(s)           : int priority according to syslog.h                   */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                    : 1 it has to be initalized first                      */
/*****************************************************************************/
int tsyslog_set(int);

/*****************************************************************************/
/* Function Name      : tsyslog_del                                          */
/* Description        : Delete a bit in the priority mask for syslog output  */
/* Input(s)           : int priority according to syslog.h                   */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                    : 1 it has to be initalized first                      */
/*****************************************************************************/
int tsyslog_del(int);

/*****************************************************************************/
/* Function Name      : tsyslog_replace                                      */
/* Description        : Replace the priority mask for syslog output          */
/* Input(s)           : int priority according to syslog.h                   */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                    : 1 it has to be initalized first                      */
/*****************************************************************************/
int tsyslog_replace(int);

/*****************************************************************************/
/*                                                                           */
/* Set priority bit      :   memsend -s2 <process > 1 <priority>             */
/* Del priority bit      :   memsend -s2 <process > 2 <priority>             */
/* Replace priority mask :   memsend -s2 <process > 3 <priority mask>        */
/* This can be used to dynamically set/delete/replace the priority mask for  */
/* different processes                                                       */
/*                                                                           */
/* LOG_EMERG     priority=0                                                  */
/* LOG_ALERT     priority=1                                                  */
/* LOG_CRIT      priority=2                                                  */
/* LOG_ERR       priority=3                                                  */
/* LOG_WARNING   priority=4                                                  */
/* LOG_NOTICE    priority=5                                                  */
/* LOG_INFO      priority=6                                                  */
/* LOG_DEBUG     priority=7                                                  */
/*                                                                           */
/*****************************************************************************/
