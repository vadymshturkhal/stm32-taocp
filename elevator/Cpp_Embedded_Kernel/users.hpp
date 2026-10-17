#pragma once

class ElevatorState;

class Users {
public:
    explicit Users(ElevatorState& s) noexcept : state(s) {}

private:
    ElevatorState& state;
};
