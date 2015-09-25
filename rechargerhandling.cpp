#include <QDebug>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling(QObject *parent) : QThread( parent )
{
	this->EncrpytMD5 = new QCryptographicHash( QCryptographicHash::Md5 );
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
	QString CurrentTimestampString;
	QString secret;
	QString LoginParameters;
	QUrl url;

	/* wait for timestamp initialized. */
	while( TimestampHandling::GetInstance()->IsFirstInitialed() == false )
	{

	}


	/* log in the server. */
	qDebug() << tr( "ready to log in the server." );
	CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
	secret = QString( this->EncrpytMD5->result().toHex() ) + CurrentTimestampString;
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( secret.toLower().toUtf8() );
	secret = QString( this->EncrpytMD5->result().toHex() );

	LoginParameters = ( tr( "identify=" ) + CardRecharger::SelfInstance->CardRechargerClientID );
	LoginParameters += ( tr( "&secret=" ) + secret );
	LoginParameters += ( tr( "&timestamp=" ) + CurrentTimestampString );

	url.clear();
	url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + tr( "/clientapi/login?" ) + LoginParameters );
	qDebug() << "url = " << url.toString();

	//this->RechargerClient.RequestPost( url, LoginParameters, MessageHandling::RechargerMessages, MessageHandling::Login );
	this->RechargerClient.RequestGet( url, MessageHandling::RechargerMessages, MessageHandling::Login );



	while( true )
	{








	}


}

