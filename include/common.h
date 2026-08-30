#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define DISK_SIZE       (1024ULL * 1024ULL * 1024ULL)
#define BLOCK_SIZE      4096
#define TOTAL_BLOCKS    262144

#define INODE_SIZE      1024
#define TOTAL_INODES      65536
#define DIRECT_BLOCKS   256
#define MAX_FILE_SIZE   (DIRECT_BLOCKS * BLOCK_SIZE)

#define MAX_OPEN_FILES  256
#define MAX_FILENAME    251

#define MAGIC_NUMBER    0x42594653
#define VERSION         1U

typedef enum {
    FS_CLEAN = 0,
    FS_DIRTY = 1
} fs_state_t;

#endif
