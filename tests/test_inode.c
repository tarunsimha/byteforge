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

    if (!bitmap_init() || !bitmap_write()) {
        printf("Bitmap initialization failed\n");
        disk_close();
        return 1;
    }

    printf("Filesystem initialized successfully\n");

    if (superblock_get_free_inodes() != TOTAL_INODES - 1U) {
        printf("Initial free inode count is incorrect\n");
        disk_close();
        return 1;
    }

    printf("Initial free inode count is correct\n");

    if (!bitmap_test_inode(0U)) {
        printf("Root inode should be occupied\n");
        disk_close();
        return 1;
    }

    if (!inode_allocate(&inode_num)) {
        printf("Inode allocation failed\n");
        disk_close();
        return 1;
    }

    if (inode_num != 1U) {
        printf("Incorrect inode allocated: %u\n", inode_num);
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(inode_num)) {
        printf("Allocated inode is not marked as occupied\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count did not decrease correctly\n");
        disk_close();
        return 1;
    }

    printf("Inode allocation test successful\n");

    if (!inode_allocate(&inode_num)) {
        printf("Second inode allocation failed\n");
        disk_close();
        return 1;
    }

    if (inode_num != 2U) {
        printf("Incorrect second inode allocated: %u\n", inode_num);
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 3U) {
        printf("Free inode count after second allocation is incorrect\n");
        disk_close();
        return 1;
    }

    printf("Multiple inode allocation test successful\n");

    inode_num = 1U;

    if (!inode_read(&inode, inode_num)) {
        printf("Allocated inode could not be read\n");
        disk_close();
        return 1;
    }

    if (inode.type != INODE_TYPE_FREE) {
        printf("Newly allocated inode is not initialized correctly\n");
        disk_close();
        return 1;
    }

    inode.type = INODE_TYPE_FILE;
    inode.mode = 1U;
    inode.size = 8192U;
    inode.atime = 100U;
    inode.mtime = 200U;
    inode.ctime = 300U;
    inode.link_count = 1U;
    inode.block_count = 2U;
    inode.blocks[0] = DATA_BLOCK_START;
    inode.blocks[1] = DATA_BLOCK_START + 1U;

    if (!inode_is_file(&inode)) {
        printf("Inode file type test failed\n");
        disk_close();
        return 1;
    }

    if (inode_is_directory(&inode)) {
        printf("File incorrectly detected as directory\n");
        disk_close();
        return 1;
    }

    if (!inode_write(&inode, 1U)) {
        printf("Inode write failed\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&read_inode, 1U)) {
        printf("Inode read failed\n");
        disk_close();
        return 1;
    }

    if (memcmp(&inode, &read_inode, sizeof(inode_t)) != 0) {
        printf("Inode data does not match\n");
        disk_close();
        return 1;
    }

    printf("Inode data test successful\n");

    if (inode_free(1U) == 0) {
        printf("Inode free failed\n");
        disk_close();
        return 1;
    }

    if (bitmap_test_inode(1U)) {
        printf("Freed inode is still marked as occupied\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count did not increase correctly\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&read_inode, 1U)) {
        printf("Freed inode could not be read\n");
        disk_close();
        return 1;
    }

    if (read_inode.type != INODE_TYPE_FREE ||
        read_inode.size != 0U ||
        read_inode.link_count != 0U ||
        read_inode.block_count != 0U) {
        printf("Freed inode was not cleared correctly\n");
        disk_close();
        return 1;
    }

    printf("Inode free test successful\n");

    if (inode_free(1U)) {
        printf("Already-free inode was incorrectly freed\n");
        disk_close();
        return 1;
    }

    printf("Duplicate inode free test successful\n");

    if (inode_free(0U)) {
        printf("Root inode was incorrectly freed\n");
        disk_close();
        return 1;
    }

    printf("Root inode protection test successful\n");

    if (!bitmap_write()) {
        printf("Bitmap could not be written\n");
        disk_close();
        return 1;
    }

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

    if (!bitmap_read()) {
        printf("Bitmap could not be read\n");
        disk_close();
        return 1;
    }

    if (bitmap_test_inode(1U)) {
        printf("Freed inode allocation was not persisted\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(2U)) {
        printf("Allocated inode was not persisted\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count was not persisted\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&read_inode, 1U)) {
        printf("Freed inode could not be read after reopening\n");
        disk_close();
        return 1;
    }

    if (read_inode.type != INODE_TYPE_FREE) {
        printf("Freed inode data was not persisted correctly\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&read_inode, 2U)) {
        printf("Second inode could not be read after reopening\n");
        disk_close();
        return 1;
    }

    if (read_inode.type != INODE_TYPE_FREE) {
        printf("Second allocated inode was not initialized correctly\n");
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
