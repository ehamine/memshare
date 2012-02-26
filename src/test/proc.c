/*
 *      Author: tommy.wiklund
 *
 *      Revision information
 *      2012-02   twik    Initial draft
 *
 *
 */
 
#include <memshare_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int state = 0;
char *procname;

void data_callback(char *proc, char *msg, int len)
{
    if (!strcmp("exit", msg)) {
        printf("%s exiting\n", procname);
        exit(0);
    }
    if (strcmp(procname, "one")) {
        data(proc, msg, len);
    } else {
        switch (state) {
            case 3: {
                if (!strcmp("memshare", msg))
                    printf("Test 4 ok \n");
                state++;
            }
            default: {
            }
        }
    }
}

void signal1_callback(char *proc, int value)
{
    if (strcmp(procname, "one")) {
        signal1(proc, value);
    } else {
        switch (state) {
            case 0: {
                if (value == 2)
                    printf("Test 1 ok \n");
                state++;
            }
            default: {
            }
        }
    }
}

void signal2_callback(char *proc, int value1, int value2)
{
    if (strcmp(procname, "one")) {
        signal2(proc, value1, value2);
    } else {
        switch (state) {
            case 1: {
                if ((value1 == 3) && (value2 == 4))
                    printf("Test 2 ok \n");
                state++;
            }
            default: {
            }
        }
    }
}

void signal3_callback(char *proc, int value1, int value2, int value3)
{
    if (strcmp(procname, "one")) {
        signal3(proc, value1, value2, value3);
    } else {
        switch (state) {
            case 2: {
                if ((value1 == 5) && (value2 == 6) && (value3 == 7))
                    printf("Test 3 ok \n");
                state++;
            }
            default: {
            }
        }
    }
}


int main(int argc, char *argv[])
{
    int val,temp1, temp2;
    procname = malloc(256);
    strncpy(procname, argv[1], 256);
 
    set_print_level(0);
    printf("%s started\n", procname);
    if (signal1("one", 1) != 2) {
        printf("Uninitialized flag malfunctioned\n");
        exit(1);
    }
    
    if (init_memshare(procname, 512)) {
        printf("Failed to init memshare\n");
        exit(1);
    }
    data_register(data_callback);
    signal1_register(signal1_callback);
    signal2_register(signal2_callback);
    signal3_register(signal3_callback);

    /* process one starts */
    if (!strcmp(procname, "one")) {
        /* Send signal1 to process two who will send it back */
        if (val = signal1("two", 2)) {
            printf("signal1 malfunctioned %d\n", val);
        }
        /* Send signal2 to process three who will send it back */
        if (val = signal2("three", 3, 4)) {
            printf("signal2 malfunctioned %d\n", val);
        }
        /* Send signal3 to process two who will send it back */
        if (val = signal3("two", 5, 6, 7)) {
            printf("signal3 malfunctioned %d\n", val);
        }
        /* Send data 'memshare' to process three who will send it back */
        if (val = data("three", "memshare\x0", strlen("memshare")+1)) {
            printf("data malfunctioned %d \n", val);
        }
        /* Send 'exit' to process three who will exit */
        if (val = data("three", "exit\x0", strlen("memshare")+1)) {
            printf("data malfunctioned %d \n", val);
        }
        /* Try to send signal1 to process three who should have exit */
        val = signal1("three", 1);
        if (val == 1) {
            printf("Test 5 ok\n");
        }
        /* Send 'exit' to process two who will exit */
        if (val = data("two", "exit\x0", strlen("memshare")+1)) {
            printf("data malfunctioned %d \n", val);
        }
        /* Try to send signal1 to process two who should have exit */
        val = signal1("two", 2);
        if (val == 1) {
            printf("Test 6 ok\n");
        }
        /* Try to send signal1 to process unknown who never have existed */
        val = signal1("unknown", 2);
        if (val == 1) {
            printf("Test 7 ok\n");
        }
        exit(0);
    }
    printf("%s reaching exit\n", procname);
    while (1)
        sleep(3);
}

