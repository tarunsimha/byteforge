#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/superblock.h"
#include "../include/bitmap.h"
#include "../include/inode.h"
#include "../include/common.h"

int main(void)
{
    inode_t inode;
    inode_t read_inode;
    inode_t root_inode;

    if (!disk_create("virtual_disk.img")) {
        printf("Disk could not be created\n");
        return 1;
    }

    printf("Disk created successfully\n");

    if (!disk_open("virtual_disk.img")) {
        printf("Disk could not be opened\n");
        return 1;
    }

    printf("Disk opened successfully\n");

    if (!superblock_init() || !superblock_write()) {
        printf("Superblock initialization failed\n");
        disk_close();
        return 1;
    }

    if (!bitmap_init() || !bitmap_write()) {
        printf("Bitmap initialization failed\n");
        disk_close();
        return 1;
    }

    printf("Filesystem initialized successfully\n");

    if (!inode_init(&inode)) {
        printf("Inode initialization failed\n");
        disk_close();
        return 1;
    }

    inode.mode = 1U;
    inode.size = 8192U;
    inode.atime = 100U;
    inode.mtime = 200U;
    inode.ctime = 300U;
    inode.link_count = 1U;
    inode.block_count = 2U;
    inode.blocks[0] = DATA_BLOCK_START;
    inode.blocks[1] = DATA_BLOCK_START + 1U;

    if (!inode_write(1U, &inode)) {
        printf("Inode write failed\n");
        disk_close();
        return 1;
    }

    printf("Inode write successful\n");

    if (!inode_read(1U, &read_inode)) {
        printf("Inode read failed\n");
        disk_close();
        return 1;
    }

    if (memcmp(&inode, &read_inode, sizeof(inode_t)) != 0) {
        printf("Inode data does not match\n");
        disk_close();
        return 1;
    }

    printf("Inode read/write test successful\n");

    if (!inode_read(0U, &root_inode)) {
        printf("Root inode read failed\n");
        disk_close();
        return 1;
    }

    if (root_inode.mode != 0U ||
        root_inode.size != 0U ||
        root_inode.link_count != 0U ||
        root_inode.block_count != 0U) {
        printf("Root inode was corrupted\n");
        disk_close();
        return 1;
    }

    printf("Inode isolation test successful\n");

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("Disk closed successfully\n");

    if (!disk_open("virtual_disk.img")) {
        printf("Disk could not be reopened\n");
        return 1;
    }

    printf("Disk reopened successfully\n");

    if (!inode_read(1U, &read_inode)) {
        printf("Inode could not be read after reopening\n");
        disk_close();
        return 1;
    }

    if (memcmp(&inode, &read_inode, sizeof(inode_t)) != 0) {
        printf("Inode data was not persisted\n");
        disk_close();
        return 1;
    }

    printf("Inode persistence test successful\n");

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("All inode tests passed\n");

    return 0;
}
