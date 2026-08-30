#include <stdio.h>
#include "../include/disk.h"

int main()
{
    printf("Creating 1GiB file now\n");
    int flag = disk_create("virtual_disk.img");
    if (flag) {
        printf("File creation was successful\n");
    } else {
        printf("File could not be created\n");
    }

    flag = disk_open("virtual_disk.img");
    if (flag) {
        printf("Disk opened successfuly\n");
    } else {
        printf("Disk could not be opened\n");
    }
}
