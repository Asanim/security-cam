/* 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  \file fifo_queue.h
 */

 
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
