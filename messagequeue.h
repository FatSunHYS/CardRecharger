#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QQueue>

#include <pthread.h>

#include "messagequeuenode.h"


class MessageQueue
{
public:
	explicit MessageQueue();
	MessageQueueNode* MessageDequeue();
	void MessageEnqueue( MessageQueueNode* node );
	MessageQueueNode* MessageHead();
	bool QueueIsEmpty();

private:
	QQueue< MessageQueueNode* > Messages;
	pthread_mutex_t MessageQueueLock;


};




#endif // MESSAGEQUEUE_H
