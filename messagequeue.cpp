#include "messagequeue.h"

MessageQueue::MessageQueue(QObject *parent ) : QObject( parent )
{

}

MessageQueueNode* MessageQueue::MessageDequeue()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	if( this->Messages.isEmpty() )
	{
		return NULL;
	}
	else
	{
		return this->Messages.dequeue();
	}
}

void MessageQueue::MessageEnqueue( MessageQueueNode* node)
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	this->Messages.enqueue( node );
}

MessageQueueNode* MessageQueue::MessageHead()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	return this->Messages.head();
}

bool MessageQueue::QueueIsEmpty()
{
	QMutexLocker TemperoryLocker( &this->MessageMutex );

	return this->Messages.isEmpty();
}
