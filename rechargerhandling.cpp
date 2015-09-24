#include <QCryptographicHash>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling(QObject *parent) : QThread( parent )
{
}


RechargerHandling* RechargerHandling::GetInstance()
{
	if( RechargerHandling::PrivateInstance == NULL )
	{
		RechargerHandling::PrivateInstance = new RechargerHandling();
	}

	return RechargerHandling::PrivateInstance;
}

void RechargerHandling::run()
{



	/* wait for timestamp initialized. */
	while( TimestampHandling::GetInstance()->IsFirstInitialed() == false )
	{

	}


	/* log in the server. */
	qDebug() << tr( "ready to log in the server." );
	double CurrentTimestamp = TimestampHandling::GetInstance()->GetTimestamp();
	QString CurrentTimestampString = QString::number( CurrentTimestamp, '.', 0 );
	QString secret = CardRecharger::SelfInstance->CardRechargerClientPassword.toLower();


	while( true )
	{








	}


}

