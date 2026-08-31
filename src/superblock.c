#include <string.h>

#include "../include/disk.h"
#include "../include/superblock.h"
#include "../include/common.h"

static superblock_t superblock;

int superblock_init(void)
{
    superblock.magic = MAGIC_NUMBER;
    superblock.version = VERSION;
    superblock.block_size = BLOCK_SIZE;
    superblock.total_blocks = TOTAL_BLOCKS;
    superblock.total_inodes = TOTAL_INODES;
    superblock.free_blocks = TOTAL_BLOCKS - DATA_BLOCK_START; // Accounted for block bitmap
    superblock.free_inodes = TOTAL_INODES - 1U; // Accounted for root inode
    superblock.root_inode = 0U;
    superblock.filesystem_state = FS_CLEAN;

    return 1;
}

int superblock_write(void)
{
    char buffer[BLOCK_SIZE] = {0};

    memcpy(buffer, &superblock, sizeof(superblock));

    return disk_write_block(0, buffer);
}

int superblock_read(void)
{
    char buffer[BLOCK_SIZE];

    if (!disk_read_block(0, buffer)) {
        return 0;
    }

    memcpy(&superblock, buffer, sizeof(superblock));

    return 1;
}

int superblock_validate(void)
{
    if (superblock.magic != MAGIC_NUMBER) return 0;
    if (superblock.version != VERSION) return 0;
    if (superblock.block_size != BLOCK_SIZE) return 0;
    if (superblock.total_blocks != TOTAL_BLOCKS) return 0;
    if (superblock.total_inodes != TOTAL_INODES) return 0;
    if (superblock.root_inode != 0U) return 0;

    if (superblock.free_blocks > superblock.total_blocks) return 0;
    if (superblock.free_inodes > superblock.total_inodes) return 0;

    if (superblock.filesystem_state != FS_CLEAN && superblock.filesystem_state != FS_DIRTY) return 0;

    return 1;
}

void superblock_mark_clean(void)
{
    superblock.filesystem_state = FS_CLEAN;
    superblock_write();
}

void superblock_mark_dirty(void)
{
    superblock.filesystem_state = FS_DIRTY;
    superblock_write();
}

void superblock_change_free_blocks(uint32_t blocks)
{
    if (blocks > superblock.total_blocks) return;

    superblock.free_blocks = blocks;
    superblock_write();
}

void superblock_change_free_inodes(uint32_t inodes)
{
    if (inodes > superblock.total_inodes) return;

    superblock.free_inodes = inodes;
    superblock_write();
}

uint32_t superblock_get_free_blocks(void)
{
    return superblock.free_blocks;
}

uint32_t superblock_get_free_inodes(void)
{
    return superblock.free_inodes;
}
