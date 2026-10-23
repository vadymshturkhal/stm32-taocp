// elevator_list.cpp - elevator_list.c ported 1:1
#include <cstdint>
#include "cstyle.hpp"

namespace cstyle {

std::uint32_t elevator_list_init(ElevatorList* elevator_list, StoragePool* storage_pool) {
    if (elevator_list == nullptr || storage_pool == nullptr) return 1;

    auto* head = static_cast<ElevatorNode*>(storage_pool_pop(storage_pool));
    if (head == nullptr) return 2;

    elevator_list->head = head;
    head->left1 = head;
    head->right1 = head;
    head->left2 = head;
    head->right2 = head;

    elevator_list->storage_pool = storage_pool;
    return 0;
}

void elevator_list_insert_node_at_frontw(ElevatorList* elevator_list, ElevatorNode* node) {
    // Insert node into WAIT list using LLINK1 and RLINK1
    ElevatorNode* X = elevator_list->head;
    node->left1 = X;
    node->right1 = X->right1;
    X->right1->left1 = node;
    X->right1 = node;
}

void elevator_list_insert_node_at_rear(ElevatorList* elevator_list, ElevatorNode* node) {
    ElevatorNode* Q = elevator_list->head->left2;
    node->left2 = Q;
    elevator_list->head->left2 = node;
    Q->right2 = node;
    node->right2 = elevator_list->head;
}

void elevator_list_delete_nodew(ElevatorNode* node) {
    // Delete node from WAIT list
    node->left1->right1 = node->right1;
    node->right1->left1 = node->left1;
}

void elevator_list_delete_node(ElevatorNode* node) {
    node->left2->right2 = node->right2;
    node->right2->left2 = node->left2;
}

}  // namespace cstyle
