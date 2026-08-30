#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/disk.h"
#include "../include/common.h"

#define BUFFER_SIZE (1024 * 1024)

static FILE* disk_file = NULL;
static int disk_opened = 0;

int disk_create(const char* filename)
{
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    char *buffer = calloc(1, BUFFER_SIZE);
    if (buffer == NULL) {
        fclose(fp);
        return 0;
    }

    long long unsigned written = 0ULL;

    while (written < DISK_SIZE) {
        long long unsigned remaining = DISK_SIZE - written;
        size_t bytes_to_write = BUFFER_SIZE;

        if (remaining < BUFFER_SIZE) {
            bytes_to_write = (size_t) remaining;
        }

        size_t bytes_written = fwrite(buffer, 1, bytes_to_write, fp);

        if (bytes_written != bytes_to_write) {
            free(buffer);
            fclose(fp);
            fp = NULL;
            return 0;
        }

        written += bytes_written;
    }

    free(buffer);
    fclose(fp);
    fp = NULL;
    return 1;
}

int disk_open(const char* filename)
{
    if (disk_opened) {
        return 0;
    }

    FILE* fp = fopen(filename, "r+b");

    if (fp == NULL) {
        return 0;
    }

    disk_file = fp;
    disk_opened = 1;

    return 1;
}

int disk_close(void)
{
    if (disk_file == NULL) {
        return 0;
    }

    int flag = fclose(disk_file);
    if (flag) {
        return 0;
    }

    disk_file = NULL;
    return 1;
}

int disk_read_block(uint32_t block_num, void* buffer)
{
    if (!disk_opened || disk_file == NULL) return 0;
    if (block_num >= TOTAL_BLOCKS) return 0;
    if (buffer == NULL) return 0;

    uint64_t offset = (uint64_t) block_num * BLOCK_SIZE;

    int flag = fseek(disk_file, (long) offset, SEEK_SET);
    if (flag != 0) return 0;

    size_t bytes_read = fread(buffer, 1, BLOCK_SIZE, disk_file);
    if (bytes_read != BLOCK_SIZE) return 0;

    return 1;
}

int disk_write_block(uint32_t block_num, const void* buffer)
{
    if (!disk_opened || disk_file == NULL) return 0;
    if (block_num >= TOTAL_BLOCKS) return 0;
    if (buffer == NULL) return 0;

    uint64_t offset = (uint64_t) block_num * BLOCK_SIZE;

    int flag = fseek(disk_file, (long) offset, SEEK_SET);
    if (flag != 0) return 0;

    size_t bytes_written = fwrite(buffer, 1, BLOCK_SIZE, disk_file);
    if (bytes_written != BLOCK_SIZE) return 0;

    return 1;
}
