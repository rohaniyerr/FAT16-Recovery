#include "directory_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int PERMISSION = 0777;

void init_node(node_t *node, char *name, node_type_t type) {
    if (name == NULL) {
        name = strdup("ROOT");
        assert(name != NULL);
    }
    node->name = name;
    node->type = type;
}

file_node_t *init_file_node(char *name, size_t size, uint8_t *contents) {
    file_node_t *node = malloc(sizeof(file_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, FILE_TYPE);
    node->size = size;
    node->contents = contents;
    return node;
}

directory_node_t *init_directory_node(char *name) {
    directory_node_t *node = malloc(sizeof(directory_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, DIRECTORY_TYPE);
    node->num_children = 0;
    node->children = NULL;
    return node;
}

void add_child_directory_tree(directory_node_t *dnode, node_t *child) {
    dnode->children =
        realloc(dnode->children, sizeof(node_t *) * (dnode->num_children + 1));
    if (dnode->num_children == 0) {
        dnode->children[0] = child;
    }
    else {
        for (int i = dnode->num_children - 1; i >= 0; i--) {
            dnode->children[i + 1] = child;
            if (strcmp(dnode->children[i]->name, child->name) <= 0) {
                break;
            }
            else {
                dnode->children[i + 1] = dnode->children[i];
                dnode->children[i] = child;
            }
        }
    }
    dnode->num_children++;
}

void print_directory_tree(node_t *node) {
    print_helper(node, 0);
}
void print_helper(node_t *node, size_t spaces) {
    for (size_t i = 0; i < spaces; i++) {
        printf(" ");
    }
    printf("%s\n", node->name);
    if (node->type == DIRECTORY_TYPE) {
        directory_node_t *curr = (directory_node_t *) node;
        for (size_t i = 0; i < curr->num_children; i++) {
            print_helper(curr->children[i], spaces + 4);
        }
    }
}

void helper_directory_tree(node_t *node, char *filename) {
    char *dir_name = malloc(sizeof(char) * (strlen(filename) + strlen(node->name) + 2));
    strcpy(dir_name, filename);
    strcat(dir_name, "/");
    strcat(dir_name, node->name);
    if (node->type == DIRECTORY_TYPE) {
        directory_node_t *dir_node = (directory_node_t *) node;
        assert(mkdir(dir_name, PERMISSION) == 0);
        for (size_t i = 0; i < dir_node->num_children; i++) {
            helper_directory_tree(dir_node->children[i], dir_name);
        }
    }
    else {
        file_node_t *file_node = (file_node_t *) node;
        FILE *file = fopen(dir_name, "w");
        assert(fwrite(file_node->contents, sizeof(char), file_node->size, file) ==
               file_node->size);
        assert(fclose(file) == 0);
    }
    free(dir_name);
}

void create_directory_tree(node_t *node) {
    if (node != NULL) {
        helper_directory_tree(node, ".");
    }
}

void free_directory_tree(node_t *node) {
    if (node->type == FILE_TYPE) {
        file_node_t *fnode = (file_node_t *) node;
        free(fnode->contents);
    }
    else {
        assert(node->type == DIRECTORY_TYPE);
        directory_node_t *dnode = (directory_node_t *) node;
        for (size_t i = 0; i < dnode->num_children; i++) {
            free_directory_tree(dnode->children[i]);
        }
        free(dnode->children);
    }
    free(node->name);
    free(node);
}
