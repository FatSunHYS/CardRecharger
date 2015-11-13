#ifndef MESSAGEQUEUENODE_H
#define MESSAGEQUEUENODE_H

#include <QString>

class MessageQueueNode
{
public:

	QString MessageContent;
	int MessageGroupID;
	int MessageAppID;

	MessageQueueNode();
	MessageQueueNode( const MessageQueueNode& node );
	MessageQueueNode& operator =( MessageQueueNode node );

};

#endif // MESSAGEQUEUENODE_H
