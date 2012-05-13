//* Memshare, quick and easy IPC.                                                   */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memshare.h"
#include "memshare_api.h"

void print_usage()
{
	printf("Usage: memwatch [arguments]\n");
	printf("\t-l\t\tList every proc registered\n");
	printf("\t-ld <proc name>\t\tList detailed info of proc\n");
	printf("\tv 1.0\n");
}

int main(int argc, char *argv[])
{
	int send, rec, data_size, mode = 0, retvalue, index, i;
	proc_entry *entry;
	char dest_proc[PROC_NAME_SIZE], *datastr, *procptr;

	memset(dest_proc, '\0', PROC_NAME_SIZE);

	/* Parse the parameters */
	if (argc < 2) {
		print_usage();
		exit(2);
	}
	if (!strcmp(argv[1], "-l")) {
		if (argc != 2) {
			print_usage();
			exit(2);
		}
		mode = 1;
	} else if (!strcmp(argv[1], "-ld")) {
		if (argc != 3) {
			print_usage();
			exit(2);
		}
		mode = 2;
		strncpy(dest_proc, argv[2], PROC_NAME_SIZE);
	}
	if (mode == 0) {
		print_usage();
		exit(3);
	}

	/*set_print_level(CH_DEBUG); */
	init_memshare("memsend", 0);

	switch (mode) {
	case 2:
		if ((index = get_proc_index(dest_proc)) != -1) {
			get_proc_info(index, &send, &rec, &data_size, &procptr);
			printf
			    ("Index %d\t\t%s\tSent %d\tReceived %d, Data size of %d\n",
			     index, procptr, send, rec, data_size);
			exit(0);
		}
		printf("No such process %s\n", dest_proc);
		exit(1);
		break;

	case 1:
		for (i = 0; i < NUMBER_OF_PROCS; i++) {
			if (check_proc_entry(i)) {
				get_proc_info(i, &send, &rec, &data_size,
					      &procptr);
				printf
				    ("Index %d\t\t%s\tSent %d\tReceived %d, Data size of %d\n",
				     i, procptr, send, rec, data_size);
			}
		}
		exit(0);
		break;

	default:
		/* No place to be */
		break;
	}

	/* No place to be */
	exit(2);
}
