#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

int bitmap_init(void);

int bitmap_set_block(uint32_t block_num);
int bitmap_clear_block(uint32_t block_num);
int bitmap_test_block(uint32_t block_num);

int bitmap_set_inode(uint32_t inode_num);
int bitmap_clear_inode(uint32_t inode_num);
int bitmap_test_inode(uint32_t inode_num);

int bitmap_find_free_block(uint32_t* block_num);
int bitmap_find_free_inode(uint32_t* inode_num);

int bitmap_write(void);
int bitmap_read(void);

#endif
