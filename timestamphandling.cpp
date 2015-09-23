#include "timestamphandling.h"

#include <QDebug>

#include <time.h>
#include <sys/time.h>

#include "messagehandling.h"

TimestampHandling* TimestampHandling::PrivateInstace = NULL;

TimestampHandling::TimestampHandling(QObject *parent) : QThread(parent)
{
	this->TimestampIsInitialized = false;
	this->unixtimestamp = 0;
}



TimestampHandling* TimestampHandling::GetInstance()
{
	if( TimestampHandling::PrivateInstace == NULL )
	{
		TimestampHandling::PrivateInstace = new TimestampHandling();
	}

	return TimestampHandling::PrivateInstace;
}


void TimestampHandling::run()
{
	int TimeoutCounter;

	QUrl url( tr( "http://112.74.202.84:8080/jeefw/clientapi/getSysTime" ) );
	qDebug() << url.toString();

	while( true )
	{
		this->TimestampIsInitialized = false;
		this->TimestampClient.RequestGet( url, MessageHandling::RechargerMessages, MessageHandling::GetSysTime );

		TimeoutCounter = 0;
		while( this->IsTimestampInitialized() == false )
		{
			sleep( 1 );
			++TimeoutCounter;
			if( TimeoutCounter == 3 )
			{
				break;
			}
		}

		if( TimeoutCounter >= 3 )
		{
			continue;
		}

		qDebug() << tr( "calibrate successfully.");


		for( int i = 0; i < 86400; ++i )
		{
			sleep( 1 );
			//this->TimestampChangeMutex.lock();
			this->unixtimestamp += 1000;
			//this->TimestampChangeMutex.unlock();
			qDebug() << "current time: " << QString::number( this->GetTimestamp(), '.', 0 );
		}

	}


}


/*
 * If the timestamp is initialized, return true.
 */
bool TimestampHandling::IsTimestampInitialized()
{
	//QMutexLocker TemperoryLocker( &this->TimestampInitialMutex );

	return this->TimestampIsInitialized;
}


void TimestampHandling::TimestampReset()
{
	//QMutexLocker TemperoryLocker( &this->TimestampInitialMutex );
	this->TimestampIsInitialized = false;
}



void TimestampHandling::CalibrateTimestamp(double newtimestamp)
{
	//QMutexLocker TemperoryLocker1( &this->TimestampInitialMutex );
	//QMutexLocker TemperoryLocker2( &this->TimestampChangeMutex );

	this->unixtimestamp = newtimestamp;
	qDebug() << tr( "unix timestamp: " ) << QString::number( newtimestamp, '.', 0 );

	this->TimestampIsInitialized = true;
}

double TimestampHandling::GetTimestamp()
{
	//QMutexLocker TemperoryLocker( &this->TimestampChangeMutex );

	return this->unixtimestamp;
}


