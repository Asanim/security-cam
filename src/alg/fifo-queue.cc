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


#include "fifo_queue.cc"

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
