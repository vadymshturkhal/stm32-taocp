#include "elevator_list.hpp"

void ElevatorList::delete_node(ElevatorNode& node) noexcept {
    node.left2->right2 = node.right2;
    node.right2->left2 = node.left2;

    // Point links to itself
    node.left2 = node.right2 = &node;
}

void ElevatorList::insert_at_rear(ElevatorNode& node) noexcept {
    ElevatorNode* Q = head.left2;
	node.left2 = Q;
	head.left2 = &node;
	Q->right2 = &node;
    node.right2 = &head;
}

bool ElevatorList::is_empty() const noexcept {
    return head.right2 == &head;
}
