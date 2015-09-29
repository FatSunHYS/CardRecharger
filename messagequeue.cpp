#include "messagequeue.h"

MessageQueue::MessageQueue()
{
	pthread_mutex_init( &this->MessageQueueLock, NULL );
}

MessageQueueNode* MessageQueue::MessageDequeue()
{
	MessageQueueNode* TemperoryNode;

	pthread_mutex_lock( &this->MessageQueueLock );

	if( this->Messages.isEmpty() )
	{
		TemperoryNode = NULL;
	}
	else
	{
		TemperoryNode = this->Messages.dequeue();
	}

	pthread_mutex_unlock( &this->MessageQueueLock );

	return TemperoryNode;
}

void MessageQueue::MessageEnqueue( MessageQueueNode* node)
{
	pthread_mutex_lock( &this->MessageQueueLock );

	this->Messages.enqueue( node );

	pthread_mutex_unlock( &this->MessageQueueLock );
}

MessageQueueNode* MessageQueue::MessageHead()
{
	MessageQueueNode* TemperoryNode;

	pthread_mutex_lock( &this->MessageQueueLock );

	TemperoryNode = this->Messages.head();

	pthread_mutex_unlock( &this->MessageQueueLock );

	return TemperoryNode;
}

bool MessageQueue::QueueIsEmpty()
{
	bool TemperoryBool;

	pthread_mutex_lock( &this->MessageQueueLock );

	TemperoryBool = this->Messages.isEmpty();

	pthread_mutex_unlock( &this->MessageQueueLock );

	return TemperoryBool;
}
