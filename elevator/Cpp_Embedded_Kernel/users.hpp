#pragma once

class ElevatorSimulation;

class Users {
public:
    explicit Users(ElevatorSimulation& s) noexcept : simulation(s) {}

private:
    ElevatorSimulation& simulation;
};
