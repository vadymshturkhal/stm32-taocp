// c_queue.h
#ifndef C_QUEUE_H	// prevent duplicate definitions
#define C_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Node {
    uint32_t info;
    struct Node* link;
} Node;

typedef struct {
	Node* front;
    Node** rear;
    Node* avail;
} Queue;

// Prototypes
Node* init_queue_storage_pool(Queue* queue, uint32_t size);
Queue* c_create_queue(void* memory, uint32_t size);

static inline bool c_queue_push(Queue* queue, uint32_t info) {
	// return false if Overflow, else true

	// 1 (P <= Avail)
	if (queue->avail == NULL) return false;	// Overflow
	Node* P = queue->avail;
	queue->avail = queue->avail->link;

	// 2
	P->info = info;

	// 3
	P->link = NULL;

	// 4 (queue->rear->link = P;)
	// if empty: writes P into queue->front, else: into the old rear's link pointer
	*queue->rear = P;

	// 5 (queue->rear = P;)
	queue->rear = &P->link;

	return true;
}

static inline uint32_t c_queue_pop(Queue* queue, bool* pop_is_success) {
	// return false if Underflow, else true
	// input pop_is_success flag must always be true

	if (queue->front == NULL) {
		*pop_is_success = false;	// Underflow
		return 0;
	}

	// 1
	Node* P = queue->front;

	// 2
	queue->front = P->link;

	// 3
	uint32_t info = P->info;

	// 4 (Avail <= P)
	P->link = queue->avail;
	queue->avail = P;

	// 5
	if (queue->front == NULL) {
		// point Rear to the memory address of Front
		queue->rear = &queue->front;
	}

	return info;
}


#endif // C_QUEUE_H
