#include <QObject>

#include "messagequeuenode.h"

MessageQueueNode::MessageQueueNode()
{
	this->IsError = false;
	this->MessageContent = QObject::tr( "NULL" );
	this->MessageGroupID = -1;
	this->MessageAppID = -1;
}


MessageQueueNode& MessageQueueNode::operator =( MessageQueueNode node )
{
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;
	this->MessageGroupID = node.MessageGroupID;
	this->MessageAppID = node.MessageAppID;

	return *this;
}

MessageQueueNode::MessageQueueNode(const MessageQueueNode &node )
{
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;
	this->MessageGroupID = node.MessageGroupID;
	this->MessageAppID = node.MessageAppID;
}
