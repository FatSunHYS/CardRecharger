#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling()
{
	this->EncrpytMD5 = new QCryptographicHash( QCryptographicHash::Md5 );
	this->RechargerClient = NULL;
}


RechargerHandling* RechargerHandling::GetInstance()
{
	if( RechargerHandling::PrivateInstance == NULL )
	{
		RechargerHandling::PrivateInstance = new RechargerHandling();
	}

	return RechargerHandling::PrivateInstance;
}
#if 0
void test()
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
	qDebug() << QObject::tr( "ready to log in the server." );
	CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
	secret = QString( this->EncrpytMD5->result().toHex() ) + CurrentTimestampString;
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( secret.toLower().toUtf8() );
	secret = QString( this->EncrpytMD5->result().toHex() );

	LoginParameters = ( QObject::tr( "identify=" ) + CardRecharger::SelfInstance->CardRechargerClientID );
	LoginParameters += ( QObject::tr( "&secret=" ) + secret );
	LoginParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );

	url.clear();
	url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/login?" ) + LoginParameters );
	qDebug() << "url = " << url.toString();

	//this->RechargerClient.RequestPost( url, LoginParameters, MessageHandling::RechargerMessages, MessageHandling::Login );
	this->RechargerClient.RequestGet( url, MessageHandling::RechargerMessages, MessageHandling::Login );



	while( true )
	{








	}


}

#endif
