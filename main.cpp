#include <QApplication>
#include <QTextCodec>

#ifdef EMBEDDED_BOARD
#include <QWSServer>
#endif

#include <stdlib.h>
#include <curl/curl.h>

#include "cardrecharger.h"
#include "errordialog.h"

#include "messagehandling.h"
#include "timestamphandling.h"
#include "rechargerhandling.h"
#include "inifile.h"

MessageHandling* DebugMessageHandlingInstance;
TimestampHandling* DebugTimestampHandlingInstance;

bool SystemInitialization();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef EMBEDDED_BOARD
    QWSServer::setCursorVisible( false );
#endif

	//changed.

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	curl_global_init( CURL_GLOBAL_ALL );

	CardRecharger w;
	CardRecharger::SelfInstance = &w;

	ErrorDialog e;

	if( SystemInitialization() )
	//if( 0 )
	{

		DebugMessageHandlingInstance = MessageHandling::GetInstance();
		DebugMessageHandlingInstance->CreatePThread();
		TimestampHandling::GetInstance()->CreatePThread();
		//RechargerHandling::GetInstance()->start();

		w.setWindowFlags( Qt::FramelessWindowHint );
		w.show();
	}
	else
	{
		e.setWindowFlags( Qt::FramelessWindowHint );
		e.show();
	}

	return a.exec();
}


bool SystemInitialization()
{
	char TemperoryBuffer[ 1024 ];

	//初始化各类参数
	if(!read_profile_string("ClientIdentifier", "ClientID", TemperoryBuffer, 1024, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : ClientIdentifier, Programme exit!\n");
		return false;
	}

	CardRecharger::SelfInstance->CardRechargerClientID = QString( TemperoryBuffer );

	if(!read_profile_string("ClientPassword", "ClientPW", TemperoryBuffer, 1024, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : ClientPassword, Programme exit!\n");
		return false;
	}

	CardRecharger::SelfInstance->CardRechargerClientPassword = QString( TemperoryBuffer );


	if(!read_profile_string("ServerBASEURL", "ServerBURL", TemperoryBuffer, 1024, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : ServerBASEURL, Programme exit!\n");
		return false;
	}

	CardRecharger::SelfInstance->CardRechargerServerURL = QString( TemperoryBuffer );

#if 0
	if(!read_profile_string("AdvertisementURL", "AdServerURL", AdvtServerURLStr, 100, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : AdvertisementURL, Programme exit!\n");
		return 0;
	}


	if(!read_profile_string("ICDevice", "Serials", ICReadDevStr, 100, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : ICDevice, Programme exit!\n");
		return 0;
	}

	if(!read_profile_string("Version", "Currentv", VersionStr, 100, "Error", "DevInfo.ini"))
	{
		qDebug("Read ini file failed : Version, Programme exit!\n");
		return 0;
	}
#endif

	return true;
}

