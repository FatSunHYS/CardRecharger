#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QRegExp>

#include <unistd.h>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"
#include "cJSON.h"


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
	int TimeoutCounter;
	int RechargerErrorCounter;

	if( arg != NULL )
	{

	}

	qDebug() << QObject::tr( "RechargerHandler is runnint..." );


	while( true )
	{
		Handler->DeviceIsLogin = false;

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
		secret = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
		pthread_mutex_unlock( &Handler->MD5Locker );

		LoginParameters = ( QObject::tr( "identify=" ) + CardRecharger::SelfInstance->CardRechargerClientID );
		LoginParameters += ( QObject::tr( "&secret=" ) + secret );
		LoginParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );

		url.clear();
		url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/login?" ) + LoginParameters );
		qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

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

		TimeoutCounter = 0;

		while( Handler->DeviceIsLogin == false )
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
			++RechargerErrorCounter;
			if( RechargerErrorCounter >= 2 )
			{
				TimestampHandling::GetInstance()->ReCalibrateIsNeeded = true;
				qDebug() << QObject::tr( "Login Error!" );
			}
			sleep( 1 );
			continue;
		}

		RechargerErrorCounter = 0;

		/* Login successfully. */
		qDebug() << QObject::tr( "Login successfully." );

		/* Send Heart Package every 15min. */
		while( true )
		{
			Handler->IsKeepAlived = false;
			CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

			pthread_mutex_lock( &Handler->MD5Locker );
			Handler->EncrpytMD5->reset();
			Handler->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
			secret = QString( Handler->EncrpytMD5->result().toHex() );
			Handler->EncrpytMD5->reset();
			Handler->EncrpytMD5->addData( Handler->DeviceToken.toLower().toUtf8() );
			Handler->EncrpytMD5->addData( secret.toLower().toUtf8() );
			Handler->EncrpytMD5->addData( CurrentTimestampString.toLower().toUtf8() );
			secret = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
			pthread_mutex_unlock( &Handler->MD5Locker );

			LoginParameters = ( QObject::tr( "devId=" ) + QString::number( Handler->DeviceID, 10 ) );
			LoginParameters += ( QObject::tr( "&sign=" ) + secret );
			LoginParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );

			url.clear();
			url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/keepalived?" ) + LoginParameters );
			qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

			RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );

			TemperoryNode = new MessageQueueNode();
			TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
			TemperoryNode->MessageAppID = MessageHandling::KeepAlived;

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

			while( Handler->DeviceIsLogin == false )
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
				++RechargerErrorCounter;
				if( RechargerErrorCounter >= 2 )
				{
					TimestampHandling::GetInstance()->ReCalibrateIsNeeded = true;
					qDebug() << QObject::tr( "Send heart package fail!" );
					RechargerErrorCounter = 0;
					sleep( 1 );
					break;
				}
				sleep( 1 );
				continue;
			}

			qDebug() << QObject::tr( "Send Heart Package successfully." );
			RechargerErrorCounter = 0;

			sleep( 900 );
			//sleep( 30 );
		}

	}

	return ( void* )0;
}



void RechargerHandling::ParseLoginMessage(QString &Message)
{
	char rechargerloginbuffer[ 1024 ];
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

	if( ( Message_JSON == QString( "" ) ) || ( Message_MD5 == QString( "" ) ) )
	{
		return;
	}

	//qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	//qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;

	/* Checkout the MD5 */
	pthread_mutex_lock( &this->MD5Locker );
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
	QString passwordmd5 = QString( this->EncrpytMD5->result().toHex() ).toLower();
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( Message_JSON.toUtf8() );
	this->EncrpytMD5->addData( passwordmd5.toUtf8() );
	QString TestMD5Result = QString( this->EncrpytMD5->result().toHex() ).toLower();
	pthread_mutex_unlock( &this->MD5Locker );

	if( TestMD5Result == Message_MD5 )
	{
		//qDebug() << QObject::tr( "MD5 is correct." );
	}
	else
	{
		qDebug() << QObject::tr( "MD5 is error. MD5 = " ) << TestMD5Result;
		return;
	}

	strcpy( rechargerloginbuffer, Message_JSON.toUtf8().data() );
	cJSON* root = cJSON_Parse( rechargerloginbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode == 0 )
	{
		this->DeviceID = cJSON_GetObjectItem( root, "devId" )->valueint;
		this->DeviceToken = QString( cJSON_GetObjectItem( root, "token" )->valuestring );
	}
	else
	{
		cJSON_Delete( root );
		return;
	}

	cJSON_Delete( root );

	qDebug() << QObject::tr( "DeviceId = " ) << this->DeviceID;
	//qDebug() << QObject::tr( "DeviceToken = " ) << this->DeviceToken;

	this->DeviceIsLogin = true;

}



void RechargerHandling::ParseKeepAlivedMessage(QString &Message)
{
	char rechargerloginbuffer[ 1024 ];
	QRegExp RegularExpression;
	RegularExpression.setPattern( QObject::tr( "(\\{.*\\})([0-9a-z]{32})?"));

	RegularExpression.indexIn( Message );
	QStringList list = RegularExpression.capturedTexts();

	if( list.size() != 3 )
	{
		return;
	}

	QString Message_JSON = list.at( 1 );
	QString Message_MD5 = list.at( 2 );

#if 0
	if( ( Message_JSON == QString( "" ) ) || ( Message_MD5 == QString( "" ) ) )
	{
		return;
	}

	//qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	//qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;

	/* Checkout the MD5 */
	pthread_mutex_lock( &this->MD5Locker );
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
	QString passwordmd5 = QString( this->EncrpytMD5->result().toHex() ).toLower();
	this->EncrpytMD5->reset();
	this->EncrpytMD5->addData( Message_JSON.toUtf8() );
	this->EncrpytMD5->addData( passwordmd5.toUtf8() );
	QString TestMD5Result = QString( this->EncrpytMD5->result().toHex() ).toLower();
	pthread_mutex_unlock( &this->MD5Locker );

	if( TestMD5Result == Message_MD5 )
	{
		//qDebug() << QObject::tr( "MD5 is correct." );
	}
	else
	{
		qDebug() << QObject::tr( "MD5 is error. MD5 = " ) << TestMD5Result;
		return;
	}
#endif

	strcpy( rechargerloginbuffer, Message_JSON.toUtf8().data() );
	cJSON* root = cJSON_Parse( rechargerloginbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode == 0 )
	{

	}
	else
	{
		cJSON_Delete( root );
		return;
	}

	cJSON_Delete( root );

	//qDebug() << QObject::tr( "DeviceId = " ) << this->DeviceID;
	//qDebug() << QObject::tr( "DeviceToken = " ) << this->DeviceToken;

	this->IsKeepAlived = true;
}







