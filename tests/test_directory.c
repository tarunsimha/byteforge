#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../include/disk.h"
#include "../include/superblock.h"
#include "../include/bitmap.h"
#include "../include/inode.h"
#include "../include/directory.h"
#include "../include/common.h"

int main(void)
{
    directory_entry_t entries[BLOCK_SIZE / sizeof(directory_entry_t)];
    directory_entry_t read_entries[BLOCK_SIZE / sizeof(directory_entry_t)];

    inode_t root_inode;
    inode_t created_inode;

    uint32_t inode_num;
    uint32_t block_num;

    if (!disk_create("virtual_disk.img")) {
        printf("Disk could not be created\n");
        return 1;
    }

    printf("Disk created successfully\n");

    if (!disk_open("virtual_disk.img")) {
        printf("Disk could not be opened\n");
        return 1;
    }

    printf("Disk opened successfully\n");

    if (!superblock_init() || !superblock_write()) {
        printf("Superblock initialization failed\n");
        disk_close();
        return 1;
    }

    if (!bitmap_init() || !bitmap_write()) {
        printf("Bitmap initialization failed\n");
        disk_close();
        return 1;
    }

    printf("Filesystem initialized successfully\n");

    /*
     * Directory entry / block read-write test
     */

    for (uint32_t i = 0;
         i < BLOCK_SIZE / sizeof(directory_entry_t);
         i++) {

        if (!directory_entry_init(&entries[i])) {
            printf("Directory entry initialization failed\n");
            disk_close();
            return 1;
        }

        entries[i].inode_num = i + 1U;
        snprintf(entries[i].name, sizeof(entries[i].name), "entry%u", i);
    }

    block_num = DATA_BLOCK_START;

    if (!directory_write(entries, block_num)) {
        printf("Directory block write failed\n");
        disk_close();
        return 1;
    }

    printf("Directory block write successful\n");

    if (!directory_read(read_entries, block_num)) {
        printf("Directory block read failed\n");
        disk_close();
        return 1;
    }

    if (memcmp(entries, read_entries, sizeof(entries)) != 0) {
        printf("Directory block data does not match\n");
        disk_close();
        return 1;
    }

    printf("Directory block read/write test successful\n");

    /*
     * Initialize root inode as a directory
     */

    if (!inode_init(&root_inode)) {
        printf("Root inode initialization failed\n");
        disk_close();
        return 1;
    }

    root_inode.type = INODE_TYPE_DIRECTORY;
    root_inode.link_count = 1U;
    root_inode.block_count = 1U;
    root_inode.blocks[0] = block_num;

    if (!inode_write(&root_inode, 0U)) {
        printf("Root inode write failed\n");
        disk_close();
        return 1;
    }

    /*
     * Directory add entry test
     */

    if (!directory_add_entry(0U, 1U, "test")) {
        printf("Directory add entry failed\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, block_num)) {
        printf("Directory block could not be read after adding entry\n");
        disk_close();
        return 1;
    }

    if (read_entries[0].inode_num != 1U ||
        strcmp(read_entries[0].name, "test") != 0) {
        printf("Directory entry was not added correctly\n");
        disk_close();
        return 1;
    }

    printf("Directory add entry test successful\n");

    /*
     * Duplicate name test
     */

    if (directory_add_entry(0U, 2U, "test")) {
        printf("Duplicate directory name was incorrectly accepted\n");
        disk_close();
        return 1;
    }

    printf("Duplicate directory name test successful\n");

    /*
     * Directory create test
     */

    if (!directory_create(0U, &inode_num, "child")) {
        printf("Directory creation failed\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(inode_num)) {
        printf("Created directory inode is not marked as occupied\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&created_inode, inode_num)) {
        printf("Created directory inode could not be read\n");
        disk_close();
        return 1;
    }

    if (!inode_is_directory(&created_inode)) {
        printf("Created inode is not a directory\n");
        disk_close();
        return 1;
    }

    if (created_inode.block_count != 1U) {
        printf("Created directory has incorrect block count\n");
        disk_close();
        return 1;
    }

    block_num = created_inode.blocks[0];

    if (!bitmap_test_block(block_num)) {
        printf("Created directory block is not marked as occupied\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, block_num)) {
        printf("Created directory block could not be read\n");
        disk_close();
        return 1;
    }

    /*
     * Check "."
     */

    if (read_entries[0].inode_num != inode_num ||
        strcmp(read_entries[0].name, ".") != 0) {
        printf("'.' entry is incorrect\n");
        disk_close();
        return 1;
    }

    /*
     * Check ".."
     */

    if (read_entries[1].inode_num != 0U ||
        strcmp(read_entries[1].name, "..") != 0) {
        printf("'..' entry is incorrect\n");
        disk_close();
        return 1;
    }

    printf("Directory '.' and '..' test successful\n");

    /*
     * Check parent entry
     */

    if (!directory_read(read_entries, root_inode.blocks[0])) {
        printf("Parent directory could not be read\n");
        disk_close();
        return 1;
    }

    int found_child = 0;

    for (uint32_t i = 0;
         i < BLOCK_SIZE / sizeof(directory_entry_t);
         i++) {

        if (read_entries[i].inode_num == inode_num &&
            strcmp(read_entries[i].name, "child") == 0) {

            found_child = 1;
            break;
        }
    }

    if (!found_child) {
        printf("Created directory was not added to parent\n");
        disk_close();
        return 1;
    }

    printf("Parent directory entry test successful\n");

    /*
     * Persistence test
     */

    if (!bitmap_write()) {
        printf("Bitmap could not be written\n");
        disk_close();
        return 1;
    }

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("Disk closed successfully\n");

    if (!disk_open("virtual_disk.img")) {
        printf("Disk could not be reopened\n");
        return 1;
    }

    printf("Disk reopened successfully\n");

    if (!superblock_read() || !superblock_validate()) {
        printf("Superblock validation failed\n");
        disk_close();
        return 1;
    }

    if (!bitmap_read()) {
        printf("Bitmap could not be read\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(inode_num)) {
        printf("Created directory inode was not persisted\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&created_inode, inode_num)) {
        printf("Created directory inode could not be read after reopening\n");
        disk_close();
        return 1;
    }

    if (!inode_is_directory(&created_inode)) {
        printf("Created directory type was not persisted\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, created_inode.blocks[0])) {
        printf("Created directory could not be read after reopening\n");
        disk_close();
        return 1;
    }

    if (read_entries[0].inode_num != inode_num ||
        strcmp(read_entries[0].name, ".") != 0) {
        printf("'.' was not persisted correctly\n");
        disk_close();
        return 1;
    }

    if (read_entries[1].inode_num != 0U ||
        strcmp(read_entries[1].name, "..") != 0) {
        printf("'..' was not persisted correctly\n");
        disk_close();
        return 1;
    }

    printf("Directory persistence test successful\n");

    if (!disk_close()) {
        printf("Disk could not be closed\n");
        return 1;
    }

    printf("All directory tests passed\n");

    return 0;
}
