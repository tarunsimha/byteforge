#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "../include/common.h"

int disk_create(const char* filename);
int disk_open(const char* filename);
int disk_close(void);

int disk_read_block(uint32_t block_num, void* buffer);
int disk_write_block(uint32_t block_num, const void* buffer);

#endif
