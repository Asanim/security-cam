#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include <iostream>
#include <string>

struct IotMessage {
    std::string& publishTopic;
    std::string& payload;
};

template <typename T>
class FifoQueue {
private:
    struct QueueNode {
        T data;
        QueueNode* next;

        QueueNode(const T& value) : data(value), next(nullptr) {}
    };

    QueueNode* front;
    QueueNode* rear;

public:
    FifoQueue();
    ~FifoQueue();

    void enqueue(const IotMessage& message);
    void dequeue();
    IotMessage peek() const;
    bool isEmpty() const;
    void clear();
};

#endif // FIFO_QUEUE_H
