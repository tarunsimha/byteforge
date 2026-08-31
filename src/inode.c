#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/common.h"
#include "../include/inode.h"

int inode_init(inode_t* inode)
{
    if (inode == NULL) return 0;

    memset(inode, 0, sizeof(inode_t));

    inode->mode = 0U;
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
