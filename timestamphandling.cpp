#include <QObject>
#include <QDebug>
#include <QUrl>
#include <QString>

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <iostream>
using namespace std;

#include "timestamphandling.h"
#include "cardrecharger.h"
#include "messagehandling.h"
#include "messagequeue.h"
#include "messagequeuenode.h"
#include "cJSON.h"


TimestampHandling* TimestampHandling::PrivateInstace = NULL;

TimestampHandling::TimestampHandling()
{
	this->TimestampIsRefreshed = false;
	this->unixtimestamp = 0;
	this->FirstInitialed = false;
	this->ReCalibrateIsNeeded = false;
}



TimestampHandling* TimestampHandling::GetInstance()
{
	if( TimestampHandling::PrivateInstace == NULL )
	{
		TimestampHandling::PrivateInstace = new TimestampHandling();
	}

	return TimestampHandling::PrivateInstace;
}


void* TimestampHandler( void* arg )
{
	int TimeoutCounter;
	int CalibrateErrorCounter;
	bool ButtonFirstEnable = false;
	TimestampHandling* Handler = TimestampHandling::GetInstance();
	CURLcode RequestResult;
	QString RespondContent;
	MessageQueueNode* TemperoryNode;

	if( arg != NULL )
	{

	}

	qDebug() << QObject::tr( "TimestampHandler is running..." );

	QUrl url( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/getSysTime" ) );
	qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

	CalibrateErrorCounter = 0;
	while( true )
	{
		Handler->SetTimestampRefreshState( false );

		RequestResult = Handler->TimestampClient.RequestGet( url, RespondContent );

		//qDebug() << QObject::tr( "request result = " ) << requestresult;
		//qDebug() << QObject::tr( "test buffer = " ) << QString::fromStdString( ReceiveContent );

		TemperoryNode = new MessageQueueNode();
		TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
		TemperoryNode->MessageAppID = MessageHandling::GetSysTime;

		if( RequestResult == CURLE_OK )
		{
			TemperoryNode->IsError = false;
			TemperoryNode->MessageContent = RespondContent;
		}
		else
		{
			TemperoryNode->IsError = true;
			TemperoryNode->MessageContent = QObject::tr( "NULL" );
		}

		MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
		TemperoryNode = NULL;

		TimeoutCounter = 0;

		while( Handler->GetTimestampRefreshState() == false )
		{
			sleep( 1 );
			++TimeoutCounter;
			if( TimeoutCounter >= 3 )
			{
				break;
			}
		}

		if( TimeoutCounter >= 3 )
		{
			TimeoutCounter = 0;
			++CalibrateErrorCounter;
			if( CalibrateErrorCounter >= 5 )
			{
#ifdef CHINESE_OUTPUT
				CardRecharger::SelfInstance->SetStatusLabel( tr( "网络断开，请检查网络！"));
#else
				CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Please reconnect the network!"));
#endif
			}
			continue;
		}

		qDebug() << QObject::tr( "calibrate successfully.");
		CalibrateErrorCounter = 0;
		Handler->FirstInitialDone();

		if( ButtonFirstEnable == false )
		{
			ButtonFirstEnable = true;
			CardRecharger::SelfInstance->AllButtonEnable();
#ifdef CHINESE_OUTPUT
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "请点击充值金额"));
#else
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Click the recharger button to recharge"));
#endif

			qDebug() << QObject::tr( "All Button is Enable." );

		}

		Handler->ReCalibrateIsNeeded = false;
		for( int i = 0; i < 86400; ++i )
		{
			sleep( 1 );
			Handler->RefreshTimestamp();
			if( Handler->ReCalibrateIsNeeded == true )
			{
				break;
			}
			qDebug() << QObject::tr( "current time: " ) << QString::number( Handler->GetTimestamp(), '.', 0 );
		}

	}

	return ( void* )0;
}



void TimestampHandling::CalibrateTimestamp(double newtimestamp)
{
	this->unixtimestamp = newtimestamp;
	//qDebug() << QObject::tr( "unix timestamp: " ) << QString::number( newtimestamp, '.', 0 );

	this->TimestampIsRefreshed = true;
}

double TimestampHandling::GetTimestamp()
{
	return this->unixtimestamp;
}


bool TimestampHandling::IsFirstInitialed()
{
	return this->FirstInitialed;
}




bool TimestampHandling::CreatePThread()
{
	int err = pthread_create( &( this->TimestampHandlingPthreadID ), NULL, TimestampHandler, NULL );

	if( err != 0 )
	{
		qDebug() << QObject::tr( "Create pthread TimestampHandler error!" );
		return false;
	}
	else
	{
		qDebug() << QObject::tr( "Create pthread TimestampHandler Successfully." );
		return true;
	}
}


void TimestampHandling::SetTimestampRefreshState(bool newstate)
{
	this->TimestampIsRefreshed = newstate;
}


bool TimestampHandling::GetTimestampRefreshState()
{
	return this->TimestampIsRefreshed;
}


void TimestampHandling::FirstInitialDone()
{
	this->FirstInitialed = true;
}

void TimestampHandling::RefreshTimestamp()
{
	this->unixtimestamp += 1000;
}


void TimestampHandling::ParseGetSysTimeMessage(QString &Message)
{
	char timestampbuffer[ 1024 ];
	strcpy( timestampbuffer, Message.toUtf8().data() );
	cJSON* root = cJSON_Parse( timestampbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode == 0 )
	{
		double TemperoryTimestamp = cJSON_GetObjectItem( root, "timestamp" )->valuedouble;
		this->CalibrateTimestamp( TemperoryTimestamp );
	}
	else
	{

	}

	cJSON_Delete( root );
}











