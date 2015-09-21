#include "messagequeuenode.h"




MessageQueueNode::MessageQueueNode(QObject *parent) : QObject(parent)
{
	this->MessageRequestID = -1;
	this->IsError = false;
	this->MessageContent = tr( "NULL" );
	this->MessageGroupID = -1;
}


MessageQueueNode& MessageQueueNode::operator =( MessageQueueNode node )
{
	this->MessageRequestID = node.MessageRequestID;
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;
	this->MessageGroupID = node.MessageGroupID;

	return *this;
}

MessageQueueNode::MessageQueueNode(const MessageQueueNode &node ) : QObject( 0 )
{
	this->MessageRequestID = node.MessageRequestID;
	this->IsError = node.IsError;
	this->MessageContent = node.MessageContent;
	this->MessageGroupID = node.MessageGroupID;
}
