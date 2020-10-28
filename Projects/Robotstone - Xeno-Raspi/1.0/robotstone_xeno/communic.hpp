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


#ifndef RT_COMM_HPP_
#define RT_COMM_HPP_


/*MODULE INCLUDES*/
/*=======================================================================================*/

#include "task.hpp"
#include "stdout.hpp"
#include "realtime.hpp"


/*IMPLEMENTATION SPECIFIC INCLUDES*/
/*=======================================================================================*/

#include <alchemy/queue.h>
#include <alchemy/heap.h>
#include <alchemy/mutex.h>
#include "stdio.h"
#include "stdlib.h"
#include "list.hpp"
#include "rtheap.hpp"

namespace RealTime
{

/*MACROS*/
/*=======================================================================================*/

#define comTOPIC_NAME	"topic_%d"


/*MODULE TYPES*/
/*=======================================================================================*/

typedef uint8_t message_t;
typedef RT_QUEUE comQueue_t;
typedef RT_MUTEX comMutex_t;

/*FUNCTIONS*/
/*=======================================================================================*/

void TopicTaskFunc(void *args);

/*CLASSES*/
/*=======================================================================================*/

/**
 * @brief This class represents a subscribing link to a specific topic.
 *
 * 		  The task that wants to subscribing in a topic, must instantiate
 * 		  this class and use it as the link to the topic.
 *
 */
class Subscriber
{
private:
	size_t id; /**< An unique subscriber identifier. */
	size_t topic; /**< Topic subscribed. */
	size_t messageSize; /**< The message size expected to receive in the topic. */
	RT_QUEUE queue; /**< A queue to buffer the incoming message to subscriber. */

public:
	Subscriber(size_t id, size_t topic, size_t msgSize);
	~Subscriber(void);
	int Init(void);
	size_t GetTopic(void);
	size_t GetID();
	size_t GetMessageSize(void);
	size_t Receive(void *msg);
	size_t PutMessage(void *msg);
	bool operator == (Subscriber &other);
	bool operator == (size_t id);
};

/**
 * @brief This class represents a publishing link to a specific topic.
 *
 * 		  The task that wants to publishing in a topic, must instantiate
 * 		  this class and use it as the link to the topic.
 *
 */
class Publisher
{
private:
	size_t topic; /**< Topic to publish. */
	void * topicHandler; /**< Topic instance reference. */
	size_t id; /**< An unique publisher identifier. */
	size_t messageSize; /**< The size of the message that is sent in the topic. */
	comQueue_t *topicQueue; /**< The shared topic queue reference. */
public:
	Publisher(size_t id, size_t topic, size_t msgSize, void *topicHandler);
	~Publisher(void);
	int Init(void);
	size_t GetTopic(void);
	size_t GetID();
	size_t GetMessageSize(void);
	size_t Send(void *msg);
	bool operator == (Publisher &other);
	bool operator == (size_t id);
};

/**
 * @brief This class represents a specific topic.
 *
 * 		  This class is invisible to the tasks. It will be used internally by
 * 		  the Communic system.
 *
 */
class Topic
{
private:
	uint16_t *tasksNumber; /**< Number of tasks using the topic instance. */
	uint32_t messageLen; /**< The size of the message that is sent in the topic. */
	List<RealTime::Subscriber, size_t> subsList; /**< The list of subscribers instances related to the topic. Each key in the list is an instance identifier. */
	List<RealTime::Publisher, size_t> pubsList;  /**< The list of publishers instances related to the topic. Each key in the list is an instance identifier. */
	uint8_t topicID; /**< An unique topic identifier. */
	uint8_t pubNumber; /**< The number of publishers instances related to the topic. */
	uint8_t subNumber; /**< The number of subscribers instances related to the topic. */
	RT_HEAP heap;
	comQueue_t queue; /**< The topic queue that is shared with publishers and gatekeeper topic tasks. */
	comMutex_t mutex; /**< Protected critical sessions in topic services. */

public:
	Task* topicTask;
	Topic(uint8_t topic, size_t messageSize);
	~Topic(void);
	int Init();
	size_t GetTopicID(void);
	size_t AddSubscriber(Subscriber* sub);
	size_t AddPublisher(Publisher* pub);
	size_t RemoveSubscriber(Subscriber* sub);
	size_t RemovePublisher(Publisher* pub);
	Task* GetTask(void);
	size_t GetObjectsNumGlobal(void);
	size_t GetObjectsNumLocal(void);
	message_t * GetFromQueue(void);
	void ReleaseQueue(message_t* queuePointer);
	comQueue_t * GetQueue(void);
	bool operator == (Topic &other);
	bool operator == (size_t topicID);
	bool HasSubID(size_t id);
	bool HasPubID(size_t id);

private:
	size_t SendMessages(void);
	friend taskFUNCTION(TopicTaskFunc, args);
};

/**
 * @brief Namespace related to the communication subsytem services.
 */
namespace Communic{
	static const uint8_t  maxTasks = 30;
	static const uint8_t  maxTasksPerTopic = 5;
	static const uint8_t  maxTopics = 20;
	static const uint8_t  minTasksID = 50;
	static const uint8_t  maxTasksID = minTasksID+maxTasks;

	int Init(void);
	Subscriber* SubscribingRequest(size_t topic, uint64_t msgSize);
	int LeaveTopic(Subscriber* subHandler);
	Publisher* PublishingRequest(size_t topic, uint64_t msgSize);
	int LeaveTopic(Publisher* pubHandler);
	Topic* GetTopicHandler(size_t topic);
	size_t GetValidSubID(Topic* topic);
	size_t GetValidPubID(Topic* topic);
	void ClearAll(void);

};

}

#endif /* RT_COMM_HPP_ */

/***************************************************************************************
 * END: Module - communic.hpp
 ***************************************************************************************/
