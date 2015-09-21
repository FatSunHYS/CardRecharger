#ifndef MESSAGEQUEUENODE_H
#define MESSAGEQUEUENODE_H

#include <QObject>

class MessageQueueNode : public QObject
{
	Q_OBJECT
public:
	enum MessageGroupIDEnum
	{
		RechargerMessages = 0,
		AdvertisementMessages
	};

	int MessageRequestID;
	bool IsError;
	QString MessageContent;
	int MessageGroupID;

	explicit MessageQueueNode(QObject *parent = 0);
	explicit MessageQueueNode( const MessageQueueNode& node );
	MessageQueueNode& operator =( MessageQueueNode node );


signals:

public slots:




};

#endif // MESSAGEQUEUENODE_H
