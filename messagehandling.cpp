#include <QDebug>

#include "messagehandling.h"


MessageHandling* MessageHandling::PrivateInstance = NULL;


MessageHandling* MessageHandling::GetInstance()
{
	if( MessageHandling::PrivateInstance == NULL )
	{
		MessageHandling::PrivateInstance = new MessageHandling();
	}

	return MessageHandling::PrivateInstance;
}



MessageHandling::MessageHandling(QObject *parent) : QThread(parent)
{
	this->MessageQueuePointer = new MessageQueue();
}


void MessageHandling::run()
{
	MessageQueueNode* TemperoryMessage = NULL;

	while( true )
	{
		if( this->MessageQueuePointer->QueueIsEmpty() )
		{
			continue;
		}

		TemperoryMessage = this->MessageQueuePointer->MessageDequeue();

		switch( TemperoryMessage->MessageGroupID )
		{
			case MessageQueueNode::RechargerMessages:
			{
				this->ParsingRechargerMessages( TemperoryMessage );
				break;
			}

			case MessageQueueNode::AdvertisementMessages:
			{
				break;
			}

			default:
			{
				break;
			}
		}


	}


}


void MessageHandling::ParsingRechargerMessages( MessageQueueNode* message )
{
	qDebug() << tr( "ParsingRechargerMessages:" ) << message->MessageRequestID;
	qDebug() << tr( "IsError:") << message->IsError;

	if( message->IsError )
	{
		qDebug() << tr( "error") << endl;
	}
	else
	{
		qDebug() << message->MessageContent << endl;
	}
}
