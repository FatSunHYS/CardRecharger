#ifndef MESSAGEHANDLING_H
#define MESSAGEHANDLING_H

#include <pthread.h>

#include "messagequeue.h"
#include "messagequeuenode.h"


class MessageHandling
{

public:
	enum
	{
		RechargerMessages = 0,
		AdvertisementMessages
	}MessageGroupIDEnum;

	enum
	{
		GetSysTime = 0,
		Login
	}MessageAppID;

	MessageQueue* MessageQueuePointer;

	static MessageHandling* GetInstance();
	void ParsingRechargerMessages( MessageQueueNode* message );
	bool CreatePThread();



private:
	static MessageHandling* PrivateInstance;
	pthread_t MessageHandlingPthreadID;

	explicit MessageHandling();

};

void* MessageHandler( void* arg );

#endif // RECHARGERMESSAGEHANDLING_H
