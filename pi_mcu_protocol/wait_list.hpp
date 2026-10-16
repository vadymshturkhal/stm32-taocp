#pragma once
#include <cstdint>

template <class Context> class WaitList;

template <class Context>
class WaitNode {
public:
    using NextInst = void (Context::*)(std::uint32_t now) noexcept;

    WaitNode() = default;
    constexpr WaitNode(NextInst next_inst, std::uint32_t delay) noexcept 
        : NEXTINST(next_inst), DELAY(delay) {}

    WaitNode(const WaitNode&) = delete;
    WaitNode& operator=(const WaitNode&) = delete;

private:
    friend class WaitList<Context>;
    WaitNode* left = this;
    WaitNode* right = this;
    std::uint32_t NEXTTIME = 0;
    NextInst NEXTINST = nullptr;
    std::uint32_t DELAY = 0;
};

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
        
        unlink_node(node);
        sortin(node);
    }

    // Use node's DELAY
    void hold(Node& node, std::uint32_t now) noexcept {
        // Schedule node to run delay units from now
        node.NEXTTIME = now + node.DELAY;
        
        unlink_node(node);
        sortin(node);
    }

    void holdc(Node& node, std::uint32_t now, std::uint32_t delay, NextInst next_inst) noexcept {
        // Set NEXTINST and then invoke hold
        node.NEXTINST = next_inst;
        hold(node, now, delay);
    }

    // Use node's DELAY
    // NOTE: holdc(node, now, 0) will add nullptr to NEXTINST
    void holdc(Node& node, std::uint32_t now, NextInst next_inst) noexcept {
        // Set NEXTINST and then invoke hold
        node.NEXTINST = next_inst;
        hold(node, now);
    }

    // Insert node first in WAIT list
    void immed(Node& node, std::uint32_t now) noexcept {
        // Unlink node
        unlink_node(node);

        // Set NEXTTIME
        node.NEXTTIME = now;

        // Insert
        node.left = &head;
        node.right = head.right;
        head.right->left = &node;
        head.right = &node;
    }

    [[nodiscard]] bool is_node_ready(std::uint32_t now) const noexcept {
        return !is_empty() && !is_less(now, head.right->NEXTTIME);
    }

    [[nodiscard]] NextInst delete_first() noexcept {
        if (is_empty()) return nullptr;

        Node& n = *head.right;
        delete_node(n);

        return n.NEXTINST;
    }

    void delete_node(Node& node) noexcept {
        // Unlink node
        unlink_node(node);

        // Point to itself
        node.left = &node;
        node.right = &node;
    }

    [[nodiscard]] bool is_empty() const noexcept {
        return head.right == &head;
    }
        
private:
    WaitNode<Context> head;

    void unlink_node(Node& node) noexcept {
        node.left->right = node.right;
        node.right->left = node.left;
    }

    // Sort node into WAIT list using left and right
    void sortin(Node& C) noexcept {
        Node* P = head.left;  // Last node

        // Compare NEXTTIME fields right to left
        while (P != &head && is_less(C.NEXTTIME, P->NEXTTIME)) {
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
