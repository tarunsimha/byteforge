#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "../include/common.h"
#include <stdint.h>

typedef struct {
    uint32_t inode_num;
    char name[MAX_FILENAME + 1];
} directory_entry_t;

_Static_assert(sizeof(directory_entry_t) == 256, "directory_entry_t must be exactly 256 bytes");

int directory_entry_init(directory_entry_t* entry);

int directory_read(directory_entry_t* entries, uint32_t block_num);
int directory_write(const directory_entry_t* entries, uint32_t block_num);

int directory_add_entry(uint32_t parent_inode_num, uint32_t inode_num, const char* name);
int directory_create(uint32_t parent_inode_num, uint32_t* inode_num, const char* name);

#endif
