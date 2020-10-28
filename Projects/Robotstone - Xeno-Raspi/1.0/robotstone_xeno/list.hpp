/***************************************************************************************
 * Module      : list.hpp
 * Revision    : 1.0
 * Date        : 16/10/2018
 * Description : It contains the implementations of List and ListNode template classes.
 * 			     The List class is implemented with doubly-linked list and provides
 * 			     methods for insertion, remotion, search and many others.
 * 			     Any object in the list must have a key.
 * 			     For its manipulation with any kind of object, the object class must implement:
 * 			     	- A constructor with void arguments, setting if object is valid;
 * 			     	- A "size_t isInitiated(void)" method for indicates that object is
 * 			     	  valid (return 1) or not valid (return 0) when List return a object
 * 			     	  that is not valid.
 * 			     	- A "size_t operator == (size_t key)" and "size_t operator == (Object &other)"
 * 			     	  for comparing object with a key or comparing object with another object.
 *
 * Comments    : Copyright (c) 2018, Matheus Leitzke Pinto.
 * 		 		 All rights reserved.
 *
 * 		 		 Redistribution and use in source and binary forms, with or without modification,
 * 		 		 are permitted provided that the following conditions are met:
 * 		 		 	- Redistributions of source code must retain the above copyright notice, this list
 * 		 		 	  of conditions and the following disclaimer.
 *
 * 					- Redistributions in binary form must reproduce the above copyright notice, this
 * 					  list of conditions and the following disclaimer in the documentation and/or
 * 					  other materials provided with the distribution.
 * Author(s)   : Matheus Leitzke Pinto
 ***************************************************************************************/


#ifndef LIST_HPP_
#define LIST_HPP_

#include "alchemy/heap.h"
#include <stdio.h>
#include "test_led.hpp"
#include "rtheap.hpp"

/*
 * list.cpp
 *
 *  Created on: 20/11/2018
 *      Author: matheus
 */
#include <stddef.h>

template<class T>
class ListNode{
private:
	T* value;
	ListNode* next;
	ListNode* previous;
public:
	ListNode(T* value);
	T* GetValue();
	void SetNext(ListNode* next);
	void SetPrevious(ListNode* prev);
	ListNode* GetNext(void);
	ListNode* GetPrevious(void);
};

template<class T, class U>
class List{
public:
	static const size_t fromBegin = 0;
	static const size_t fromEnd = 1;
private:
	ListNode<T>* head;
	ListNode<T>* tail;
	ListNode<T>* actual;
	size_t size;
	bool isBegin;
	bool isEnd;
public:
	List();
	void InsertTail(T* value);
	void InsertHead(T* value);
	void Remove(T* value);
	void Remove(U key);
	T* RemoveTail(void);
	T* RemoveHead(void);
	size_t HasValue(T* value);
	size_t HasKey(U key);
	void Clear(void);
	T* GetValue(U key);
	T* GetValue(void);
	size_t isInvalid(T* value);
	size_t IsEmpty();
	ListNode<T>* GetHead(void);
	void InitIteration(void);
	void InitIteration(size_t dir);
	size_t IsEnd(void);
	size_t IsBegin(void);
	T* operator ++ (void);
	T* operator -- (void);
};

template<class T>
ListNode<T>::ListNode(T* value):
value(value), next(NULL), previous(NULL)
{

}

template<class T>
T* ListNode<T>::GetValue()
{
	return this->value;
}

template<class T>
void ListNode<T>::SetNext(ListNode* next)
{
	this->next = next;
}

template<class T>
void ListNode<T>::SetPrevious(ListNode* prev)
{
	this->previous = prev;
}

template<class T>
ListNode<T>* ListNode<T>::GetNext(void)
{
	return this->next;
}

template<class T>
ListNode<T>* ListNode<T>::GetPrevious(void)
{
	return this->previous;
}

template<class T, class U>
List<T,U>::List():
		head(NULL), tail(NULL), actual(NULL), size(0), isBegin(1), isEnd(1)
{

}

template<class T, class U>
void List<T,U>::InsertTail(T* value)
{
	this->actual = NULL;
	void *p = RealTime::Heap::Alloc(sizeof(ListNode<T>));

	ListNode<T> *node = new(p) ListNode<T>(value);

	if(this->size == 0)
	{
		node->SetNext(NULL);
		node->SetPrevious(NULL);
		this->head = this->tail = node;
	}
	else
	{
		node->SetPrevious(this->tail);
		node->SetNext(NULL);

		this->tail->SetNext(node);

		this->tail = node;
	}

	this->size++;
}

template<class T, class U>
void List<T,U>::InsertHead(T* value)
{
	this->actual = NULL;
	void *p = RealTime::Heap::Alloc(sizeof(ListNode<T>));

	ListNode<T> *node = new (p) ListNode<T>(value);

	if(this->size == 0)
	{
		node->SetNext(NULL);
		node->SetPrevious(NULL);
		this->head = this->tail = node;
	}
	else
	{
		node->SetNext(this->head);
		node->SetPrevious(NULL);

		this->head->SetNext(node);

		this->head = node;
	}

	this->size++;
}


template<class T, class U>
void List<T,U>::Remove(T* value)
{
	if(value == NULL)
	{
		return;
	}

	ListNode<T> *iterator = NULL;
	this->actual = NULL;
	if(this->size == 1)
	{
		if(*(this->head->GetValue()) == *value)
		{
			iterator = this->head;
			this->head = this->tail = NULL;
			this->size = 0;
		}
	}
	else
	{
		for(iterator = this->head; iterator != NULL; iterator = iterator->GetNext())
		{
			if(*(iterator->GetValue()) == *value)
			{
				if(iterator->GetPrevious() != NULL)
				{
					iterator->GetPrevious()->SetNext(iterator->GetNext());
				}
				else
				{
					this->head = iterator->GetNext();
					this->head->SetPrevious(NULL);
				}
				if(iterator->GetNext() != NULL)
				{
					iterator->GetNext()->SetPrevious(iterator->GetPrevious());
				}
				else
				{
					this->tail = iterator->GetPrevious();
					this->tail->SetNext(NULL);
				}
				this->size--;
				break;
			}
		}

	}

	if(iterator != NULL)
	{
		RealTime::Heap::Free(iterator);
	}
}

template<class T, class U>
void List<T,U>::Remove(U key)
{
	ListNode<T> *iterator = NULL;
	this->actual = NULL;
	if(this->size == 1)
	{
		if(*(this->head->GetValue()) == key)
		{
			iterator = this->head;
			this->head = this->tail = NULL;
			this->size = 0;
		}
	}
	else
	{
		for(iterator = this->head; iterator != NULL; iterator = iterator->GetNext())
		{
			if(*(iterator->GetValue()) == key)
			{
				if(iterator->GetPrevious() != NULL)
				{
					iterator->GetPrevious()->SetNext(iterator->GetNext());
				}
				else
				{
					this->head = iterator->GetNext();
					this->head->SetPrevious(NULL);
				}
				if(iterator->GetNext() != NULL)
				{
					iterator->GetNext()->SetPrevious(iterator->GetPrevious());
				}
				else
				{
					this->tail = iterator->GetPrevious();
					this->tail->SetNext(NULL);
				}
				this->size--;
				break;
			}
		}

	}

	if(iterator != NULL)
	{
		RealTime::Heap::Free(iterator);
	}
}

template<class T, class U>
T* List<T,U>::RemoveTail(void)
{
	this->actual = NULL;

	if(this->size == 0)
	{
		return NULL;
	}

	ListNode<T> *tempNode = this->tail;
	T* value = tempNode->GetValue();
	if(this->size == 1)
	{
		this->head = this->tail = NULL;
	}
	else
	{
		this->tail = this->tail->GetPrevious();
		this->tail->SetNext(NULL);
	}

	RealTime::Heap::Free(tempNode);
	this->size--;
	return value;
}

template<class T, class U>
T* List<T,U>::RemoveHead(void)
{
	this->actual = NULL;

	if(this->size == 0)
	{
		return NULL;
	}

	ListNode<T> *tempNode = this->head;
	T* value = tempNode->GetValue();
	if(this->size == 1)
	{
		this->head = this->tail = NULL;
	}
	else
	{
		this->head = this->head->GetNext();
		this->head->SetPrevious(NULL);
	}

	RealTime::Heap::Free(tempNode);
	this->size--;
	return value;
}

template<class T, class U>
size_t List<T,U>::HasValue(T* value)
{
	for(ListNode<T> *iterator = this->head; iterator != NULL; iterator = iterator->GetNext())
	{
		if(*(iterator->GetValue()) == *value)
		{
			return 1;
		}
	}
	return 0;
}

template<class T, class U>
size_t List<T,U>::HasKey(U key)
{
	for(ListNode<T> *iterator = this->head; iterator != NULL; iterator = iterator->GetNext())
		{
			if(*(iterator->GetValue()) == key)
			{
				return 1;
			}
		}
		return 0;
}

template<class T, class U>
void List<T,U>::Clear(void)
{
	ListNode<T> *temp, *temp2 = NULL;
	for(temp = this->head; temp != NULL; temp = temp2)
	{
		temp2 = temp->GetNext();
		temp->GetValue()->~T();
		RealTime::Heap::Free(temp->GetValue());
		RealTime::Heap::Free(temp);
	}
}

template<class T, class U>
T* List<T,U>::GetValue(U key)
{
	ListNode<T> *iterator;

	for(iterator = this->head; iterator != NULL; iterator = iterator->GetNext())
	{
		if(*(iterator->GetValue()) == key)
		{
			return iterator->GetValue();
		}
	}

	return NULL;
}

template<class T, class U>
T* List<T,U>::GetValue(void)
{
	return this->actual->GetValue();
}

template<class T, class U>
size_t List<T,U>::IsEmpty()
{
	return !(this->size);
}

template<class T, class U>
ListNode<T>* List<T,U>::GetHead(void)
{
	return this->head;
}

template<class T, class U>
void List<T,U>::InitIteration(void)
{
	if(this->size == 0)
	{
		this->isEnd = 1;
	}
	else
	{
		this->isEnd = 0;
	}
	this->actual = this->head;
}

template<class T, class U>
void List<T,U>::InitIteration(size_t dir)
{
	if(dir == List<T,U>::fromBegin)
	{
		if(this->size == 0)
		{
			this->isEnd = 1;
		}
		else
		{
			this->isEnd = 0;
		}

		this->actual = this->head;
	}
	else
	{
		if(this->size == 0)
		{
			this->isBegin = 1;
		}
		else
		{
			this->isBegin = 0;
		}

		this->actual = this->tail;
	}
}

template<class T, class U>
size_t List<T,U>::IsEnd(void)
{
	return this->isEnd;
}

template<class T, class U>
size_t List<T,U>::IsBegin(void)
{
	return this->isBegin;
}

template<class T, class U>
T* List<T,U>::operator ++ (void)
{
	if(this->actual == NULL)
	{
		return NULL;
	}

	if(*(this->actual->GetValue()) == *(this->tail->GetValue()))
	{
		this->actual = NULL;
		this->isEnd = 1;
		return NULL;
	}

	this->actual = this->actual->GetNext();
	return this->actual->GetValue();
}

template<class T, class U>
T* List<T,U>::operator -- (void)
{
	if(this->actual == NULL)
	{
		return NULL;
	}

	if(*(this->actual->GetValue()) == *(this->head->GetValue()))
	{
		this->actual = NULL;
		this->isBegin = 1;
		return NULL;
	}

	this->actual = this->actual->GetPrevious();
	return this->actual->GetValue();
}

#endif /* LIST_HPP_ */
