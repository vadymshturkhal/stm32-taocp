#pragma once
#include <cstdint>

// forward declaration
class WaitList;

class WaitNode {
public:
    using NextInst = void (*)(WaitNode&, std::uint32_t now) noexcept;

    WaitNode() = default;
    constexpr WaitNode(NextInst next_inst, std::uint32_t delay) noexcept 
        : NEXTINST(next_inst), DELAY(delay) {}

    WaitNode(const WaitNode&) = delete;
    WaitNode& operator=(const WaitNode&) = delete;

private:
    friend class WaitList;

    WaitNode* left = this;
    WaitNode* right = this;
    std::uint32_t NEXTTIME = 0;
    NextInst NEXTINST = nullptr;
    std::uint32_t DELAY = 0;
};

class WaitList {
    
};
