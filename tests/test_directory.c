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
    inode_t file_inode;
    inode_t created_inode;

    uint32_t scratch_block;
    uint32_t root_block;
    uint32_t file_inode_num;
    uint32_t inode_num;
    uint32_t found_inode_num;
    uint32_t child_block;

    /*
     * Disk initialization
     */

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
     *
     * Use a separate allocated block so that
     * the test data does not interfere with root.
     */

    if (!bitmap_find_free_block(&scratch_block) ||
        !bitmap_set_block(scratch_block) ||
        !bitmap_write()) {

        printf("Scratch block allocation failed\n");
        disk_close();
        return 1;
    }

    for (uint32_t i = 0;
         i < BLOCK_SIZE / sizeof(directory_entry_t);
         i++) {

        if (!directory_entry_init(&entries[i])) {
            printf("Directory entry initialization failed\n");
            disk_close();
            return 1;
        }

        entries[i].inode_num = i + 1U;

        snprintf(
            entries[i].name,
            sizeof(entries[i].name),
            "entry%u",
            i
        );
    }

    if (!directory_write(entries, scratch_block)) {
        printf("Directory block write failed\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, scratch_block)) {
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
     * Initialize root directory
     */

    if (!bitmap_find_free_block(&root_block) ||
        !bitmap_set_block(root_block) ||
        !bitmap_write()) {

        printf("Root block allocation failed\n");
        disk_close();
        return 1;
    }

    for (uint32_t i = 0;
         i < BLOCK_SIZE / sizeof(directory_entry_t);
         i++) {

        if (!directory_entry_init(&entries[i])) {
            printf("Root directory entry initialization failed\n");
            disk_close();
            return 1;
        }
    }

    /*
     * Root "." points to root inode 0.
     * Root ".." also points to root inode 0.
     */

    entries[0].inode_num = 0U;
    strcpy(entries[0].name, ".");

    entries[1].inode_num = 0U;
    strcpy(entries[1].name, "..");

    if (!directory_write(entries, root_block)) {
        printf("Root directory block write failed\n");
        disk_close();
        return 1;
    }

    if (!inode_init(&root_inode)) {
        printf("Root inode initialization failed\n");
        disk_close();
        return 1;
    }

    root_inode.type = INODE_TYPE_DIRECTORY;
    root_inode.link_count = 1U;
    root_inode.block_count = 1U;
    root_inode.blocks[0] = root_block;

    if (!inode_write(&root_inode, 0U)) {
        printf("Root inode write failed\n");
        disk_close();
        return 1;
    }

    printf("Root directory initialized successfully\n");

    /*
     * Allocate a real file inode for add-entry test
     */

    if (!inode_allocate(&file_inode_num)) {
        printf("File inode allocation failed\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&file_inode, file_inode_num)) {
        printf("File inode read failed\n");
        disk_close();
        return 1;
    }

    file_inode.type = INODE_TYPE_FILE;
    file_inode.link_count = 1U;

    if (!inode_write(&file_inode, file_inode_num)) {
        printf("File inode write failed\n");
        disk_close();
        return 1;
    }

    /*
     * Directory add entry test
     */

    if (!directory_add_entry(0U, file_inode_num, "test")) {
        printf("Directory add entry failed\n");
        disk_close();
        return 1;
    }

    if (!directory_lookup(0U, "test", &found_inode_num)) {
        printf("Added directory entry could not be found\n");
        disk_close();
        return 1;
    }

    if (found_inode_num != file_inode_num) {
        printf("Added directory entry has incorrect inode\n");
        disk_close();
        return 1;
    }

    printf("Directory add entry test successful\n");

    /*
     * Verify root "." and ".." were not overwritten
     */

    if (!directory_read(read_entries, root_block)) {
        printf("Root directory read failed\n");
        disk_close();
        return 1;
    }

    if (strcmp(read_entries[0].name, ".") != 0 ||
        read_entries[0].inode_num != 0U ||
        strcmp(read_entries[1].name, "..") != 0 ||
        read_entries[1].inode_num != 0U) {

        printf("Root '.' or '..' was overwritten\n");
        disk_close();
        return 1;
    }

    printf("Root special entries test successful\n");

    /*
     * Duplicate name test
     */

    if (directory_add_entry(0U, file_inode_num, "test")) {
        printf("Duplicate directory name was accepted\n");
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
        printf("Created inode is not allocated\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&created_inode, inode_num)) {
        printf("Created directory inode read failed\n");
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

    child_block = created_inode.blocks[0];

    if (!bitmap_test_block(child_block)) {
        printf("Created directory block is not allocated\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, child_block)) {
        printf("Created directory block read failed\n");
        disk_close();
        return 1;
    }

    if (read_entries[0].inode_num != inode_num ||
        strcmp(read_entries[0].name, ".") != 0) {

        printf("Child '.' entry is incorrect\n");
        disk_close();
        return 1;
    }

    if (read_entries[1].inode_num != 0U ||
        strcmp(read_entries[1].name, "..") != 0) {

        printf("Child '..' entry is incorrect\n");
        disk_close();
        return 1;
    }

    printf("Directory creation test successful\n");
    printf("Directory '.' and '..' test successful\n");

    /*
     * Verify child was added to parent
     */

    if (!directory_lookup(0U, "child", &found_inode_num)) {
        printf("Child not found in parent directory\n");
        disk_close();
        return 1;
    }

    if (found_inode_num != inode_num) {
        printf("Child entry has incorrect inode number\n");
        disk_close();
        return 1;
    }

    printf("Parent directory entry test successful\n");

    /*
     * Directory lookup tests
     */

    if (!directory_lookup(0U, "test", &found_inode_num) ||
        found_inode_num != file_inode_num) {

        printf("File lookup test failed\n");
        disk_close();
        return 1;
    }

    printf("File lookup test successful\n");

    if (!directory_lookup(0U, "child", &found_inode_num) ||
        found_inode_num != inode_num) {

        printf("Child lookup test failed\n");
        disk_close();
        return 1;
    }

    printf("Child lookup test successful\n");

    /*
     * Lookup "." inside child
     */

    if (!directory_lookup(inode_num, ".", &found_inode_num) ||
        found_inode_num != inode_num) {

        printf("'.' lookup test failed\n");
        disk_close();
        return 1;
    }

    printf("Directory '.' lookup test successful\n");

    /*
     * Lookup ".." inside child
     */

    if (!directory_lookup(inode_num, "..", &found_inode_num) ||
        found_inode_num != 0U) {

        printf("'..' lookup test failed\n");
        disk_close();
        return 1;
    }

    printf("Directory '..' lookup test successful\n");

    /*
     * Root "." and ".." lookup
     */

    if (!directory_lookup(0U, ".", &found_inode_num) ||
        found_inode_num != 0U) {

        printf("Root '.' lookup test failed\n");
        disk_close();
        return 1;
    }

    if (!directory_lookup(0U, "..", &found_inode_num) ||
        found_inode_num != 0U) {

        printf("Root '..' lookup test failed\n");
        disk_close();
        return 1;
    }

    printf("Root lookup tests successful\n");

    /*
     * Nonexistent entry lookup
     */

    if (directory_lookup(0U, "nonexistent", &found_inode_num)) {
        printf("Nonexistent entry was incorrectly found\n");
        disk_close();
        return 1;
    }

    printf("Nonexistent lookup test successful\n");

    /*
     * Empty name lookup
     */

    if (directory_lookup(0U, "", &found_inode_num)) {
        printf("Empty name was incorrectly accepted\n");
        disk_close();
        return 1;
    }

    printf("Empty name lookup test successful\n");

    /*
     * Name longer than MAX_FILENAME
     */

    char long_name[MAX_FILENAME + 2];

    memset(long_name, 'a', sizeof(long_name));
    long_name[sizeof(long_name) - 1] = '\0';

    if (directory_lookup(0U, long_name, &found_inode_num)) {
        printf("Oversized name was incorrectly accepted\n");
        disk_close();
        return 1;
    }

    printf("Oversized name lookup test successful\n");

    /*
     * Persistence test
     *
     * Do not call bitmap_write() here.
     * Directory operations must persist their
     * own bitmap allocations.
     */

    if (!disk_close()) {
        printf("Disk close failed\n");
        return 1;
    }

    printf("Disk closed successfully\n");

    if (!disk_open("virtual_disk.img")) {
        printf("Disk reopen failed\n");
        return 1;
    }

    printf("Disk reopened successfully\n");

    if (!superblock_read() || !superblock_validate()) {
        printf("Superblock validation failed\n");
        disk_close();
        return 1;
    }

    if (!bitmap_read()) {
        printf("Bitmap read failed after reopening\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_inode(inode_num)) {
        printf("Created inode was not persisted\n");
        disk_close();
        return 1;
    }

    if (!bitmap_test_block(child_block)) {
        printf("Created directory block allocation was not persisted\n");
        disk_close();
        return 1;
    }

    if (!inode_read(&created_inode, inode_num)) {
        printf("Created inode read failed after reopening\n");
        disk_close();
        return 1;
    }

    if (!inode_is_directory(&created_inode)) {
        printf("Created inode type was not persisted\n");
        disk_close();
        return 1;
    }

    if (!directory_read(read_entries, created_inode.blocks[0])) {
        printf("Created directory read failed after reopening\n");
        disk_close();
        return 1;
    }

    if (strcmp(read_entries[0].name, ".") != 0 ||
        read_entries[0].inode_num != inode_num ||
        strcmp(read_entries[1].name, "..") != 0 ||
        read_entries[1].inode_num != 0U) {

        printf("Directory special entries were not persisted\n");
        disk_close();
        return 1;
    }

    printf("Directory persistence test successful\n");

    /*
     * Lookup persistence tests
     */

    if (!directory_lookup(0U, "child", &found_inode_num) ||
        found_inode_num != inode_num) {

        printf("Child lookup failed after reopening\n");
        disk_close();
        return 1;
    }

    if (!directory_lookup(0U, "test", &found_inode_num) ||
        found_inode_num != file_inode_num) {

        printf("File lookup failed after reopening\n");
        disk_close();
        return 1;
    }

    if (!directory_lookup(inode_num, "..", &found_inode_num) ||
        found_inode_num != 0U) {

        printf("Parent lookup failed after reopening\n");
        disk_close();
        return 1;
    }

    printf("Directory lookup persistence test successful\n");

    if (!disk_close()) {
        printf("Final disk close failed\n");
        return 1;
    }

    printf("\nAll directory tests passed successfully!\n");

    return 0;
}
