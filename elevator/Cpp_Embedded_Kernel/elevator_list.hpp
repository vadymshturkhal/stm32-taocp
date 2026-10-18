#pragma once
#include "elevator_node.hpp"

class ElevatorList {
public:
    static void delete_node(ElevatorNode& node) noexcept;
    void insert_at_rear(ElevatorNode& node) noexcept;
    [[nodiscard]] bool is_empty() const noexcept;

    ElevatorNode head;
};
