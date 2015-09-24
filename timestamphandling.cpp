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
	this->FirstInitialed = false;
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
	int CalibrateErrorCounter;

	QUrl url( CardRecharger::SelfInstance->CardRechargerServerURL + tr( "/clientapi/getSysTime" ) );
	qDebug() << url.toString();

	CalibrateErrorCounter = 0;
	while( true )
	{
		this->TimestampIsInitialized = false;
		this->TimestampClient.RequestGet( url, MessageHandling::RechargerMessages, MessageHandling::GetSysTime );

		TimeoutCounter = 0;

		while( this->TimestampIsInitialized == false )
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
				CardRecharger::SelfInstance->SetStatusLabel( tr( "Please reconnect the network!"));
#endif
			}
			continue;
		}


		qDebug() << tr( "calibrate successfully.");
		CalibrateErrorCounter = 0;
		this->FirstInitialed = true;

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


bool TimestampHandling::IsFirstInitialed()
{
	return this->FirstInitialed;
}

