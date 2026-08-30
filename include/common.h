#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

// Disk configuration
#define DISK_SIZE       (1024ULL * 1024ULL * 1024ULL)
#define BLOCK_SIZE      4096U
#define TOTAL_BLOCKS    262144U

// Inode configuration
#define INODE_SIZE      1024U
#define TOTAL_INODES    65536U

// Filesystem layout
#define SUPERBLOCK_BLOCK        0U
#define BLOCK_BITMAP_START      1U
#define BLOCK_BITMAP_BLOCKS     8U
#define INODE_BITMAP_START      (BLOCK_BITMAP_START + BLOCK_BITMAP_BLOCKS)
#define INODE_BITMAP_BLOCKS     2U
#define INODE_TABLE_START       (INODE_BITMAP_START + INODE_BITMAP_BLOCKS)
#define INODE_TABLE_BLOCKS      16384U
#define DATA_BLOCK_START        (INODE_TABLE_START + INODE_TABLE_BLOCKS)

// Other filesystem limits
#define DIRECT_BLOCKS   256U
#define MAX_FILE_SIZE   (DIRECT_BLOCKS * BLOCK_SIZE)
#define MAX_OPEN_FILES  256U
#define MAX_FILENAME    251U

// ByteForge identification
#define MAGIC_NUMBER    0x42594653U
#define VERSION         1U

typedef enum {
    FS_CLEAN = 0,
    FS_DIRTY = 1
} fs_state_t;

#endif
