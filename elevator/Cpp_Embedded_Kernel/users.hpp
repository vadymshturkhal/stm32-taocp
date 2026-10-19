#pragma once

class ElevatorSimulation;

class Users {
public:
    explicit Users(ElevatorSimulation& s) noexcept : state(s) {}

private:
    ElevatorSimulation& state;
};
