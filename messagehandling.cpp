#include <QDebug>
#include <QObject>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "messagehandling.h"
#include "timestamphandling.h"
#include "rechargerhandling.h"


MessageHandling* MessageHandling::PrivateInstance = NULL;


MessageHandling* MessageHandling::GetInstance()
{
	if( MessageHandling::PrivateInstance == NULL )
	{
		MessageHandling::PrivateInstance = new MessageHandling();
	}

	return MessageHandling::PrivateInstance;
}



MessageHandling::MessageHandling()
{
	this->MessageQueuePointer = new MessageQueue();
}


void* MessageHandler( void* arg )
{
	MessageQueueNode* TemperoryMessage = NULL;
	MessageHandling* Handler = MessageHandling::GetInstance();

	qDebug() << QObject::tr( "MessageHandler is running..." );

	if( arg != NULL )
	{

	}

	while( true )
	{
		if( Handler->MessageQueuePointer->QueueIsEmpty() )
		{
			continue;
		}

		TemperoryMessage = Handler->MessageQueuePointer->MessageDequeue();

		switch( TemperoryMessage->MessageGroupID )
		{
			case MessageHandling::RechargerMessages:
			{
				Handler->ParsingRechargerMessages( TemperoryMessage );
				break;
			}

			case MessageHandling::AdvertisementMessages:
			{
				break;
			}

			default:
			{
				break;
			}
		}

		delete TemperoryMessage;
		TemperoryMessage = NULL;

	}

	return ( void* )0;

}


void MessageHandling::ParsingRechargerMessages( MessageQueueNode* message )
{
	//qDebug() << QObject::tr( "IsError:") << message->IsError;

	qDebug() << message->MessageContent;

	switch( message->MessageAppID )
	{
		case MessageHandling::GetSysTime:
		{
			TimestampHandling::GetInstance()->ParseGetSysTimeMessage( message->MessageContent );
			break;
		}

		case MessageHandling::Login:
		{
			RechargerHandling::GetInstance()->ParseLoginMessage( message->MessageContent );
			break;
		}

		case MessageHandling::KeepAlived:
		{
			RechargerHandling::GetInstance()->ParseKeepAlivedMessage( message->MessageContent );
			break;
		}

		case MessageHandling::Precreate:
		{
			RechargerHandling::GetInstance()->ParsePrecreateMessage( message->MessageContent );
			break;
		}

		case MessageHandling::Query:
		{
			RechargerHandling::GetInstance()->ParseQueryMessage( message->MessageContent );
			break;
		}

		case MessageHandling::PreRechargeCheck:
		{
			RechargerHandling::GetInstance()->ParsePreRechargeCheckMessage( message->MessageContent );
			break;
		}

		case MessageHandling::RechargeFinish:
		{
			RechargerHandling::GetInstance()->ParseRechargeFinishMessage( message->MessageContent );
			break;
		}

		default:
		{
			qDebug() << QObject::tr( "MessageAppID Error!" );
			return;
		}
	}



}

bool MessageHandling::CreatePThread()
{
	int err = pthread_create( &( this->MessageHandlingPthreadID ), NULL, MessageHandler, NULL );

	if( err != 0 )
	{
		qDebug() << QObject::tr( "Create pthread MessageHandling error!" );
		return false;
	}
	else
	{
		qDebug() << QObject::tr( "Create pthread MessageHandling Successfully." );
		return true;
	}
}


























