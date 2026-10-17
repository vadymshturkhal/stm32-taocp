#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
#include "protocol.hpp"
#include "wait_list.hpp"

// Not need explicit numbers, the code only compares state == State:LISTEN
enum class State : std::uint8_t { LISTEN, SYN_RCVD, ESTABLISHED };

class Session {
public:
    State state() const noexcept {
        return state_;
    }

    void on_tick(std::uint32_t now) noexcept {
        if (!wait_list_.is_node_ready(now)) return;
        if (const auto fn = wait_list_.delete_first()) (this->*fn)(now);
    }

    void change_state(State next) noexcept {

    }

private:
    State state_ = State::LISTEN;
    WaitList<Session> wait_list_;
    WaitNode<Session> handshake_{&Session::handshake_expired, SESSION_TIMEOUT_MS};

    void handshake_expired(std::uint32_t /*now*/) noexcept {
        state_ = State::LISTEN;
    }

    // peer silent
    // heartbeat
    // close
};
