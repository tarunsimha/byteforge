#include <stdio.h>
#include <stdlib.h>

#include "../include/disk.h"

#define VFS_DISK_SIZE (1024LL * 1024LL * 1024LL)
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

    long long written = 0LL;

    while (written < VFS_DISK_SIZE) {
        long long remaining = VFS_DISK_SIZE - written;
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
