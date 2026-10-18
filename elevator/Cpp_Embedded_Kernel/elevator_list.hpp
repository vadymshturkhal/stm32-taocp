#pragma once
#include "elevator_node.hpp"

class ElevatorList {
public:
    static void remove(ElevatorNode& node) noexcept;
    void insert_at_rear(ElevatorNode& node) noexcept;
    [[nodiscard]] bool is_empty() const noexcept;

    ElevatorNode head;
};
