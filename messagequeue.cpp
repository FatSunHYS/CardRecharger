#include "messagequeue.h"


MessageQueueNode MessageQueue::dequeue()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	return this->Messages.dequeue();
}

void MessageQueue::enqueue(const MessageQueueNode &node)
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	this->Messages.enqueue( node );
}

MessageQueueNode MessageQueue::head()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	return this->Messages.head();
}

bool MessageQueue::IsEmpty()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	return this->Messages.isEmpty();
}
