#pragma once

class ElevatorState;

class Elevator {
public:
    explicit Elevator (ElevatorState& s) noexcept : state(s) {}

private:
    ElevatorState& state;
};
