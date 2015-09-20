#include "messagequeuenode.h"

MessageQueueNode::MessageQueueNode(QObject *parent) : QObject(parent)
{
	this->MessageRequestID = -1;
	this->IsError = false;
	this->MessageContent = tr( "NULL" );
}


MessageQueueNode& MessageQueueNode::operator =( MessageQueueNode node )
{
	this->MessageRequestID = node.MessageRequestID;
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;

	return *this;
}

MessageQueueNode::MessageQueueNode(const MessageQueueNode &node )
{
	this->MessageRequestID = node.MessageRequestID;
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;
}
