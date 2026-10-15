#pragma once
#include <cstdint>

template <class Context> class WaitList;

template <class Context>
class WaitNode {
public:
    WaitNode() = default;
    WaitNode(const WaitNode&) = delete;
    WaitNode& operator=(const WaitNode&) = delete;

    using NextInst = void (Context::*)(std::uint32_t now) noexcept;

private:
    friend class WaitList<Context>;
    WaitNode* left = this;
    WaitNode* right = this;
    std::uint32_t NEXTTIME = 0;
    NextInst NEXTINST = nullptr;
};

// WaitList
// peek
// is_node_ready
// delete_and_run

template <class Context>
class WaitList {
public:
    using Node = WaitNode<Context>;
    using NextInst = typename Node::NextInst;

    static constexpr bool is_less(std::uint32_t a, std::uint32_t b) noexcept {
        return static_cast<std::int32_t>(a - b) < 0;
    }

    void hold(Node& node, std::uint32_t now, std::uint32_t delay) noexcept {
        // Schedule node to run delay units from now
        node.NEXTTIME = now + delay;
        
        // Unlink node
        node.left->right = node.right;
        node.right->left = node.left;

        sortin(node);
    }

    void holdc(Node& node, std::uint32_t now, std::uint32_t delay, NextInst next_inst) noexcept {
        // Set NEXTINST and then invoke hold
        node.NEXTINST = next_inst;
        hold(node, now, delay);
    }

    void delete_node(Node& node) noexcept {
        // Unlink node
        node.left->right = node.right;
        node.right->left = node.left;

        // Point to itself
        node.left = &node;
        node.right = &node;
    }

    // Insert node first in WAIT list
    void immed(Node& node, std::uint32_t now) noexcept {
        // Unlink node
        node.left->right = node.right;
        node.right->left = node.left;

        // Set NEXTTIME
        node.NEXTTIME = now;

        // Insert
        node.left = &head;
        node.right = head.right;
        head.right->left = &node;
        head.right = &node;
    }
        
private:
    WaitNode<Context> head;

    // Sort node into WAIT list using left and right
    void sortin(Node& C) noexcept {
        Node* P = head.left;  // Last node

        // Compare NEXTTIME fields right to left
        while (is_less(C.NEXTTIME, P->NEXTTIME) && P != &head) {
            P = P->left;
        }

        // Insert node to WAIT list
        Node* Q = P->right;
        C.right = Q;
        C.left = P;
        P->right = &C;
        Q->left = &C;
    }
};
