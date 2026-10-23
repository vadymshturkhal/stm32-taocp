// cstyle_loop.cpp - c_elevator_loop.c ported 1:1: the run split into start / one step / stop
#include <cstdint>
#include <cstdlib>
#include "cstyle.hpp"

using namespace cstyle;

static void* master_memory = nullptr;
static SharedState* shared_state = nullptr;

extern "C" std::uint32_t cstyle_elevator_start(std::uint32_t max_users) {
    values_seed(1);  // fixed seed: the same users as every other run

    std::uint32_t num_lists = FLOORS + 2;                   // ELEVATOR_LIST, QUEUE[0..FLOORS-1], WAIT_LIST
    std::uint32_t max_nodes = num_lists + 4 + max_users;    // list heads, ELEV1-3, USER1, and the users

    std::uint32_t storage_pool_size = static_cast<std::uint32_t>(sizeof(StoragePool) + max_nodes * sizeof(ElevatorNode));
    std::uint32_t shared_state_size = static_cast<std::uint32_t>(sizeof(SharedState));
    std::uint32_t elevator_size = static_cast<std::uint32_t>(sizeof(Elevator));
    std::uint32_t users_size = static_cast<std::uint32_t>(sizeof(Users));
    std::uint32_t master_memory_size = storage_pool_size + shared_state_size + elevator_size + users_size;

    master_memory = std::malloc(master_memory_size);
    if (master_memory == nullptr) return 1;

    StoragePool* storage_pool = create_storage_pool(master_memory, static_cast<std::uint32_t>(sizeof(ElevatorNode)), max_nodes);
    if (storage_pool == nullptr) {
        cstyle_elevator_stop();
        return 2;
    }

    auto* shared_state_memory = static_cast<std::uint8_t*>(master_memory) + storage_pool_size;
    auto* elevator_memory = shared_state_memory + shared_state_size;
    auto* users_memory = elevator_memory + elevator_size;

    shared_state = reinterpret_cast<SharedState*>(shared_state_memory);
    auto* elevator = reinterpret_cast<Elevator*>(elevator_memory);
    auto* users = reinterpret_cast<Users*>(users_memory);

    std::uint32_t status = shared_state_init(shared_state, storage_pool);
    if (status == 0) status = elevator_init(elevator, shared_state, storage_pool);
    if (status == 0) status = users_init(users, shared_state, storage_pool, max_users);
    if (status != 0) {
        cstyle_elevator_stop();
        return status;
    }

    users_start(shared_state);
    return 0;
}

extern "C" int cstyle_elevator_step(void) {
    if (shared_state == nullptr) return 0;

    // One pass of cycle(): is the WAIT list empty?
    ElevatorNode* head = shared_state->WAIT_LIST.head;
    ElevatorNode* C = head->right1;
    if (C == head) return 0;

    // Advance TIME to its NEXTTIME, unlink it, call NEXTINST
    shared_state->TIME = C->NEXTTIME;
    elevator_list_delete_nodew(C);
    C->NEXTINST(shared_state, C);
    return 1;
}

extern "C" std::uint32_t cstyle_elevator_time(void) {
    return (shared_state != nullptr) ? shared_state->TIME : 0;
}

extern "C" void cstyle_elevator_stop(void) {
    std::free(master_memory);
    master_memory = nullptr;
    shared_state = nullptr;
}
