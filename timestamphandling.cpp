#include <QDebug>

#include <time.h>
#include <sys/time.h>

#include "timestamphandling.h"
#include "cardrecharger.h"
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
	bool ButtonFirstEnable = false;

	QUrl url( tr( "http://112.74.202.84:8080/jeefw/clientapi/getSysTime" ) );
	qDebug() << url.toString();

	while( true )
	{
		this->TimestampIsInitialized = false;
		this->TimestampClient.RequestGet( url, MessageHandling::RechargerMessages, MessageHandling::GetSysTime );

		TimeoutCounter = 0;
		while( this->TimestampIsInitialized == false )
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

		if( ButtonFirstEnable == false )
		{
			ButtonFirstEnable = true;
			CardRecharger::SelfInstance->AllButtonEnable();
#ifdef CHINESE_OUTPUT
			CardRecharger::SelfInstance->SetStatusLabel( tr( "请点击充值金额"));
#else
			CardRecharger::SelfInstance->SetStatusLabel( tr( "Click the recharger button to recharge"));
#endif

			qDebug() << tr( "All Button is Enable." );

		}

		for( int i = 0; i < 86400; ++i )
		{
			sleep( 1 );
			this->unixtimestamp += 1000;
			qDebug() << "current time: " << QString::number( this->GetTimestamp(), '.', 0 );
		}

	}


}



void TimestampHandling::CalibrateTimestamp(double newtimestamp)
{
	this->unixtimestamp = newtimestamp;
	qDebug() << tr( "unix timestamp: " ) << QString::number( newtimestamp, '.', 0 );

	this->TimestampIsInitialized = true;
}

double TimestampHandling::GetTimestamp()
{
	return this->unixtimestamp;
}


