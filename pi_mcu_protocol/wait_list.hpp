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

template <class Context>
class WaitList {
public:

private:
    WaitNode<Context> head;
};
