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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memshare.h"
#include "memshare_api.h"

void print_usage()
{
        printf("Usage: memsend [arguments]\n");
        printf("\t-s1 <proc name> <integer>\n");
        printf("\t-s2 <proc name> <integer> <integer>\n");
        printf("\t-s3 <proc name> <integer> <integer> <integer>\n");
        printf("\t-d <proc name> <data>\n");
        printf("\tv 1.0\n");
}

int main(int argc, char *argv[])
{
        int value1, value2, value3, mode = 0, retvalue, data_size;
        char dest_proc[PROC_NAME_SIZE], *datastr;

        memset(dest_proc, '\0', PROC_NAME_SIZE);
        /* Parse the parameters */
        if (argc < 4) {
                print_usage();
                exit(2);
        }
        if (!strcmp(argv[1], "-s1")) {
                if (argc != 4) {
                        print_usage();
                        exit(2);
                }
                mode = 1;
                value1 = atoi(argv[3]);
                strncpy(dest_proc, argv[2], PROC_NAME_SIZE);
        } else if (!strcmp(argv[1], "-s2")) {
                if (argc != 5) {
                        print_usage();
                        exit(2);
                }
                mode = 2;
                value1 = atoi(argv[3]);
                value2 = atoi(argv[4]);
                strncpy(dest_proc, argv[2], PROC_NAME_SIZE);
        } else if (!strcmp(argv[1], "-s3")) {
                if (argc != 6) {
                        print_usage();
                        exit(2);
                }
                mode = 3;
                value1 = atoi(argv[3]);
                value2 = atoi(argv[4]);
                value3 = atoi(argv[5]);
                strncpy(dest_proc, argv[2], PROC_NAME_SIZE);
        } else if (!strcmp(argv[1], "-d")) {
                if (argc != 4) {
                        print_usage();
                        exit(2);
                }
                mode = 4;
                data_size = strlen(argv[3]);
                strncpy(dest_proc, argv[2], PROC_NAME_SIZE);
                datastr = malloc(data_size);
                strncpy(datastr, argv[3], data_size);
        }
        if (mode == 0) {
                print_usage();
                exit(3);
        }

        /*set_print_level(CH_DEBUG); */
        init_memshare("memsend", 0, 0);

        switch (mode) {
        case 1:
                retvalue = signal1(dest_proc, value1);
                break;

        case 2:
                retvalue = signal2(dest_proc, value1, value2);
                break;

        case 3:
                retvalue = signal3(dest_proc, value1, value2, value3);
                break;

        case 4:
                retvalue = data(dest_proc, datastr, data_size);
                free(datastr);
                break;

        default:
                /* No place to be */
                break;
        }

        if (retvalue == 0)
                exit(0);

        if (retvalue == 1) {
                printf("The destination process %s is not present!\n",
                       dest_proc);
                exit(1);
        }

        /* No place to be */
        exit(2);
}
