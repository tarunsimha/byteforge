#include <stdio.h>
#include <stdint.h>

#include "../include/disk.h"
#include "../include/superblock.h"
#include "../include/bitmap.h"
#include "../include/common.h"

int main(void)
{
    uint32_t block_num;
    uint32_t inode_num;

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

    printf("Superblock written successfully\n");

    if (!bitmap_init() || !bitmap_write()) {
        printf("Bitmap initialization failed\n");
        disk_close();
        return 1;
    }

    printf("Bitmaps written successfully\n");

    if (!bitmap_set_block(DATA_BLOCK_START)) {
        printf("Could not set block\n");
        disk_close();
        return 1;
    }

    if (!bitmap_set_inode(1)) {
        printf("Could not set inode\n");
        disk_close();
        return 1;
    }

    if (!bitmap_write()) {
        printf("Bitmap update failed\n");
        disk_close();
        return 1;
    }

    printf("Bitmap update successful\n");

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

    if (!superblock_read() || !superblock_validate()) {
        printf("Superblock validation failed\n");
        disk_close();
        return 1;
    }

    printf("Superblock validation successful\n");

    if (!bitmap_read()) {
        printf("Bitmap could not be read\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_block(DATA_BLOCK_START)) {
        printf("Block allocation was not persisted\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(1)) {
        printf("Inode allocation was not persisted\n");
        disk_close();
        return 1;
    }

    if (!bitmap_find_free_block(&block_num)) {
        printf("Could not find free block\n");
        disk_close();
        return 1;
    }

    if (!bitmap_find_free_inode(&inode_num)) {
        printf("Could not find free inode\n");
        disk_close();
        return 1;
    }

    printf("Next free block: %u\n", block_num);
    printf("Next free inode: %u\n", inode_num);

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("All tests passed\n");

    return 0;
}
