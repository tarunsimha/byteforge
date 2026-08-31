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

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START) {
        printf("Initial free block count is incorrect\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 1U) {
        printf("Initial free inode count is incorrect\n");
        disk_close();
        return 1;
    }

    printf("Initial free counts are correct\n");

    if (!bitmap_test_block(SUPERBLOCK_BLOCK) ||
        !bitmap_test_block(BLOCK_BITMAP_START) ||
        !bitmap_test_block(INODE_BITMAP_START) ||
        !bitmap_test_block(INODE_TABLE_START)) {
        printf("Metadata blocks are not marked as used\n");
        disk_close();
        return 1;
    }

    printf("Metadata block test successful\n");

    if (bitmap_test_block(DATA_BLOCK_START)) {
        printf("Data block should initially be free\n");
        disk_close();
        return 1;
    }

    printf("Initial data block test successful\n");

    if (!bitmap_set_block(DATA_BLOCK_START)) {
        printf("Could not set block\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START - 1U) {
        printf("Free block count did not decrease correctly\n");
        disk_close();
        return 1;
    }

    printf("Block allocation test successful\n");

    if (!bitmap_set_block(DATA_BLOCK_START)) {
        printf("Could not set already-used block\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START - 1U) {
        printf("Free block count changed after duplicate allocation\n");
        disk_close();
        return 1;
    }

    printf("Duplicate block allocation test successful\n");

    if (!bitmap_clear_block(DATA_BLOCK_START)) {
        printf("Could not clear block\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START) {
        printf("Free block count did not increase correctly\n");
        disk_close();
        return 1;
    }

    printf("Block free test successful\n");

    if (!bitmap_clear_block(DATA_BLOCK_START)) {
        printf("Could not clear already-free block\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START) {
        printf("Free block count changed after duplicate free\n");
        disk_close();
        return 1;
    }

    printf("Duplicate block free test successful\n");

    if (!bitmap_set_inode(1)) {
        printf("Could not set inode\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count did not decrease correctly\n");
        disk_close();
        return 1;
    }

    printf("Inode allocation test successful\n");

    if (!bitmap_set_inode(1)) {
        printf("Could not set already-used inode\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count changed after duplicate allocation\n");
        disk_close();
        return 1;
    }

    printf("Duplicate inode allocation test successful\n");

    if (!bitmap_clear_inode(1)) {
        printf("Could not clear inode\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 1U) {
        printf("Free inode count did not increase correctly\n");
        disk_close();
        return 1;
    }

    printf("Inode free test successful\n");

    if (!bitmap_clear_inode(1)) {
        printf("Could not clear already-free inode\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 1U) {
        printf("Free inode count changed after duplicate free\n");
        disk_close();
        return 1;
    }

    printf("Duplicate inode free test successful\n");

    if (!bitmap_find_free_block(&block_num)) {
        printf("Could not find free block\n");
        disk_close();
        return 1;
    }

    if (block_num != DATA_BLOCK_START) {
        printf("Incorrect free block found\n");
        disk_close();
        return 1;
    }

    printf("Free block search successful\n");

    if (!bitmap_find_free_inode(&inode_num)) {
        printf("Could not find free inode\n");
        disk_close();
        return 1;
    }

    if (inode_num != 1U) {
        printf("Incorrect free inode found\n");
        disk_close();
        return 1;
    }

    printf("Free inode search successful\n");

    if (!bitmap_set_block(DATA_BLOCK_START) ||
        !bitmap_set_inode(1)) {
        printf("Could not prepare persistence test\n");
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

    if (superblock_get_free_blocks() != TOTAL_BLOCKS - DATA_BLOCK_START - 1U) {
        printf("Free block count was not persisted\n");
        disk_close();
        return 1;
    }

    if (superblock_get_free_inodes() != TOTAL_INODES - 2U) {
        printf("Free inode count was not persisted\n");
        disk_close();
        return 1;
    }

    printf("Bitmap persistence successful\n");

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("All bitmap tests passed\n");

    return 0;
}
