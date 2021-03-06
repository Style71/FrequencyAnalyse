/*
 * DataStructure.h
 *
 *  Created on: Oct 28, 2020
 *      Author: QiYang
 */

#ifndef DATASTRUCTURE_H_
#define DATASTRUCTURE_H_

#include <stdbool.h>
#include "main.h"

#define USART_TXRX_BUFFER_SIZE 1024
#define PACKET_BUFFER_SIZE 32  // This macro should be slightly greater than MAX_PACKET_PAYLOAD_SIZE
#define MESSAGE_BUFFER_SIZE 80 // This macro should be slightly greater than MAX_MSG_PAYLOAD_SIZE
#define FREQWAVE_BUFFER_SIZE 16
template <class T, unsigned int Queue_Size>
class Queue
{
private:
    int tail;
    int head;
    int length;

public:
    Queue(/* args */);
    ~Queue();
    int size() { return length; }
    int capacity() { return Queue_Size; }
    bool push_back(T element);
    void brute_push_back(T element);
    T pop_front();
    void pop_front(int num);
    T pop_back();
    void pop_back(int num);
    void clear();
    T &operator[](unsigned int index);
    bool replace(unsigned int index, T element);
    bool isEmpty() { return (length == 0); }
    bool isFull() { return (length == Queue_Size); }
    int get_head() { return head; }
    int get_tail() { return tail; }

    // Do not use these function calls unless you konw what you are doing.
    bool set_tail(int num);
    bool set_length(int num);

    T queue[Queue_Size];
};

#endif /* DATASTRUCTURE_H_ */
