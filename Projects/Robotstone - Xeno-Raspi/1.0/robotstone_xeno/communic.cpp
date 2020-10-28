/**
 * @file	communic.cpp
 * @author  Matheus Leitzke Pinto <matheus.pinto@ifsc.edu.br>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This module contains the implementation of the publisher/subscriber services.
 */

#include "communic.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "test_led.hpp"

#define SUB_BUFFER_SIZE 30
#define TOPIC_TASK_PRIORITY 90

using namespace std;

namespace RealTime
{

#define comSTART_TASKS_ID 1000


/*PRIVATE FUNCTIONS*/
/*=======================================================================================*/

/**
 * @brief A generic task code that will have an instance by each topic.
 *
 * 		  The tasks related to this function will manage the sending of
 * 		  messages as soon as a message is complete put in its specific
 * 		  topic buffer by a publisher task.
 *
 */
taskFUNCTION(TopicTaskFunc, args)
{
	Topic *topicHandler = (Topic *)args;

	for(;;)
	{
		if(topicHandler->SendMessages() == 1)
		{
			break;
		}
	}

}

/*CLASS METHODS*/
/*=======================================================================================*/

/**
 * @brief The Subscriber class constructor.
 *
 * @param id - A subscriber unique identifier (it is attributed automatically by Communic system).
 * @param topic - The topic identifier.
 * @param topic - The size of the messages in the requested topic.
 */
Subscriber::Subscriber(size_t id, size_t topic, size_t msgSize)
{

	this->id = id;
	this->topic = topic;
	this->messageSize = msgSize;
}

/**
 * @brief Initialize the Subscriber instance parameters.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int Subscriber::Init(void)
{
	char subName[15];
	size_t queueSize = (this->messageSize)*SUB_BUFFER_SIZE;
	int createReturn;

	sprintf(subName, "sub_%d_%d_%d", getpid(), topic, id);
	createReturn = rt_queue_create(&(this->queue), subName, queueSize, Q_UNLIMITED, Q_PRIO);
	if(createReturn)
	{
		// Problem to create subscriber for topic: impossible create queue for subscriber handler
		return RealTime::fail;
	}

	return RealTime::ok;
}

/**
 * @brief Subscriber class destructor.
 */
Subscriber::~Subscriber(void)
{
	rt_queue_delete(&(this->queue));
}

/**
 * @brief Get the topic identifier of the subscriber.
 *
 * @return The topic identifier.
 */
size_t Subscriber::GetTopic(void)
{
	return this->topic;
}

/**
 * @brief Get the subscriber identifier.
 *
 * @return subscriber identifier.
 */
size_t Subscriber::GetID()
{
	return this->id;
}

/**
 * @brief Get the message size related to the subscribed topic.
 *
 * @return The message size in bytes.
 */
size_t Subscriber::GetMessageSize(void)
{
	return this->messageSize;
}

/**
 * @brief Wait to receive a message in the subscribed topic.
 *
 * 		  While a message doesn't arrives, the caller task
 * 		  will be suspended. When a complete message arrives,
 * 		  then the task will exit this method.
 *
 * @param msg - A pointer to a valid buffer at least the size
 * 				of the message.
 *
 * @return 1 - if success;
 * 		   0 - otherwise;
 */
size_t Subscriber::Receive(void *msg)
{
	void *queueMsg = NULL;

	// Take the queue buffer space with the length of message and copy the message content to message pointer parameter
	if(rt_queue_receive(&(this->queue),
			  	  	  (void **)&queueMsg,
			  	  	  TM_INFINITE) < 0)
	{
		return 0;
	}

	if(queueMsg == NULL)
	{
		rt_printf("Problem to get message: queue null for subscriber %d_%d_%d\n", this->id, this->topic, this->id);
		//return 0;
		exit(1);
	}

	memcpy(msg, queueMsg, this->messageSize);

	// The buffer space taken from pointer of "rt_queue_receive" to warning Xenomai that message was read by the task.
	rt_queue_free(&(this->queue), (void*)queueMsg);
	return 1;
}

/**
 * @brief This method is called by the topic gatekeeper task to
 * 		  put a copy of the last message sent to the topic in
 * 		  the subscriber buffer.
 *
 * @param msg - A pointer to a buffer with the copy of the message.
 *
 * @return 1 - if success;
 * 		   0 - otherwise;
 */
size_t Subscriber::PutMessage(void *msg)
{
	void *queueSubMsg = NULL;
	queueSubMsg = rt_queue_alloc(&(this->queue), this->messageSize);

	if(queueSubMsg == NULL)
	{
		rt_printf("Problem put message: impossible to alloc subscriber queue for topic %d\n", this->topic);
		return 0;
		//exit(1);
	}

	memcpy(queueSubMsg, msg, this->messageSize);

	rt_queue_send(&(this->queue), queueSubMsg, this->messageSize, Q_NORMAL);
	return 1;
}

bool Subscriber::operator == (Subscriber &other)
{
	return this->id == other.GetID();
}

bool Subscriber::operator == (size_t id)
{
	return  this->id == id;
}

/**
 * @brief The Publisher class constructor.
 *
 * @param id - A publisher unique identifier (it is attributed automatically by Communic system).
 * @param topic - The topic identifier.
 * @param msgSize - The size of the messages in the requested topic.
 * @param topicHandler - A topic instance reference.
 */
Publisher::Publisher(size_t id, size_t topic, size_t msgSize, void *topicHandler):
		topic(topic),
		topicHandler(topicHandler),
		id(id),
		messageSize(msgSize),
		topicQueue(((Topic*)topicHandler)->GetQueue())
{

}

/**
 * @brief Initialize the Publisher instance parameters.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int Publisher::Init(void)
{
	return RealTime::ok;
}

/**
 * @brief The Publisher class destructor.
 *
 */
Publisher::~Publisher(void)
{

}

/**
 * @brief Get the topic identifier of the publisher.
 *
 * @return The topic identifier.
 */
size_t Publisher::GetTopic(void)
{
	return this->topic;
}

/**
 * @brief Get the publisher identifier.
 *
 * @return publisher identifier.
 */
size_t Publisher::GetID()
{
	return this->id;
}

/**
 * @brief Get the message size related to the publishing topic.
 *
 * @return The message size in bytes.
 */
size_t Publisher::GetMessageSize(void)
{
	return this->messageSize;
}

/**
 * @brief Send a message to the publishing topic.
 *
 * @param msg - A pointer to a valid buffer with
 * 				the message size expected by the topic.
 *
 * @return 1 - if success;
 * 		   0 - otherwise.
 */
size_t Publisher::Send(void *msg)
{
	void *queueMsg;
	// Take the queue buffer space with the length of message and copy the message content to buffer
	queueMsg = rt_queue_alloc((RT_QUEUE*)(this->topicQueue),this->messageSize);

	if(queueMsg == NULL)
	{
		return 0;
	}

	memcpy(queueMsg, msg, this->messageSize);

	// Send the message in the queue buffer for all tasks waiting the message
	rt_queue_send((RT_QUEUE*)(this->topicQueue), (void*)queueMsg, this->messageSize, Q_BROADCAST);

	return 1;
}

bool Publisher::operator == (Publisher &other)
{
	return this->id == other.GetID();
}


bool Publisher::operator == (size_t id)
{
	return  this->id == id;
}

/**
 * @brief The Topic class constructor.
 *
 * @param topic - The topic identifier.
 * @param messageSize - The size of the messages in the requested topic.
 *
 */
Topic::Topic(uint8_t topic, size_t messageSize):
		pubNumber(0),
		subNumber(0),
		topicTask(NULL),
		tasksNumber(0)
{
	this->messageLen = messageSize;
	this->topicID = topic;
}

/**
 * @brief Initialize the Topic instance parameters.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int Topic::Init()
{
	int createReturn;
	size_t queueSize = this->messageLen;
	char *topicName;

	topicName = (char *)Heap::Alloc(sizeof(strlen("topic_xx")));

	if(!topicName)
	{
		return RealTime::fail;
	}

	sprintf(topicName, comTOPIC_NAME, this->topicID);

	void* p = Heap::Alloc(sizeof(Task));

	topicTask = new(p) Task(TopicTaskFunc, TOPIC_TASK_PRIORITY, 100000, this, comSTART_TASKS_ID + this->topicID, "");

	// If no queue related to topic exist, so there it is no exist.
	if(rt_queue_bind(&(this->queue), topicName, 1000000000))
	{
		createReturn = rt_mutex_create(&(this->mutex), topicName);
		if(createReturn)
		{
			// Problem to create topic: impossible create topic
			return RealTime::fail;
		}

		/*
		* Arguments: -*queue descriptor,
		*            -string with topic name related to the queue,
		*            -queue size with message length + the message header length used by Xenomai,
		*            -Q_UNLIMITED indicates no error if buffer overflow,
		*            -Q_PRIO indicate that the message from the highest priority task that arrives is the first that goes.
		*/
		createReturn = rt_queue_create(&(this->queue),
										topicName,
					      	  	  	  	queueSize,
					      	  	  	  	Q_UNLIMITED,
					      	  	  	  	Q_PRIO);

		if(createReturn)
        {
			// Problem to create topic: impossible create topic queue
			return RealTime::fail;
		}

		if(rt_heap_create(&(this->heap), topicName, 2, H_SINGLE) != 0)
		{
			// Problem to create topic: impossible create heap for topic
			return RealTime::fail;
		}
		if(rt_heap_alloc(&(this->heap),
						  2,
						  TM_INFINITE,
						  (void**)&(this->tasksNumber))
				!= 0)
		{
			// Problem to create topic: impossible allocate heap for topic
			return RealTime::fail;
		}

		*(this->tasksNumber) = 0;

	}
	else
	{
		createReturn = rt_mutex_bind(&(this->mutex), topicName, 1000000000);
		if(createReturn)
        {
			// Problem to create topic locally: impossible bind topic mutex
			return RealTime::fail;
		}
		createReturn = rt_heap_bind(&(this->heap), topicName, 1000000000);
		if(createReturn)
        {
			// Problem to create topic locally: impossible bind topic heap
			return RealTime::fail;
		}
		if(rt_heap_alloc(&(this->heap),
						  2,
						  TM_INFINITE,
						  (void**)&(this->tasksNumber))
				!= 0)
		{
			// Problem to create topic: impossible allocate heap locally for topic
			return RealTime::fail;
		}

	}

	this->topicTask->Start();
	return RealTime::ok;
}

/**
 * @brief The Topic class destructor.
 */
Topic::~Topic(void)
{
	*(this->tasksNumber) = *(this->tasksNumber) - subNumber - pubNumber;

	this->topicTask->Unblock();
	this->topicTask->Join();

	if(*(this->tasksNumber) == 0)
	{
		rt_queue_delete(&(this->queue));
		rt_heap_free(&this->heap, this->tasksNumber);
		rt_heap_delete(&this->heap);
		rt_mutex_delete(&(this->mutex));
	}
	else
	{
		rt_queue_unbind(&(this->queue));
		rt_mutex_unbind(&(this->mutex));
	}
}

/**
 * @brief Get the topic queue.
 *
 * 		  This queue is shared between the publisher and the gatekeeper tasks.
 *
 * @return A pointer to the topic queue.
 */
comQueue_t * Topic::GetQueue(void)
{
	return &(this->queue);
}

/**
 * @brief Get the topic identifier.
 *
 * @return The topic identifier.
 */
size_t Topic::GetTopicID(void)
{
	return this->topicID;
}

/**
 * @brief Insert a subscriber instance in the topic list.
 *
 * @param A subscriber instance pointer.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
size_t Topic::AddSubscriber(Subscriber* sub)
{
	this->subsList.InsertTail(sub);
	this->subNumber += 1;
	*(this->tasksNumber) = *(this->tasksNumber) + 1;
	return RealTime::ok;
}

/**
 * @brief Insert a publisher instance in the topic list.
 *
 * @param A publisher instance pointer.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
size_t Topic::AddPublisher(Publisher* pub)
{
	this->pubsList.InsertTail(pub);
	this->pubNumber += 1;
	*(this->tasksNumber) = *(this->tasksNumber) + 1;
	return RealTime::ok;
}

/**
 * @brief Remove a subscriber instance in the topic list.
 *
 * @param A subscriber instance pointer.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
size_t Topic::RemoveSubscriber(Subscriber* sub)
{
	this->subsList.Remove(sub);
	this->subNumber -= 1;
	*(this->tasksNumber) = *(this->tasksNumber) - 1;
	return RealTime::ok;
}

/**
 * @brief Remove a publisher instance in the topic list.
 *
 * @param A subscriber instance pointer.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
size_t Topic::RemovePublisher(Publisher* pub)
{
	this->pubsList.Remove(pub);
    this->pubNumber -= 1;
	*(this->tasksNumber) = *(this->tasksNumber) - 1;;
	return RealTime::ok;
}

/**
 * @brief Get the topic task gatekeeper object handler.
 *
 * @return The pointer to the task gatekeeper object handler.
 */
Task* Topic::GetTask(void)
{
	return this->topicTask;
}

/**
 * @brief Get the total number of publishers and subscribers
 * 		  instances related to the topic in all possible nodes.
 *
 * @return The number of publishers and subscribers instances.
 */
size_t Topic::GetObjectsNumGlobal(void)
{
	return *(this->tasksNumber);
}

/**
 * @brief Get the number of publishers and subscribers
 * 		  instances related to the topic in the local node.
 *
 * @return The number of publishers and subscribers instances.
 */
size_t Topic::GetObjectsNumLocal(void)
{
	return this->subNumber+this->pubNumber;
}

bool Topic::HasSubID(size_t id)
{
	return this->subsList.HasKey(id);
}

bool Topic::HasPubID(size_t id)
{
	return this->pubsList.HasKey(id);
}

/**
 * @brief When a topic gatekeeper task go out from suspended
 * 		  it calls this method.
 *
 * 		  The topic gatekeeper task put a copy of the message
 * 		  from the topic queue (sent previously by a publisher) to
 * 		  ever subsciber queue in the topic list.
 *
 *
 * @return - 1 if success;
 * 		   - 0 otherwise.
 */
size_t Topic::SendMessages(void)
{
	message_t *queuePointer = NULL;

	// Take the queue buffer space with the length of message and copy the message content to message pointer parameter
	rt_queue_receive(&(this->queue), (void **)&queuePointer, TM_INFINITE);

	if(queuePointer == NULL)
	{
		return 1; // Topic was deleted. Get out, to finish.
	}

	// Send for each subscriber the message on topic
	for(this->subsList.InitIteration(); !this->subsList.IsEnd(); ++this->subsList)
	{
	  	subsList.GetValue()->PutMessage(queuePointer);
	}

    rt_queue_free(&(this->queue), queuePointer);
    return 0;
}

bool Topic::operator == (Topic &other)
{
	return this->topicID == other.GetTopicID();
}

bool Topic::operator == (size_t topicID)
{
	return  this->topicID == topicID;
}


namespace Communic
{

//static List<Topic, size_t> topicsList;
#define MAX_TOPICS 256

static Topic* topicsList[MAX_TOPICS];

static RT_MUTEX comMutex;

/**
 * @brief Initialize the Communic subsystem parameters and services.
 *
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int Init(void)
{
	if(rt_mutex_bind(&comMutex, "commtx", 10000000))
	{
		int createReturn;

		createReturn = rt_mutex_create(&comMutex, "commtx");
		if(createReturn)
		{
			// Problem in Communic: impossible create mutex
			return RealTime::fail;
		}
	}
	for(size_t i = 0; i < MAX_TOPICS; ++i)
	{
		topicsList[i] = NULL;
	}

	return RealTime::ok;
}

/**
 * @brief Make a request for subscribing in a specific topic.
 *
 * @param topic - The topic identifier (a decimal number defined by the user).
 * @param msgSize - The size of the message to be expected in this topic.
 *
 * @return - A valid Subscriber instance if success;
 * 		   - NULL otherwise.
 */
Subscriber* SubscribingRequest(size_t topic, uint64_t msgSize){

	rt_mutex_acquire(&comMutex, TM_INFINITE); // nobody can request for publish, subscribe, send or receive messages

	Subscriber *subHandler;
	int newTopicFlag = 0;

	// Get the list of all topics handlers from communic system
	Topic * topicHandler = topicsList[topic];

	// Topic not create previously in executable
	if(topicHandler == NULL)
	{
		newTopicFlag = 1; // indicate the creation from a new topic

		void * p = RealTime::Heap::Alloc(sizeof(Topic));
		if(p == NULL) // impossible create space for topic
		{
			rt_mutex_release(&comMutex);
			return NULL;
		}
		else
		{
			// Create the topic
			topicHandler = new(p) Topic(topic, msgSize);

			if(topicHandler->Init() == RealTime::fail)
			{
				topicsList[topic] = NULL;
				topicHandler->~Topic();
				RealTime::Heap::Free(topicHandler);

				rt_mutex_release(&comMutex);
				return NULL;
			}

			topicsList[topic] = topicHandler;
		}
	}

	void *p = RealTime::Heap::Alloc(sizeof(Subscriber));
	if(p == NULL) // impossible create space for subscriber
	{
		if(newTopicFlag) // if topic was not created before, erase it
		{
			topicsList[topic] = NULL;
			topicHandler->~Topic();
			RealTime::Heap::Free(topicHandler);
		}

		rt_mutex_release(&comMutex);
		return NULL;
	}

	// Create a new subscriber with a new unique ID
	subHandler = new(p) Subscriber(GetValidSubID(topicHandler), topic, msgSize);

	if(subHandler->Init() == RealTime::fail)
	{
		if(newTopicFlag) // if topic was not created before, erase it
		{
			topicsList[topic] = NULL;
			topicHandler->~Topic();
			RealTime::Heap::Free(topicHandler);
		}
		else // just delete the subscriber
		{
			subHandler->~Subscriber();
			RealTime::Heap::Free(subHandler);
		}

		rt_mutex_release(&comMutex);
		return NULL;
	}

	topicHandler->AddSubscriber(subHandler);

	rt_mutex_release(&comMutex);
	return subHandler;
}

/**
 * @brief Make a request for publishing in a specific topic.
 *
 * @param topic - The topic identifier (a decimal number defined by the user).
 * @param msgSize - The size of the message to be expected in this topic.
 *
 * @return - A valid Publisher instance if success;
 * 		   - NULL otherwise.
 */
Publisher* PublishingRequest(size_t topic, uint64_t msgSize){

	rt_mutex_acquire(&comMutex, TM_INFINITE); // nobody can request for publish, subscribe, send or receive messages

	Publisher *pubHandler;
	int newTopicFlag = 0;

	// Get the list of all topics handlers from communic system
	Topic * topicHandler = topicsList[topic];

	// Topic not create previously in executable
	if(topicHandler == NULL)
	{
		newTopicFlag = 1; // indicate the creation from a new topic

		void * p = RealTime::Heap::Alloc(sizeof(Topic));
		if(p == NULL) // impossible create space for topic
		{
			rt_mutex_release(&comMutex);
			return NULL;
		}
		else
		{
			// Create the topic
			topicHandler = new(p) Topic(topic, msgSize);

			if(topicHandler->Init() == RealTime::fail)
			{
				topicsList[topic] = NULL;
				topicHandler->~Topic();
				RealTime::Heap::Free(topicHandler);

				rt_mutex_release(&comMutex);
				return NULL;
			}

			topicsList[topic] = topicHandler;
		}
	}

	void *p = RealTime::Heap::Alloc(sizeof(Publisher));
	if(p == NULL) // impossible create space for publisher
	{
		if(newTopicFlag) // if topic was not created before, erase it
		{
			topicsList[topic] = NULL;
			topicHandler->~Topic();
			RealTime::Heap::Free(topicHandler);
		}

		rt_mutex_release(&comMutex);
		return NULL;
	}

	// Create a new publisher with a new unique ID
	pubHandler = new(p) Publisher(GetValidPubID(topicHandler),
								  topic,
								  msgSize,
								  topicHandler);

	if(pubHandler->Init() == RealTime::fail)
	{
		if(newTopicFlag) // if topic was not created before, erase it
		{
			topicsList[topic] = NULL;
			topicHandler->~Topic();
			RealTime::Heap::Free(topicHandler);
		}
		else // just delete the subscriber
		{
			pubHandler->~Publisher();
			RealTime::Heap::Free(pubHandler);
		}

		rt_mutex_release(&comMutex);
		return NULL;
	}

	topicHandler->AddPublisher(pubHandler);

	rt_mutex_release(&comMutex);
	return pubHandler;
}

/**
 * @brief Request to leave the publishing in a specific topic.
 *
 * @param pubHandler - A valid Publisher instance.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int LeaveTopic(Publisher* pubHandler){
	rt_mutex_acquire(&comMutex, TM_INFINITE); // nobody can request for publish, subscribe, send or receive messages
	size_t topic = pubHandler->GetTopic();
	Topic *topicHandler = topicsList[topic];

	if(topicHandler == NULL)
	{
		// Publisher not valid for unpublish
		return RealTime::fail;
	}

	topicHandler->RemovePublisher(pubHandler);
	pubHandler->~Publisher();
	RealTime::Heap::Free(pubHandler);
	pubHandler = NULL;

	if(topicHandler->GetObjectsNumLocal() == 0)
	{
		topicsList[topic] = NULL;
		topicHandler->~Topic();
		RealTime::Heap::Free(topicHandler);
	}

	rt_mutex_release(&comMutex);
	return RealTime::ok;
}

/**
 * @brief Request to leave the subscribing in a specific topic.
 *
 * @param pubHandler - A valid Subscriber instance.
 *
 * @return - RealTime::ok if success;
 * 		   - RealTime::fail otherwise.
 */
int LeaveTopic(Subscriber* subHandler){
	rt_mutex_acquire(&comMutex, TM_INFINITE);

	size_t topic = subHandler->GetTopic();

	Topic * topicHandler = topicsList[topic];

	if(topicHandler == NULL)
	{
		// Subscriber not valid for unsubscribe
		return RealTime::fail;
	}

	topicHandler->RemoveSubscriber(subHandler);

	subHandler->~Subscriber();
	RealTime::Heap::Free(subHandler);
	subHandler = NULL;

	if(topicHandler->GetObjectsNumLocal() == 0)
	{
		topicsList[topic] = NULL;
		topicHandler->~Topic();
		RealTime::Heap::Free(topicHandler);
	}

	rt_mutex_release(&comMutex);
	return RealTime::ok;
}

size_t GetValidSubID(Topic* topic)
{
	size_t id;

	do
	{
		id = Timer::GetRandomValue();
	}while(topic->HasSubID(id));

	return id;
}

size_t GetValidPubID(Topic* topic)
{
	size_t id;
	do
	{
		id = Timer::GetRandomValue();
	}while(topic->HasPubID(id));

	return id;
}

void ClearAll(void)
{
	//topicsList.Clear();
}
}


}
/***************************************************************************************
 * END: Module - communic.cpp
 ***************************************************************************************/
