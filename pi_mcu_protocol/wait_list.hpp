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
// immed
// hold
// holdc
// peek
// delete
// sortin
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


private:
    WaitNode<Context> head;
    
    // Sort node into WAIT list using left and right
    void sortin(Node& C) noexcept {
        Node* P = head.left;  // Last node

        // Compare NEXTTIME fields right to left
        while (is_less(C.NEXTTIME, P->NEXTTIME) && P != &head) {
            P = P->left;
        }

        // Insert to WAIT list
        Node* Q = P->right;
        C.right = Q;
        C.left = P;
        P->right = &C;
        Q->left = &C;
    }
};
