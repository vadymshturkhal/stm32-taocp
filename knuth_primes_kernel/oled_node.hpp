#pragma once

template <OledView View>
class OledNode : public WaitNode {
public:
    OledNode(Kernel& k, View& v) noexcept : WaitNode(start_frame, 0), kernel(k), view(v) {}

    void transfer_finished(bool ok) noexcept;  // ISR

    [[nodiscard]] std::uint32_t frames_shown() const noexcept;
    [[nodiscard]] std::uint32_t frames_failed() const noexcept;

    // ?
    [[nodiscard]] std::uint32_t timeouts() const noexcept;


private:
    // ?
    static void start_frame(WaitNode&, std::uint32_t now) noexcept;  // Timer, every L

    // Events and deadline
    static void fill_step(WaitNode& node, std::uint32_t now) noexcept {
        static_cast<OledNode>(node).fill(now);
    }
    
    // ?
    void fill(std::uint32_t now) noexcept;  // The fill loop

    void start_transfer() noexcept;  // Start DMA if the bus is idle

    // ?
    void  end_frame(std::uint32_t now, bool ok) noexcept;

    Kernel& kernel;
    View& view;
};
