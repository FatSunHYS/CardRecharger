#include <QDebug>
#include <QObject>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "messagehandling.h"
#include "timestamphandling.h"
#include "cJSON.h"

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


	}

	return ( void* )0;

}


void MessageHandling::ParsingRechargerMessages( MessageQueueNode* message )
{
	char timestampbuffer[ 1024 ];

	qDebug() << QObject::tr( "IsError:") << message->IsError;

	if( message->IsError )
	{
		qDebug() << message->MessageContent << endl;
		return;
	}
	else
	{
		qDebug() << message->MessageContent << endl;
	}


	switch( message->MessageAppID )
	{
		case MessageHandling::GetSysTime:
		{
			strcpy( timestampbuffer, message->MessageContent.toUtf8().data() );
			cJSON* root = cJSON_Parse( timestampbuffer );
			double tempint = cJSON_GetObjectItem( root, "timestamp" )->valuedouble;
			cJSON_Delete( root );
			//qDebug() << tr( "tempint = " ) << tempint;

			TimestampHandling::GetInstance()->CalibrateTimestamp( tempint );

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


























