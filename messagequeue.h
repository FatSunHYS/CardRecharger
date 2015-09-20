#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>

#include "messagequeuenode.h"


class MessageQueue : public QObject
{
	Q_OBJECT
public:
	explicit MessageQueue(QObject *parent = 0);
	MessageQueueNode dequeue();
	void enqueue( const MessageQueueNode& node );
	MessageQueueNode head();
	bool IsEmpty();

signals:

public slots:


private:
	QQueue<MessageQueueNode> Messages;
	QMutex MessageMutex;


};

#endif // MESSAGEQUEUE_H
