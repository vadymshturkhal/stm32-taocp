// cstyle.hpp - the C elevator (elevator/*.c) ported 1:1 to C++
//
// The same design as the C version, on purpose: one malloc'd block, a
// SharedState passed to every function, a WAIT list with Knuth's sentinel
// (head NEXTTIME = 0) and unsigned compares, the same functions in the same
// files. Only zero-cost C++ spellings differ (namespace, constexpr, nullptr,
// static_cast, bool). It answers one question: is C++ itself slower than C?
// Compared on the board with the C version and the Knuth-kernel version.
#pragma once
#include <cstdint>

namespace cstyle {

inline constexpr std::uint32_t FLOORS = 5;
inline constexpr std::uint32_t HOME_FLOOR = 2;

inline constexpr std::uint32_t CALLUP   = 0b100;
inline constexpr std::uint32_t CALLDOWN = 0b010;
inline constexpr std::uint32_t CALLCAR  = 0b001;

struct ElevatorNode;
struct SharedState;

using NextInst = void (*)(SharedState*, ElevatorNode*);

// storage_pool.h
struct StoragePool {
    void* avail;
    std::uint32_t node_size;
    std::uint32_t size;
};

StoragePool* create_storage_pool(void* memory, std::uint32_t node_size, std::uint32_t storage_size);
void* storage_pool_pop(StoragePool* storage_pool);
bool storage_pool_push(StoragePool* storage_pool, void* node);

// elevator_list.h
struct ElevatorNode {
    ElevatorNode* left1;    // WAIT list
    ElevatorNode* right1;
    ElevatorNode* left2;    // QUEUE, ELEVATOR_LIST
    ElevatorNode* right2;

    std::uint32_t NEXTTIME;
    NextInst NEXTINST;

    std::uint32_t IN;       // User
    std::uint32_t OUT;      // User
    std::uint32_t GIVEUPTIME;
    std::uint32_t id;       // For trace printing
};

struct ElevatorList {
    ElevatorNode* head;
    StoragePool* storage_pool;
};

std::uint32_t elevator_list_init(ElevatorList* elevator_list, StoragePool* storage_pool);
void elevator_list_insert_node_at_frontw(ElevatorList* elevator_list, ElevatorNode* node);
void elevator_list_insert_node_at_rear(ElevatorList* elevator_list, ElevatorNode* node);
void elevator_list_delete_nodew(ElevatorNode* node);
void elevator_list_delete_node(ElevatorNode* node);

// elevator_settings.h
struct Elevator;
struct Users;

struct SharedState {
    std::uint32_t TIME;
    std::uint32_t CALLS[FLOORS];
    ElevatorList ELEVATOR_LIST;   // Users currently riding the car
    ElevatorList QUEUE[FLOORS];   // Users waiting at each floor
    ElevatorList WAIT_LIST;
    Elevator* elevator;
    Users* users;
};

std::uint32_t shared_state_init(SharedState* shared_state, StoragePool* storage_pool);
void immed(SharedState* shared_state, ElevatorNode* wait_node);
void sortin(SharedState* shared_state, ElevatorNode* C);
void hold(SharedState* shared_state, ElevatorNode* node, std::uint32_t delay);
void holdc(SharedState* shared_state, ElevatorNode* node, std::uint32_t delay, NextInst next_inst);
void cycle1(ElevatorNode* node, NextInst next_inst);
void decision(SharedState* shared_state, NextInst caller);

// elevator.h
struct Elevator {
    SharedState* shared_state;
    std::int32_t STATE;
    std::uint32_t FLOOR;
    std::uint32_t D1;
    std::uint32_t D2;
    std::uint32_t D3;

    ElevatorNode* ELEV1;    // the elevator's actions, except for E5 and E9
    ElevatorNode* ELEV2;    // the independent action at E5
    ElevatorNode* ELEV3;    // the independent action at E9

    bool first_search;      // printing, E3-E4
};

std::uint32_t elevator_init(Elevator* elevator, SharedState* shared_state, StoragePool* storage_pool);
void E1(SharedState* shared_state, ElevatorNode* C);
void E3(SharedState* shared_state, ElevatorNode* C);
void E5A(SharedState* shared_state, std::uint32_t delay);
void E6(SharedState* shared_state, ElevatorNode* C);

// users.h
struct Users {
    SharedState* shared_state;
    StoragePool* storage_pool;  // used by U1 to allocate arriving users' nodes
    ElevatorNode* USER1;
    std::uint32_t user_id;
    std::uint32_t users_quantity;
};

std::uint32_t users_init(Users* users, SharedState* shared_state, StoragePool* storage_pool, std::uint32_t users_quantity);
void users_start(SharedState* shared_state);
void U5(SharedState* shared_state, ElevatorNode* C);
void U6(SharedState* shared_state, ElevatorNode* C);

// values.h
struct Values {
    std::uint32_t IN;
    std::uint32_t OUT;
    std::uint32_t GIVEUPTIME;
    std::uint32_t INTERTIME;
};

void values_seed(std::uint32_t seed);
Values values();

// trace.h
#ifdef TRACE
[[gnu::format(printf, 3, 4)]]
void trace(SharedState* shared_state, const char* step, const char* fmt, ...);
#else
inline void trace(SharedState*, const char*, const char*, ...) {}
#endif

}  // namespace cstyle

#include "cstyle_loop.h"
