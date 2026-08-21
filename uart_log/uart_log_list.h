#ifndef UART_LOG_LIST_H
#define UART_LOG_LIST_H

#include <stdint.h>

#include "storage_pool.h"

// Pending-message queue node -- FIFO, not time-ordered like the elevator's
// WAIT_LIST (log lines have no NEXTTIME, they just drain in arrival order).
// Only ever belongs to one list at a time, unlike ElevatorNode, so a single
// left/right pair is enough (see elevator_list.h for the dual-pair version).
struct UartLogNode;
typedef void (*UartLogNextInst)(struct UartLogNode*);

typedef struct UartLogNode {
    struct UartLogNode* left;
    struct UartLogNode* right;

    const uint8_t* buf;
    uint32_t len;
    // UartLogNextInst NEXTINST;   // what CYCLE-equivalent dispatch calls once this node's transfer completes
} UartLogNode;

typedef struct {
    UartLogNode* head;
    Storage_Pool* storage_pool;
    uint32_t size;
} UartLogList;

uint32_t uart_log_list_init(UartLogList* list, Storage_Pool* storage_pool);
void uart_log_list_insert_node_at_rear(UartLogList* list, UartLogNode* node);
void uart_log_list_delete_node(UartLogNode* node);

#endif
