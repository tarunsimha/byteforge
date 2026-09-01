#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/common.h"
#include "../include/bitmap.h"
#include "../include/inode.h"

int inode_allocate(uint32_t* inode_num)
{
    if (inode_num == NULL) return 0;

    uint32_t num;
    if (!bitmap_find_free_inode(&num)) return 0;

    inode_t inode;
    if (!inode_init(&inode)) return 0;
    if (!bitmap_set_inode(num)) return 0;

    if (!bitmap_write()) {
        bitmap_clear_inode(num);
        return 0;
    }

    if (!inode_write(&inode, num)) {
        bitmap_clear_inode(num);
        bitmap_write();
        return 0;
    }

    *inode_num = num;

    return 1;
}

int inode_free(uint32_t inode_num)
{
    if (inode_num == 0U) return 0;
    if (inode_num >= TOTAL_INODES) return 0;
    if (!bitmap_test_inode(inode_num)) return 0;

    inode_t inode;

    if (!inode_init(&inode)) return 0;
    if (!inode_write(&inode, inode_num)) return 0;
    if (!bitmap_clear_inode(inode_num)) return 0;
    if (!bitmap_write()) return 0;

    return 1;
}

int inode_init(inode_t* inode)
{
    if (inode == NULL) return 0;

    memset(inode, 0, sizeof(inode_t));

    inode->mode = 0U;
    inode->type = INODE_TYPE_FREE;
    inode->size = 0U;
    inode->atime = 0U;
    inode->mtime = 0U;
    inode->ctime = 0U;
    inode->link_count = 0U;
    inode->block_count = 0U;

    return 1;
}

int inode_read(inode_t* inode, uint32_t inode_num)
{
    if (inode == NULL) return 0;
    if (inode_num >= TOTAL_INODES) return 0;

    uint32_t table_block = INODE_TABLE_START + (inode_num / INODES_PER_BLOCK);
    uint32_t inode_index = inode_num % INODES_PER_BLOCK;
    uint8_t buffer[BLOCK_SIZE];

    if (!disk_read_block(table_block, buffer)) return 0;

    memcpy(inode, buffer + (inode_index * INODE_SIZE), INODE_SIZE);

    return 1;
}

int inode_write(const inode_t* inode, uint32_t inode_num)
{
    if (inode == NULL) return 0;
    if (inode_num >= TOTAL_INODES) return 0;

    uint32_t table_block = INODE_TABLE_START + (inode_num / INODES_PER_BLOCK);
    uint32_t inode_index = inode_num % INODES_PER_BLOCK;
    uint8_t buffer[BLOCK_SIZE];

    if (!disk_read_block(table_block, buffer)) return 0;

    memcpy(buffer + (inode_index * INODE_SIZE), inode, INODE_SIZE);

    if (!disk_write_block(table_block, buffer)) return 0;

    return 1;
}

int inode_is_free(const inode_t* inode)
{
    if (inode == NULL) return 0;
    return inode->type == INODE_TYPE_FREE;
}

int inode_is_file(const inode_t* inode)
{
    if (inode == NULL) return 0;
    return inode->type == INODE_TYPE_FILE;
}

int inode_is_directory(const inode_t* inode)
{
    if (inode == NULL) return 0;
    return inode->type == INODE_TYPE_DIRECTORY;
}
