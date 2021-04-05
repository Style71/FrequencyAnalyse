/*
 * DataStructure.c
 *
 *  Created on: Oct 28, 2020
 *      Author: QiYang
 */
#include "DataStructure.h"
#include "DSP.h"

template <class T, unsigned int Queue_Size>
Queue<T, Queue_Size>::Queue(/* args */) : tail(0), head(0), length(0)
{
}

template <class T, unsigned int Queue_Size>
Queue<T, Queue_Size>::~Queue()
{
}

template <class T, unsigned int Queue_Size>
bool Queue<T, Queue_Size>::push_back(T element)
{
	if (length >= Queue_Size)
		return false;
	else
	{
		queue[tail++] = element;
		if (tail >= Queue_Size)
			tail = 0;
		length++;
		return true;
	}
}

template <class T, unsigned int Queue_Size>
void Queue<T, Queue_Size>::brute_push_back(T element)
{
	if (length >= Queue_Size)
	{
		head++;
		if (head >= Queue_Size)
			head = 0;
	}
	else
		length++;

	queue[tail++] = element;
	if (tail >= Queue_Size)
		tail = 0;
}

template <class T, unsigned int Queue_Size>
T Queue<T, Queue_Size>::pop_front()
{
	T element;
	if (length > 0)
	{
		element = queue[head++];
		if (head >= Queue_Size)
			head = 0;
		length--;
	}

	return element;
}

template <class T, unsigned int Queue_Size>
void Queue<T, Queue_Size>::pop_front(int num)
{
	if (num > length)
		num = length;
	length -= num;
	head += num;
	if (head >= Queue_Size)
		head -= Queue_Size;
}

template <class T, unsigned int Queue_Size>
T Queue<T, Queue_Size>::pop_back()
{
	T element;
	if (length > 0)
	{
		tail--;
		if (tail < 0)
			tail = Queue_Size - 1;
		length--;

		return queue[tail];
	}
	else
		return element;
}

template <class T, unsigned int Queue_Size>
void Queue<T, Queue_Size>::pop_back(int num)
{
	if (num > length)
		num = length;
	length -= num;
	tail -= num;
	if (tail < 0)
		tail += Queue_Size;
}

template <class T, unsigned int Queue_Size>
void Queue<T, Queue_Size>::clear()
{
	head = 0;
	tail = 0;
	length = 0;
}

template <class T, unsigned int Queue_Size>
T &Queue<T, Queue_Size>::operator[](unsigned int index)
{
	assert_param(index < length);

	index += head;
	if (index >= Queue_Size)
		index -= Queue_Size;
	return queue[index];
}

template <class T, unsigned int Queue_Size>
bool Queue<T, Queue_Size>::replace(unsigned int index, T element)
{
	//If the index is less than the queue length, replace corresponding data in the queue.
	if (index < length)
	{
		index += head;
		if (index >= Queue_Size)
			index -= Queue_Size;
		queue[index] = element;

		return true;
	}
	else
		return false;
}

template <class T, unsigned int Queue_Size>
bool Queue<T, Queue_Size>::set_tail(int num)
{
	if (num < Queue_Size)
	{
		tail = num;

		return true;
	}
	else
		return false;
}

template <class T, unsigned int Queue_Size>
bool Queue<T, Queue_Size>::set_length(int num)
{
	if (num > Queue_Size)
		return false;
	else
	{
		length = num;
		return true;
	}
}

template class Queue<unsigned char, USART_TXRX_BUFFER_SIZE>;
template class Queue<WavePara, FREQWAVE_BUFFER_SIZE>;