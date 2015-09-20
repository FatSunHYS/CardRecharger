#ifndef MESSAGEQUEUENODE_H
#define MESSAGEQUEUENODE_H

#include <QObject>

class MessageQueueNode : public QObject
{
	Q_OBJECT
public:

	int MessageRequestID;
	bool IsError;
	QString MessageContent;

	explicit MessageQueueNode(QObject *parent = 0);
	explicit MessageQueueNode( const MessageQueueNode& node );
	MessageQueueNode& operator =( MessageQueueNode node );


signals:

public slots:




};

#endif // MESSAGEQUEUENODE_H
