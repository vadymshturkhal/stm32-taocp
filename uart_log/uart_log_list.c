#include <stddef.h>

#include "storage_pool.h"
#include "uart_log_list.h"

uint32_t uart_log_list_init(UartLogList* list, Storage_Pool* storage_pool) {
    if (list == NULL || storage_pool == NULL) return 1;

    UartLogNode* head = storage_pool_pop(storage_pool);
    if (head == NULL) return 2;

    list->head = head;
    list->head->left = list->head;
    list->head->right = list->head;

    list->storage_pool = storage_pool;
    return 0;
}

void uart_log_list_insert_node_at_rear(UartLogList* list, UartLogNode* node) {
    UartLogNode* Q = list->head->left;
    node->left = Q;
    list->head->left = node;
    Q->right = node;
    node->right = list->head;
}

void uart_log_list_delete_node(UartLogNode* node) {
    node->left->right = node->right;
    node->right->left = node->left;
}
