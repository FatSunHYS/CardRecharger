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
	TimestampHandling* Handler = TimestampHandling::GetInstance();
	CURLcode RequestResult;
    QString RespondContent;

	if( arg != NULL )
	{

	}

	qDebug() << QObject::tr( "TimestampHandler is running..." );

	QUrl url( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/getSysTime" ) );
    //qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

	while( true )
	{
		Handler->SetTimestampRefreshState( false );

		RequestResult = Handler->TimestampClient.RequestGet( url, RespondContent );
		if( RequestResult != CURLE_OK )
		{
            qDebug() << QObject::tr( "Request Time Error! - 1" );
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络断开，请检查网络！"));
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Please reconnect the network!"));
#endif
            sleep( 2 );
			continue;
		}

        Handler->ParseGetSysTimeMessage( RespondContent );

		if( Handler->GetTimestampRefreshState() == false )
		{
            qDebug() << QObject::tr( "Request Time Error! - 2" );
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络断开，请检查网络！"));
#else
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Please reconnect the network!"));
#endif
			sleep( 2 );
			continue;
		}

		qDebug() << QObject::tr( "calibrate successfully.");
		Handler->FirstInitialDone();

		Handler->ReCalibrateIsNeeded = false;
		for( int i = 0; i < 86400; ++i )
		{
			sleep( 1 );
			Handler->RefreshTimestamp();
			if( Handler->ReCalibrateIsNeeded == true )
			{
                qDebug() << QObject::tr( "ReCalibrate..." );
				break;
			}
            //qDebug() << QObject::tr( "current time: " ) << QString::number( Handler->GetTimestamp(), '.', 0 );
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











