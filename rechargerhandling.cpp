#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QRegExp>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling()
{
	this->EncrpytMD5 = new QCryptographicHash( QCryptographicHash::Md5 );
	pthread_mutex_init( &this->MD5Locker, NULL );
}


RechargerHandling* RechargerHandling::GetInstance()
{
	if( RechargerHandling::PrivateInstance == NULL )
	{
		RechargerHandling::PrivateInstance = new RechargerHandling();
	}

	return RechargerHandling::PrivateInstance;
}


bool RechargerHandling::CreatePThread()
{
	int err = pthread_create( &( this->RechargerHandlingPthreadID ), NULL, RechargerHandler, NULL );

	if( err != 0 )
	{
		qDebug() << QObject::tr( "Create pthread RechargerHandler error!" );
		return false;
	}
	else
	{
		qDebug() << QObject::tr( "Create pthread RechargerHandler Successfully." );
		return true;
	}
}

void* RechargerHandler( void* arg )
{
	QString CurrentTimestampString;
	QString secret;
	QString LoginParameters;
	QUrl url;
	CURLcode RequestResult;
	QString RespondContent;
	RechargerHandling* Handler = RechargerHandling::GetInstance();
	MessageQueueNode* TemperoryNode;

	if( arg != NULL )
	{

	}

	qDebug() << QObject::tr( "RechargerHandler is runnint..." );

	/* wait for timestamp initialized. */
	while( TimestampHandling::GetInstance()->IsFirstInitialed() == false )
	{

	}

	/* log in the server. */
	qDebug() << QObject::tr( "ready to log in the server." );
	CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

	pthread_mutex_lock( &Handler->MD5Locker );
	Handler->EncrpytMD5->reset();
	Handler->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
	secret = QString( Handler->EncrpytMD5->result().toHex() ) + CurrentTimestampString;
	Handler->EncrpytMD5->reset();
	Handler->EncrpytMD5->addData( secret.toLower().toUtf8() );
	secret = QString( Handler->EncrpytMD5->result().toHex() );
	pthread_mutex_unlock( &Handler->MD5Locker );

	LoginParameters = ( QObject::tr( "identify=" ) + CardRecharger::SelfInstance->CardRechargerClientID );
	LoginParameters += ( QObject::tr( "&secret=" ) + secret );
	LoginParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );

	url.clear();
	url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/login?" ) + LoginParameters );
	qDebug() << "url = " << url.toString();

	RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );

	TemperoryNode = new MessageQueueNode();
	TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
	TemperoryNode->MessageAppID = MessageHandling::Login;

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




	while( true )
	{

	}

	return ( void* )0;
}



void RechargerHandling::ParseLoginMessage(QString &Message)
{
	QRegExp RegularExpression;
	RegularExpression.setPattern( QObject::tr( "(\\{.*\\})([0-9a-z]{32})"));

	RegularExpression.indexIn( Message );
	QStringList list = RegularExpression.capturedTexts();

	if( list.size() != 3 )
	{
		return;
	}

	QString Message_JSON = list.at( 1 );
	QString Message_MD5 = list.at( 2 );

	qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;




}











