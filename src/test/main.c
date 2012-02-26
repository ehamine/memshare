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


int main(int argc, char *argv[])
{
    system("./proc three &");
    sleep(1);
    system("./proc two &");
    sleep(1);
    system("./proc one &");
    sleep(1);
    return 0;
}
