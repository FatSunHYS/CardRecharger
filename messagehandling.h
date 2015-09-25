#ifndef MESSAGEHANDLING_H
#define MESSAGEHANDLING_H

#include <QObject>
#include <QThread>
#include "messagequeue.h"
#include "messagequeuenode.h"


class MessageHandling : public QThread
{
	Q_OBJECT
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

protected:
	void run();

signals:

public slots:


private:
	static MessageHandling* PrivateInstance;

	explicit MessageHandling(QObject *parent = 0);

};

#endif // RECHARGERMESSAGEHANDLING_H
