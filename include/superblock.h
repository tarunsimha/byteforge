#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t total_inodes;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t root_inode;
    uint32_t filesystem_state;
} superblock_t;

int superblock_init(void);
int superblock_read(void);
int superblock_write(void);
int superblock_validate(void);

void superblock_mark_dirty(void);
void superblock_mark_clean(void);
void superblock_change_free_blocks(uint32_t blocks);
void superblock_change_free_inodes(uint32_t inodes);
uint32_t superblock_get_free_blocks(void);
uint32_t superblock_get_free_inodes(void);
#endif
