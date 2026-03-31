#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_queue.h"

bool c_enqueue_tub(Queue* queue, uint32_t info) {
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

uint32_t c_dequeue_tub(Queue* queue, bool* pop_is_success) {
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

Node* init_queue_storage_pool(Queue* queue, uint32_t size) {
	Node* avail = (Node*)(queue + 1);
	Node* tmp;

	avail->info = size;
	avail->link = NULL;
	size--;

	while (size > 0) {
		tmp = avail+1;
		tmp->info = size;
		tmp->link = avail;

		avail = tmp;
		size--;
	}

	return avail;
}

Queue* c_create_queue(void* memory, uint32_t size) {
	// size must be greater than 0

	Queue* queue = (Queue*)memory;
	queue->front = NULL;

	// point Rear to the memory address of Front
	queue->rear = &queue->front;

	Node* avail = init_queue_storage_pool(queue, size);
	queue->avail = avail;

	return queue;
}
