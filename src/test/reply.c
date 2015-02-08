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
	int retvalue = 0;
	if (!strcmp("exit", msg)) {
		printf("%s exiting\n", procname);
		exit(0);
	}
	if ((retvalue = data(proc, msg, len)))
		printf("replay:data failed with %d |%s|\n", retvalue, msg);
}

void signal1_callback(char *proc, int value)
{
	int retvalue = 0;
	if (value == 666) {
		system("./memsend -s1 memshare 8");
		system("./memsend -s2 memshare 12 24");
		system("./memsend -s3 memshare 48 96 192");
		return;
	}
	if ((retvalue = signal1(proc, value)))
		printf("replay:signal1 failed with %d\n", retvalue);
}

void signal2_callback(char *proc, int value1, int value2)
{
	int retvalue = 0;
	if ((retvalue = signal2(proc, value1, value2)))
		printf("replay:signal2 failed with %d\n", retvalue);
}

void signal3_callback(char *proc, int value1, int value2, int value3)
{
	int retvalue = 0;
	if ((retvalue = signal3(proc, value1, value2, value3)))
		printf("replay:signal3 failed with %d\n", retvalue);
}

int main(int argc, char *argv[])
{
	procname = malloc(256);
	strncpy(procname, argv[1], 256);

	set_print_level(0);
	printf("%s started\n", procname);

	if (init_memshare(procname, SHMEMSIZE, QUEUESIZE)) {
		printf("Failed to init memshare\n");
		if (strncmp(procname, "number_8", PROC_NAME_SIZE) == 0)
			system("./memsend -s1 memshare 7");
		exit(1);
	}
	data_register(data_callback);
	signal1_register(signal1_callback);
	signal2_register(signal2_callback);
	signal3_register(signal3_callback);

	while (1)
		sleep(3);
}
