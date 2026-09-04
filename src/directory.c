#include <string.h>
#include <stdint.h>

#include "../include/disk.h"
#include "../include/common.h"
#include "../include/inode.h"
#include "../include/directory.h"
#include "../include/bitmap.h"

int directory_entry_init(directory_entry_t* entry)
{
    if (entry == NULL) return 0;

    entry->inode_num = 0U;
    memset(entry->name, 0, sizeof(entry->name));

    return 1;
}

int directory_read(directory_entry_t* entries, uint32_t block_num)
{
    if (entries == NULL) return 0;
    if (block_num < DATA_BLOCK_START || block_num >= TOTAL_BLOCKS) return 0;

    return disk_read_block(block_num, entries);
}

int directory_write(const directory_entry_t* entries, uint32_t block_num)
{
    if (entries == NULL) return 0;
    if (block_num < DATA_BLOCK_START || block_num >= TOTAL_BLOCKS) return 0;

    return disk_write_block(block_num, entries);
}

int directory_add_entry(uint32_t parent_inode_num, uint32_t inode_num, const char* name)
{
    if (name == NULL) return 0;
    if (name[0] == '\0') return 0;
    if (parent_inode_num >= TOTAL_INODES) return 0;
    if (inode_num >= TOTAL_INODES) return 0;
    if (strlen(name) > MAX_FILENAME) return 0;

    inode_t parent_inode;

    if (!inode_read(&parent_inode, parent_inode_num)) return 0;
    if (!inode_is_directory(&parent_inode)) return 0;

    directory_entry_t entries[BLOCK_SIZE / sizeof(directory_entry_t)];

    for (uint32_t i = 0; i < parent_inode.block_count; i++) {
        uint32_t block_num = parent_inode.blocks[i];

        if (!directory_read(entries, block_num)) return 0;

        for (uint32_t j = 0; j < BLOCK_SIZE / sizeof(directory_entry_t); j++) {

            if (entries[j].inode_num != 0U && strcmp(entries[j].name, name) == 0) return 0;

            if (entries[j].inode_num == 0U) {
                entries[j].inode_num = inode_num;
                strcpy(entries[j].name, name);

                if (!directory_write(entries, block_num)) return 0;
                return 1;
            }
        }
    }

    if (parent_inode.block_count >= DIRECT_BLOCKS) return 0;

    uint32_t new_block;

    if (!bitmap_find_free_block(&new_block)) return 0;

    if (!bitmap_set_block(new_block)) return 0;

    for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(directory_entry_t); i++) {
        if (!directory_entry_init(&entries[i])) {
            bitmap_clear_block(new_block);
            return 0;
        }
    }

    entries[0].inode_num = inode_num;
    strcpy(entries[0].name, name);

    if (!directory_write(entries, new_block)) {
        bitmap_clear_block(new_block);
        return 0;
    }

    parent_inode.blocks[parent_inode.block_count] = new_block;
    parent_inode.block_count++;

    if (!inode_write(&parent_inode, parent_inode_num)) {
        bitmap_clear_block(new_block);
        return 0;
    }

    return 1;
}

int directory_create(uint32_t parent_inode_num, uint32_t* inode_num, const char* name)
{
    if (inode_num == NULL) return 0;
    if (name == NULL) return 0;
    if (name[0] == '\0') return 0;
    if (strlen(name) > MAX_FILENAME) return 0;
    if (parent_inode_num >= TOTAL_INODES) return 0;

    inode_t parent_inode;

    if (!inode_read(&parent_inode, parent_inode_num)) return 0;
    if (!inode_is_directory(&parent_inode)) return 0;

    uint32_t new_inode_num;
    uint32_t block_num;

    if (!inode_allocate(&new_inode_num)) return 0;

    if (!bitmap_find_free_block(&block_num)) {
        inode_free(new_inode_num);
        return 0;
    }

    if (!bitmap_set_block(block_num)) {
        inode_free(new_inode_num);
        return 0;
    }

    inode_t inode;

    if (!inode_init(&inode)) {
        bitmap_clear_block(block_num);
        inode_free(new_inode_num);
        return 0;
    }

    inode.type = INODE_TYPE_DIRECTORY;
    inode.link_count = 1U;
    inode.block_count = 1U;
    inode.blocks[0] = block_num;

    directory_entry_t entries[BLOCK_SIZE / sizeof(directory_entry_t)];

    for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(directory_entry_t); i++) {
        if (!directory_entry_init(&entries[i])) {
            bitmap_clear_block(block_num);
            inode_free(new_inode_num);
            return 0;
        }
    }

    entries[0].inode_num = new_inode_num;
    strcpy(entries[0].name, ".");

    entries[1].inode_num = parent_inode_num;
    strcpy(entries[1].name, "..");

    if (!directory_write(entries, block_num)) {
        bitmap_clear_block(block_num);
        inode_free(new_inode_num);
        return 0;
    }

    if (!inode_write(&inode, new_inode_num)) {
        bitmap_clear_block(block_num);
        inode_free(new_inode_num);
        return 0;
    }

    if (!directory_add_entry(parent_inode_num, new_inode_num, name)) {
        bitmap_clear_block(block_num);
        inode_free(new_inode_num);
        return 0;
    }

    *inode_num = new_inode_num;

    return 1;
}
