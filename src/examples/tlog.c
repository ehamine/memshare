/* Tlog, syslog wrapper with IPC control.                                          */
/* Copyright (C) 2012  Tommy Wiklund                                               */
/* This file is part of Tlog.                                                      */
/*                                                                                 */
/* Tlog is free software: you can redistribute it and/or modify                    */
/* it under the terms of the GNU Lesser General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or               */
/* (at your option) any later version.                                             */
/*                                                                                 */
/* Tlog is distributed in the hope that it will be useful,                         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU Lesser General Public License for more details.                             */
/*                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License        */
/* along with Tlog.  If not, see <http://www.gnu.org/licenses/>.                   */

#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "memshare_api.h"
#include "tlog_api.h"

#define SHMEMSIZE 50
#define QUEUESIZE 512

int init = 0;
int mask = 0;
char tproc[PROC_NAME_SIZE];

void signal2_callback(char *proc, int value1, int value2)
{
	switch (value1) {
	case 1:
		if ((value2 < 0) || (value2 > 8))
			return;
		/* Set a bit in the mask */
		mask |= (1 << value2);
		break;

	case 2:
		if ((value2 < 0) || (value2 > 8))
			return;
		/* Del a bit in the mask */
		mask &= ~(1 << value2);
		break;

	case 3:
		if ((value2 < 0) || (value2 > 255))
			return;
		mask = value2;
		break;

	default:
		return;
		break;
	}

	return;
}

int tsyslog_set(int value)
{
	if (!init)
		return 1;
	/* use the memshare to skip locks */
	signal2(tproc, 1, value);
	return 0;
}

int tsyslog_del(int value)
{
	if (!init)
		return 1;
	/* use the memshare to skip locks */
	signal2(tproc, 2, value);
	return 0;
}

int tsyslog_replace(int value)
{
	if (!init)
		return 1;
	/* use the memshare to skip locks */
	signal2(tproc, 3, value);
	return 0;
}

void tsyslog(int priority, const char *fmt, ...)
{
	va_list ap;

	/* check mask */
	if (mask & (1 << priority)) {
		va_start(ap, fmt);
		vsyslog(priority, fmt, ap);
		va_end(ap);
	}
}

int tsyslog_init(char *name)
{
	if (name == NULL)
		return 1;
	strncpy(tproc, name, (PROC_NAME_SIZE - 1));

	/* we don't need much space */
	if (init_memshare(tproc, SHMEMSIZE, 512) != 0)
		return 2;
		
	/* let memshare use tsyslog as well */
	logfunction_register(tsyslog);

	/* register the callback */
	signal2_register(signal2_callback);

	openlog(tproc, LOG_NDELAY | LOG_CONS, LOG_LOCAL0);

	init = 1;
	return 0;
}
