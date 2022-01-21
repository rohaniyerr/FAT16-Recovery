#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "directory_tree.h"
#include "fat16.h"

const size_t MASTER_BOOT_RECORD_SIZE = 0x20B;


void follow(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb) {
    directory_entry_t disk_entry;
    while (true) {
        assert(fread(&disk_entry, sizeof(directory_entry_t), 1, disk) == 1);
        if (disk_entry.filename[0] == '\0') {
            break;
        }
        long reset = ftell(disk);
        fseek(disk, get_offset_from_cluster(disk_entry.first_cluster, bpb), SEEK_SET);
        if (is_hidden(disk_entry)) {
            fseek(disk, reset, SEEK_SET);
            continue;
        }
        if (is_directory(disk_entry)) {
            directory_node_t *next_node =
                init_directory_node(get_file_name(disk_entry));
            add_child_directory_tree(node, (node_t *) next_node);
            follow(disk, next_node, bpb);
        }
        else {
            uint8_t *file_bytes = malloc(disk_entry.file_size);
            assert(fread(file_bytes, disk_entry.file_size, 1, disk) == 1);
            add_child_directory_tree(
                node, (node_t *) init_file_node(get_file_name(disk_entry),
                                                disk_entry.file_size, file_bytes));
        }
        fseek(disk, reset, SEEK_SET);
        }
    }

// void follow_helper(FILE *disk, directory_node_t *node, directory_entry_t disk_entry, bios_parameter_block_t bpb){
    
// }





int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <image filename>\n", argv[0]);
        return 1;
    }

    FILE *disk = fopen(argv[1], "r");
    if (disk == NULL) {
        fprintf(stderr, "No such image file: %s\n", argv[1]);
        return 1;
    }

    bios_parameter_block_t bpb;

    /* TODO: Write your code here. */
    fseek(disk, MASTER_BOOT_RECORD_SIZE, SEEK_SET);
    assert(fread(&bpb, sizeof(bpb), 1, disk) == 1);
    fseek(disk, get_root_directory_location(bpb), SEEK_SET);

    directory_node_t *root = init_directory_node(NULL);
    follow(disk, root, bpb);
    print_directory_tree((node_t *) root);
    create_directory_tree((node_t *) root);
    free_directory_tree((node_t *) root);

    assert(fclose(disk) == 0);
}
