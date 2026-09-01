#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include "../include/common.h"

typedef struct {
    // Metadata
    uint32_t mode;
    uint32_t type;
    uint32_t size;

    // Time related data
    uint64_t atime; // Access time
    uint64_t mtime; // Modified time
    uint64_t ctime; // Created time

    // Counters
    uint32_t link_count;
    uint32_t block_count;

    // Direct block pointers
    uint32_t blocks[256];

    uint8_t reserved[976];
} inode_t;

typedef enum {
    INODE_TYPE_FREE = 0,
    INODE_TYPE_FILE = 1,
    INODE_TYPE_DIRECTORY = 2
} inode_type_t;

_Static_assert(sizeof(inode_t) == INODE_SIZE, "inode_t must be exactly INODE_SIZE bytes");

int inode_allocate(uint32_t* inode_num); // Pointer bcuz returning int is for success or failure
int inode_free(uint32_t inode_num);

int inode_init(inode_t* inode);
int inode_read(inode_t* inode, uint32_t inode_num);
int inode_write(const inode_t* inode, uint32_t inode_num);

int inode_is_free(const inode_t* inode);
int inode_is_file(const inode_t* inode);
int inode_is_directory(const inode_t* inode);

#endif
