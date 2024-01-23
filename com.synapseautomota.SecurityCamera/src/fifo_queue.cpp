#include "fifo_queue.h"

template <typename T>
FifoQueue<T>::FifoQueue() : front(nullptr), rear(nullptr) {}

template <typename T>
FifoQueue<T>::~FifoQueue() {
    clear();
}

template <typename T>
void FifoQueue<T>::enqueue(const IotMessage& message) {
    QueueNode* newNode = new QueueNode(message);

    if (isEmpty()) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }
}

template <typename T>
void FifoQueue<T>::dequeue() {
    if (isEmpty()) {
        std::cerr << "Error: Attempting to dequeue from an empty queue\n";
        return;
    }

    QueueNode* temp = front;
    front = front->next;

    if (front == nullptr) {
        rear = nullptr;
    }

    delete temp;
}

template <typename T>
IotMessage FifoQueue<T>::peek() const {
    if (isEmpty()) {
        std::cerr << "Error: Attempting to peek into an empty queue\n";
        exit(-1);
    }

    return front->data;
}

template <typename T>
bool FifoQueue<T>::isEmpty() const {
    return front == nullptr;
}

template <typename T>
void FifoQueue<T>::clear() {
    while (!isEmpty()) {
        dequeue();
    }
}

// Explicit instantiation for the types that will be used
template class FifoQueue<IotMessage>;
