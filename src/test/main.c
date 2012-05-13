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

#include <memshare_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define SHMEMSIEZ  512
#define LARGE_NUM  500
#define LARGER_NUM 700

char data_str[128] = "Ta mig till ett ställe där man gör vad fan man vill\0";
#define SIZEOF_DATA strlen(data_str)
int mode, count;

void signal1_callback(char *proc, int value)
{
	if (mode == 1) {
		if (value != 1) {
			printf("Test 2.1 failed with value %d\n", value);
			exit(1);
		}
		count++;
	}
	if (mode == 3) {
		if (value != 3) {
			printf("Test 5.1 failed with value %d\n", value);
			exit(1);
		}
		count++;
	}

	if (mode == 4) {
		if (value != 8) {
			printf("Test 6.1 failed with value %d\n", value);
			exit(1);
		}
		count++;
	}
}

void signal2_callback(char *proc, int value1, int value2)
{
	if (mode == 1) {
		if ((value1 != 1) || (value2 != 2)) {
			printf("Test 2.2 failed with value %d, %d\n", value1,
			       value2);
			exit(1);
		}
		count++;
	}
	if (mode == 3) {
		if ((value1 != 3) || (value2 != 4)) {
			printf("Test 5.2 failed with value %d, %d\n", value1,
			       value2);
			exit(1);
		}
		count++;
	}
	if (mode == 4) {
		if ((value1 != 12) || (value2 != 24)) {
			printf("Test 6.2 failed with value %d, %d\n", value1,
			       value2);
			exit(1);
		}
		count++;
	}
}

void signal3_callback(char *proc, int value1, int value2, int value3)
{
	if (mode == 1) {
		if ((value1 != 1) || (value2 != 2) || (value3 != 3)) {
			printf("Test 2.3 failed with value %d, %d, %d\n",
			       value1, value2, value3);
			exit(1);
		}
		count++;
	}
	if (mode == 3) {
		if ((value1 != 3) || (value2 != 4) || (value3 != 65534)) {
			printf("Test 5.3 failed with value %d, %d, %d\n",
			       value1, value2, value3);
			exit(1);
		}
		count++;
	}
	if (mode == 4) {
		if ((value1 != 48) || (value2 != 96) || (value3 != 192)) {
			printf("Test 6.3 failed with value %d, %d, %d\n",
			       value1, value2, value3);
			exit(1);
		}
		count++;
	}
}

void data_callback(char *proc, char *msg, int len)
{
	if (mode == 1) {
		if (strcmp(msg, data_str) != 0) {
			printf("Test 2.4 failed with value %s\n", msg);
			exit(1);
		}
		count++;
	}
	if (mode == 3) {
		if (strcmp(msg, data_str) != 0) {
			printf("Test 5.4 failed with value %s\n", msg);
			exit(1);
		}
		count++;
	}

}

/*****************************************************************************/
/*                              Test one                                     */
/* Description        : Send without having memshare initialized             */
/*                                                                           */
/*****************************************************************************/
int test_one()
{
	int retvalue;
	mode = 0;
	count = 0;
	if ((retvalue = signal1("reply", 1)) != 2) {
		printf("Test 1,0 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal2("reply", 1, 2)) != 2) {
		printf("Test 1,1 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal3("reply", 1, 2, 3)) != 2) {
		printf("Test 1,2 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = data("reply", data_str, SIZEOF_DATA)) != 2) {
		printf("Test 1,3 failed return value %d\n", retvalue);
		return 1;
	}
	printf("Test 1 OK\n");
	return 0;
}

/*****************************************************************************/
/*                              Test two                                     */
/* Description        : Initilize in different ways                          */
/*                                                                           */
/*****************************************************************************/
int test_two()
{
	int retvalue;
	mode = 0;
	count = 0;
	if ((retvalue = init_memshare(NULL, 0)) != 2) {
		printf("Test 2,0 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = init_memshare("memshare", SHMEMSIEZ)) != 0) {
		printf("Test 2,1 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = init_memshare("memshare", SHMEMSIEZ)) != 1) {
		printf("Test 2,3 failed return value %d\n", retvalue);
		return 1;
	}
	printf("Test 2 OK\n");
	return 0;
}

/*****************************************************************************/
/*                              Test three                                   */
/* Description        : Send successful with all funcs                       */
/*                                                                           */
/*****************************************************************************/
int test_three()
{
	int retvalue;
	mode = 1;
	count = 0;
	if ((retvalue = signal1("reply", 1)) != 0) {
		printf("Test 3,0 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal2("reply", 1, 2)) != 0) {
		printf("Test 3,1 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal3("reply", 1, 2, 3)) != 0) {
		printf("Test 3,2 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = data("reply", data_str, SIZEOF_DATA + 1)) != 0) {
		printf("Test 3,3 failed return value %d\n", retvalue);
		return 1;
	}
	sleep(1);
	if (count == 4) {
		printf("Test 3 OK\n");
		return 0;
	} else {
		printf("Test 3 failed %d\n", count);
		return 1;
	}
}

/*****************************************************************************/
/*                              Test four                                    */
/* Description        : Send some faulty stuff                               */
/*                                                                           */
/*****************************************************************************/
int test_four()
{
	int retvalue;
	mode = 2;
	count = 0;
	if ((retvalue = signal1("replyer", 1)) != 1) {
		printf("Test 4,0 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal2("replyer", 1, 2)) != 1) {
		printf("Test 2,1 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = signal3("replyer", 1, 2, 3)) != 1) {
		printf("Test 2,2 failed return value %d\n", retvalue);
		return 1;
	}
	if ((retvalue = data("replyer", data_str, SIZEOF_DATA)) != 1) {
		printf("Test 2,3 failed return value %d\n", retvalue);
		return 1;
	}
	printf("Test 4 OK\n");
	return 0;
}

/*****************************************************************************/
/*                              Test five                                    */
/* Description        : Send a lot                                           */
/*                                                                           */
/*****************************************************************************/
int test_five(int amount)
{
	int retvalue, i;
	mode = 3;
	count = 0;

	for (i = 0; i < amount; i++) {
		if ((retvalue = signal1("reply", 3)) != 0) {
			printf("Test 5,0 failed return value %d\n", retvalue);
			return 1;
		}
	}
	sleep(1);		/* give time for reply to empty the queue */
	for (i = 0; i < amount; i++) {
		if ((retvalue = signal2("reply", 3, 4)) != 0) {
			printf("Test 5,1 failed return value %d\n", retvalue);
			return 1;
		}
	}
	sleep(1);		/* give time for reply to empty the queue */
	for (i = 0; i < amount; i++) {
		if ((retvalue = signal3("reply", 3, 4, 65534)) != 0) {
			printf("Test 5,2 failed return value %d\n", retvalue);
			return 1;
		}
	}
	sleep(1);		/* give time for reply to empty the queue */
	for (i = 0; i < amount; i++) {
		if ((retvalue = data("reply", data_str, SIZEOF_DATA + 1)) != 0) {
			printf("Test 5,3 failed return value %d\n", retvalue);
			return 1;
		}
	}
	sleep(1);		/* give time for reply to empty the queue */
	if (count == (4 * amount)) {
		printf("Test 5 OK\n");
		return 0;
	} else {
		printf("Test 5 failed, received %d out of %d\n", count,
		       (4 * amount));
		return 1;
	}
}

/*****************************************************************************/
/*                              Test six                                     */
/* Description        : Send using memsend                                   */
/*                                                                           */
/*****************************************************************************/
int test_six()
{
	int retvalue;
	mode = 4;
	count = 0;

	if ((retvalue = signal1("reply", 666)) != 0) {
		printf("Test 6,0 failed return value %d\n", retvalue);
		return 1;
	}

	sleep(3);
	if (count == 3) {
		printf("Test 6 OK\n");
		return 0;
	} else {
		printf("Test 6 failed %d\n", count);
		return 1;
	}
}

int main(int argc, char *argv[])
{
	int i;
	system("./reply reply &");
	sleep(1);

	if (test_one())
		exit(1);

	if (test_two())
		exit(1);

	data_register(data_callback);
	signal1_register(signal1_callback);
	signal2_register(signal2_callback);
	signal3_register(signal3_callback);

	if (test_three())
		exit(1);

	sleep(2);
	if (test_four())
		exit(1);

	if (test_five(LARGE_NUM))
		exit(1);

	if (test_five(LARGER_NUM) != 0)
		exit(1);

	if (test_five(LARGE_NUM))
		exit(1);

	if (test_six())
		exit(1);

	data("reply", "exit\0", strlen("exit") + 1);
	return 0;
}
