#include <QDebug>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
using namespace std;

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
			case MessageHandling::RechargerMessages:
			{
				this->ParsingRechargerMessages( TemperoryMessage );
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


}


void MessageHandling::ParsingRechargerMessages( MessageQueueNode* message )
{
	char timestampbuffer[ 1024 ];

	qDebug() << tr( "ParsingRechargerMessages:" ) << message->MessageRequestID;
	qDebug() << tr( "IsError:") << message->IsError;

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
			qDebug() << tr( "MessageAppID Error!" );
			return;
		}
	}



}




























