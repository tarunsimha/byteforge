#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/bitmap.h"
#include "../include/common.h"

#define BLOCK_BITMAP_SIZE (BLOCK_BITMAP_BLOCKS * BLOCK_SIZE)
#define INODE_BITMAP_SIZE (INODE_BITMAP_BLOCKS * BLOCK_SIZE)

static uint8_t block_bitmap[BLOCK_BITMAP_SIZE];
static uint8_t inode_bitmap[INODE_BITMAP_SIZE];

static void bitmap_set(uint8_t* bitmap, uint32_t index)
{
    uint32_t byte_index = index / 8U;
    uint32_t bit_index = index % 8U;

    bitmap[byte_index] |= (uint8_t) (1U << bit_index);
}

static void bitmap_clear(uint8_t* bitmap, uint32_t index)
{
    uint32_t byte_index = index / 8U;
    uint32_t bit_index = index % 8U;

    bitmap[byte_index] &= (uint8_t) ~(1U << bit_index);
}

static int bitmap_test(const uint8_t* bitmap, uint32_t index)
{
    uint32_t byte_index = index / 8U;
    uint32_t bit_index = index % 8U;

    return (bitmap[byte_index] & (uint8_t)(1U << bit_index)) != 0;
}

int bitmap_init(void)
{
    memset(block_bitmap, 0, sizeof(block_bitmap));
    memset(inode_bitmap, 0, sizeof(inode_bitmap));

    // Reserve filesystem metadata blocks
    for (uint32_t i = 0; i < DATA_BLOCK_START; i++) {
        bitmap_set_block(i);
    }

    // Reserve inode 0 for the root inode
    bitmap_set_inode(0);

    return 1;
}

int bitmap_set_block(uint32_t block_num)
{
    if (block_num >= TOTAL_BLOCKS) return 0;

    bitmap_set(block_bitmap, block_num);
    return 1;
}

int bitmap_clear_block(uint32_t block_num)
{
    if (block_num >= TOTAL_BLOCKS) return 0;

    bitmap_clear(block_bitmap, block_num);
    return 1;
}

int bitmap_test_block(uint32_t block_num)
{
    if (block_num >= TOTAL_BLOCKS) return 0;
    return bitmap_test(block_bitmap, block_num);
}

int bitmap_set_inode(uint32_t inode_num)
{
    if (inode_num >= TOTAL_INODES) return 0;

    bitmap_set(inode_bitmap, inode_num);
    return 1;
}

int bitmap_clear_inode(uint32_t inode_num)
{
    if (inode_num >= TOTAL_INODES) return 0;

    bitmap_clear(inode_bitmap, inode_num);
    return 1;
}

int bitmap_test_inode(uint32_t inode_num)
{
    if (inode_num >= TOTAL_INODES) return 0;
    return bitmap_test(inode_bitmap, inode_num);
}

int bitmap_find_free_block(uint32_t *block_num)
{
    if (block_num == NULL) return 0;

    for (uint32_t i = DATA_BLOCK_START; i < TOTAL_BLOCKS; i++) {
        if (!bitmap_test(block_bitmap, i)) {
            *block_num = i;
            return 1;
        }
    }

    return 0;
}

int bitmap_find_free_inode(uint32_t *inode_num)
{
    if (inode_num == NULL) return 0;

    for (uint32_t i = 1; i < TOTAL_INODES; i++) {
        if (!bitmap_test(inode_bitmap, i)) {
            *inode_num = i;
            return 1;
        }
    }

    return 0;
}

int bitmap_write(void)
{
    for (uint32_t i = 0; i < BLOCK_BITMAP_BLOCKS; i++) {
        if (!disk_write_block(BLOCK_BITMAP_START + i, block_bitmap + (i * BLOCK_SIZE))) {
            return 0;
        }
    }

    for (uint32_t i = 0; i < INODE_BITMAP_BLOCKS; i++) {
        if (!disk_write_block(INODE_BITMAP_START + i, inode_bitmap + (i * BLOCK_SIZE))) {
            return 0;
        }
    }

    return 1;
}

int bitmap_read(void)
{
    for (uint32_t i = 0; i < BLOCK_BITMAP_BLOCKS; i++) {
        if (!disk_read_block(BLOCK_BITMAP_START + i, block_bitmap + (i * BLOCK_SIZE))) {
            return 0;
        }
    }

    for (uint32_t i = 0; i < INODE_BITMAP_BLOCKS; i++) {
        if (!disk_read_block(INODE_BITMAP_START + i, inode_bitmap + (i * BLOCK_SIZE))) {
            return 0;
        }
    }

    return 1;
}
