#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/superblock.h"
#include "../include/common.h"

int main()
{
    int flag = 0;

    /*
    printf("Creating 1GiB file now\n");
    flag = disk_create("virtual_disk.img");
    if (flag) {
        printf("File creation was successful\n");
    } else {
        printf("File could not be created\n");
    }
    */

    flag = disk_open("virtual_disk.img");
    if (flag) {
        printf("Disk opened successfuly\n");
    } else {
        printf("Disk could not be opened\n");
    }

    /*
    uint32_t block_num = 0;
    char write_buffer[BLOCK_SIZE];
    memset(write_buffer, 'A', BLOCK_SIZE);
    flag = disk_write_block(block_num, write_buffer);
    if (!flag) {
        printf("Could not write data to block number %d\n", block_num);
    } else {
        printf("Data write to block number %d was successful\n", block_num);
    }

    block_num = 1;
    memset(write_buffer, 'B', BLOCK_SIZE);
    flag = disk_write_block(block_num, write_buffer);
    if (!flag) {
        printf("Could not write data to block number %d\n", block_num);
    } else {
        printf("Data write to block number %d was successful\n", block_num);
    }

    block_num = 2;
    memset(write_buffer, 'C', BLOCK_SIZE);
    flag = disk_write_block(block_num, write_buffer);
    if (!flag) {
        printf("Could not write data to block number %d\n", block_num);
    } else {
        printf("Data write to block number %d was successful\n", block_num);
    }

    superblock_init();
    superblock_write();
    */

    superblock_read();
    flag = superblock_validate();
    if (flag) {
        printf("The given file is a byfs system\n");
    } else {
        printf("The given file is not a byfs system\n");
    }

    flag = disk_close();
    if (flag) {
        printf("Disk closed successfuly\n");
    } else {
        printf("Disk could not be closed\n");
    }
}
