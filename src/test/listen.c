/* Memshare, quick and easy IPC.                                                   */
/* Copyright (C) 2012  Tommy Wiklund                                               */
/* This file is part of Memshare.                                                  */
/*                                                                                 */
/* Memshare is free software: you can redistribute it and/or modify                */
/* it under the terms of the GNU Lesser General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or               */
/* (at your option) any later version.                                             */
/*                                                                                 */
/* Memshare is distributed in the hope that it will be useful,                     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU Lesser General Public License for more details.                             */
/*                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License        */
/* along with Memshare.  If not, see <http://www.gnu.org/licenses/>.               */

#include <memshare_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define SHMEMSIZE 512
#define QUEUESIZE 800

int state = 0;
char *procname;

void data_callback(char *proc, char *msg, int len)
{
	printf("Data received %s\n", msg);
}

void signal1_callback(char *proc, int value)
{
	printf("Signal1 received %d\n", value);
}

void signal2_callback(char *proc, int value1, int value2)
{
	printf("Signal2 received %d, %d\n", value1, value2);
}

void signal3_callback(char *proc, int value1, int value2, int value3)
{
	printf("Signal3 received %d, %d, %d\n", value1, value2, value3);
}

int main(int argc, char *argv[])
{
	procname = malloc(256);
	strncpy(procname, argv[1], 256);

	set_print_level(0);
	printf("%s started\n", procname);

	if (init_memshare(procname,SHMEMSIZE , QUEUESIZE)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	data_register(data_callback);
	signal1_register(signal1_callback);
	signal2_register(signal2_callback);
	signal3_register(signal3_callback);

	while (1)
		sleep(3);
}
