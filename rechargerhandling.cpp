#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QRegExp>
#include <QMessageBox>
#include <QImage>
#include <QColor>

#include <unistd.h>

#include "rechargerhandling.h"
#include "timestamphandling.h"
#include "messagehandling.h"
#include "cardrecharger.h"
#include "cJSON.h"
#include "iccarddriver.h"


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling()
{
	this->EncrpytMD5 = new QCryptographicHash( QCryptographicHash::Md5 );
	this->PayWay = RechargerHandling::AliPay;
	pthread_mutex_init( &this->MD5Locker, NULL );
	pthread_mutex_init( &this->RechargerLoginLocker, NULL );
	pthread_cond_init( &this->LoginCondition, NULL );
	pthread_cond_init( &this->HeartPackageCondition, NULL );
	pthread_mutex_init( &this->RechargerChargeLocker, NULL );
	pthread_cond_init( &this->ChargeAction, NULL );
	pthread_cond_init( &this->RequestQRCode, NULL );
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
	int err = pthread_create( &( this->RechargerHandling1PthreadID ), NULL, RechargerLoginHandler, NULL );

	if( err != 0 )
	{
		qDebug() << QObject::tr( "Create pthread RechargerLoginHandler error!" );
		return false;
	}
	else
	{
		qDebug() << QObject::tr( "Create pthread RechargerLoginHandler Successfully." );
	}

#if 1
	err = pthread_create( &( this->RechargerHandling2PthreadID ), NULL, RechargerChargeHandler, NULL );

	if( err != 0 )
	{
		qDebug() << QObject::tr( "Create pthread RechargerChargeHandler error!" );
		return false;
	}
	else
	{
		qDebug() << QObject::tr( "Create pthread RechargerChargeHandler Successfully." );
	}
#endif
	return true;
}

void* RechargerLoginHandler( void* arg )
{
	QString CurrentTimestampString;
	QString secret;
	QString LoginParameters;
	QUrl url;
	CURLcode RequestResult;
	QString RespondContent;
	RechargerHandling* Handler = RechargerHandling::GetInstance();
	MessageQueueNode* TemperoryNode;
	int RechargerErrorCounter;

	if( arg != NULL )
	{

	}

	pthread_mutex_lock( &Handler->RechargerLoginLocker );
	qDebug() << QObject::tr( "RechargerHandler is running..." );

	while( true )
	{
		Handler->DeviceIsLogin = false;

		/* wait for timestamp initialized. */
		while( TimestampHandling::GetInstance()->IsFirstInitialed() == false )
		{

		}

#ifdef CHINESE_OUTPUT
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "设备正在登录中"));
#else
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Device is being login to the server."));
#endif

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
		if( RequestResult != CURLE_OK )
		{
			qDebug() << QObject::tr( "Login Request error!" );
			continue;
		}

		TemperoryNode = new MessageQueueNode();
		TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
		TemperoryNode->MessageAppID = MessageHandling::Login;
		TemperoryNode->MessageContent = RespondContent;

		MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
		TemperoryNode = NULL;

		pthread_cond_wait( &Handler->LoginCondition, &Handler->RechargerLoginLocker );

		if( Handler->DeviceIsLogin == false )
		{
			sleep( 1 );
			qDebug() << QObject::tr( "Login Request error!" );
			continue;
		}

		/* Login successfully. */
		qDebug() << QObject::tr( "Login successfully." );

		RechargerErrorCounter = 0;
		/* Send Heart Package every 15min. */
		while( true )
		{
			Handler->IsKeepAlived = false;
			CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

			LoginParameters = ( QObject::tr( "devId=" ) + QString::number( Handler->DeviceID, 10 ) );
			LoginParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );
			LoginParameters += ( QObject::tr( "&token=" ) + Handler->DeviceToken );

			pthread_mutex_lock( &Handler->MD5Locker );
			Handler->EncrpytMD5->reset();
			Handler->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
			secret = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
			Handler->EncrpytMD5->reset();
			Handler->EncrpytMD5->addData( LoginParameters.toUtf8() );
			Handler->EncrpytMD5->addData( secret.toUtf8() );
			Handler->EncrpytMD5->addData( CurrentTimestampString.toUtf8() );
			secret = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
			pthread_mutex_unlock( &Handler->MD5Locker );

			url.clear();
			url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/keepalived?" ) + LoginParameters + QObject::tr( "&sign=" ) + secret );
			qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

			RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );

			if( RequestResult != CURLE_OK )
			{
				qDebug() << QObject::tr( "Send heart package fail.Retry." );
				++RechargerErrorCounter;
				if( RechargerErrorCounter >= 3 )
				{
					qDebug() << QObject::tr( "Send heart package fail.Relogin." );
					break;
				}
				sleep( 2 );
				continue;
			}

			TemperoryNode = new MessageQueueNode();
			TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
			TemperoryNode->MessageAppID = MessageHandling::KeepAlived;
			TemperoryNode->MessageContent = RespondContent;
			MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
			TemperoryNode = NULL;

			pthread_cond_wait( &Handler->HeartPackageCondition, &Handler->RechargerLoginLocker );

			if( Handler->IsKeepAlived == false )
			{
				qDebug() << QObject::tr( "Send heart package fail.Retry." );
				++RechargerErrorCounter;
				if( RechargerErrorCounter >= 3 )
				{
					qDebug() << QObject::tr( "Send heart package fail.Relogin." );
					break;
				}
				sleep( 2 );
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

	if( Message_JSON == QString( "" ) )
	{
		return;
	}

	//qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	//qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;

	/* Parse the JSON. */
	strcpy( rechargerloginbuffer, Message_JSON.toUtf8().data() );
	cJSON* root = cJSON_Parse( rechargerloginbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode != 0 )
	{
		cJSON_Delete( root );
		return;
	}

	int TemperoryDeviceID = cJSON_GetObjectItem( root, "devId" )->valueint;
	QString TemperoryDeviceToken = QString( cJSON_GetObjectItem( root, "token" )->valuestring );
	QString SecNum = QString( cJSON_GetObjectItem( root, "SECNUM" )->valuestring );
	QString CardMM = QString( cJSON_GetObjectItem( root, "CARDMM" )->valuestring );
	QString MMEdition = QString( cJSON_GetObjectItem( root, "MMEdtion" )->valuestring );

	cJSON_Delete( root );


	/* Checkout the MD5 */
	if( Message_MD5 == QString( "" ) )
	{
		return;
	}

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


	this->DeviceID = TemperoryDeviceID;
	this->DeviceToken = TemperoryDeviceToken;
	this->SectionNumber = SecNum;
	this->CardPassword = CardMM;
	this->PasswordEdition = MMEdition;
	this->DeviceIsLogin = true;
	pthread_cond_signal( &this->LoginCondition );
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

	if( Message_JSON == QString( "" ) )
	{
		return;
	}

	//qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	//qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;

	/* Parse the JSON. */
	strcpy( rechargerloginbuffer, Message_JSON.toUtf8().data() );
	cJSON* root = cJSON_Parse( rechargerloginbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode != 0 )
	{
		cJSON_Delete( root );
		return;
	}

	cJSON_Delete( root );


	/* Checkout the MD5 */
	if( Message_MD5 == QString( "" ) )
	{
		return;
	}

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

	this->IsKeepAlived = true;
	pthread_cond_signal( &this->HeartPackageCondition );
}



void* RechargerChargeHandler(void *arg)
{
	RechargerHandling* Handler = RechargerHandling::GetInstance();
	QString RequestParameters;
	QString CurrentTimestampString;
	QString SignVerify;
	QUrl url;
	CURLcode RequestResult;
	QString RespondContent;
	MessageQueueNode* TemperoryNode;
	QRcode* qr;
	QImage* qrimage;
	QPainter qrpainter;
	QColor qrbackground(Qt::white);

	if( arg != NULL )
	{

	}

	pthread_mutex_lock( &Handler->RechargerChargeLocker );

	/* Wait for login. */
	while( Handler->DeviceIsLogin == false )
	{

	}

	CardRecharger::SelfInstance->AllButtonEnable();
	qDebug() << QObject::tr( "All Button is Enable." );

	while( true )
	{
#ifdef CHINESE_OUTPUT
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "请点击充值金额"));
#else
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Click the recharger button to recharge"));
#endif

		pthread_cond_wait( &Handler->ChargeAction, &Handler->RechargerChargeLocker );

		while( Handler->DeviceIsLogin == false )
		{
			qDebug() << QObject::tr( "Device is not login!" );

#ifdef CHINESE_OUTPUT
			QMessageBox::information( 0, QObject::tr( "错误！" ), QObject::tr( "网络断开，请通知维护人员！" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络断开，请通知维护人员！" ) );
#else
			QMessageBox::information( 0, QObject::tr( "Error!" ), QObject::tr( "Network is unavilable!" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable!" ) );
#endif
			sleep( 1 );
			CardRecharger::SelfInstance->AllButtonEnable();
			continue;
		}

		/* Testing code 'if' */
		if( Handler->PayWay == RechargerHandling::WeiXinPay )
		{
#ifdef CHINESE_OUTPUT
			QMessageBox::information( 0, QObject::tr( "错误！" ), QObject::tr( "目前暂不支持微信支付!" ) );
#else
			QMessageBox::information( 0, QObject::tr( "Error!" ), QObject::tr( "WeiXinPay is unavilable!" ) );
#endif
			sleep( 1 );
			CardRecharger::SelfInstance->AllButtonEnable();
			continue;
		}



		/* Read card. */

		Handler->SectionNumber = QString( "9" );
		Handler->CardPassword = QString( "ffffffffffff" );
		Handler->PasswordEdition = QString( "3" );
		Handler->CardNumber = QString( "10001" );
		Handler->CardSequenceNumber = QString( "10001" );

		/* Precreate. */
		CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

		RequestParameters = QObject::tr( "amount=" ) + QString::number( Handler->RechargeValue );
		RequestParameters += QObject::tr( "&cardNum=" ) + Handler->CardNumber;
		RequestParameters += ( QObject::tr( "&devId=" ) + QString::number( Handler->DeviceID, 10 ) );
		RequestParameters += QObject::tr( "&devIdentify=" ) + CardRecharger::SelfInstance->CardRechargerClientID;
		if( Handler->PayWay == RechargerHandling::AliPay )
		{
			RequestParameters += QObject::tr( "&payWay=alipay" );
		}
		else
		{
			RequestParameters += QObject::tr( "&payWay=weixin" );
		}
		RequestParameters += QObject::tr( "&sequenceNum=" ) + Handler->CardSequenceNumber;
		RequestParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );
		RequestParameters += ( QObject::tr( "&token=" ) + Handler->DeviceToken );

		pthread_mutex_lock( &Handler->MD5Locker );
		Handler->EncrpytMD5->reset();
		Handler->EncrpytMD5->addData( CardRecharger::SelfInstance->CardRechargerClientPassword.toLower().toUtf8() );
		SignVerify = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
		Handler->EncrpytMD5->reset();
		Handler->EncrpytMD5->addData( RequestParameters.toUtf8() );
		Handler->EncrpytMD5->addData( SignVerify.toUtf8() );
		Handler->EncrpytMD5->addData( CurrentTimestampString.toUtf8() );
		SignVerify = QString( Handler->EncrpytMD5->result().toHex() ).toLower();
		pthread_mutex_unlock( &Handler->MD5Locker );

		RequestParameters += QObject::tr( "&sign=" ) + SignVerify;

		url.clear();
		url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/trade/precreate?" ) + RequestParameters );
		qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

#ifdef CHINESE_OUTPUT
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "正在申请支付二维码" ) );
#else
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Requesting QRCode." ) );
#endif

		RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );
		if( RequestResult != CURLE_OK )
		{
#ifdef CHINESE_OUTPUT
			QMessageBox::information( 0, QObject::tr( "错误！" ), QObject::tr( "网络错误，请通知维护人员检查网络！" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络错误，请通知维护人员检查网络！" ) );
#else
			QMessageBox::information( 0, QObject::tr( "Error!" ), QObject::tr( "Network error in precreate!" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable!" ) );
#endif
			sleep( 1 );
			CardRecharger::SelfInstance->AllButtonEnable();
			continue;
		}

		TemperoryNode = new MessageQueueNode();
		TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
		TemperoryNode->MessageAppID = MessageHandling::Precreate;
		TemperoryNode->MessageContent = RespondContent;

		MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
		TemperoryNode = NULL;

		pthread_cond_wait( &Handler->RequestQRCode, &Handler->RechargerChargeLocker );
		if( Handler->TradeStatus != QString( "PRECREATE_SUCCESS" ) )
		{
#ifdef CHINESE_OUTPUT
			QMessageBox::information( 0, QObject::tr( "错误！" ), QObject::tr( "申请支付二维码失败！" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "申请支付二维码失败！" ) );
#else
			QMessageBox::information( 0, QObject::tr( "Error!" ), QObject::tr( "Request QRCode Failed!" ) );
			CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Request QRCode Failed" ) );
#endif
			sleep( 1 );
			CardRecharger::SelfInstance->AllButtonEnable();
			continue;
		}

#ifdef CHINESE_OUTPUT
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "请扫描二维码！" ) );
#else
		CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Please Scan the QRCode!" ) );
#endif

		//show the qrcode.
		qr = QRcode_encodeString( Handler->QRCodeAddress.toUtf8().data(), 1, QR_ECLEVEL_L, QR_MODE_8, 1 );
		if( qr != NULL )
		{
			qrimage = new QImage( 230, 230, QImage::Format_Mono );
			qrpainter.begin( qrimage );
			qrpainter.setBrush( qrbackground );
			qrpainter.setPen( Qt::NoPen );
			qrpainter.drawRect( 0, 0, 230, 230 );
			Handler->DrawQRcodeImage( qr, qrpainter, 230, 230 );
			qrpainter.end();
			//qrimage->save( QObject::tr( "./test.jpg" ) );
		}
		else
		{
			qDebug() << QObject::tr( "Draw qr code failed!" );
		}

		//CardRecharger::SelfInstance->SetQRLabel( *qrimage );
		CardRecharger::SelfInstance->SetQRView( qrimage );
		qrimage = NULL;

		sleep( 5 );
		CardRecharger::SelfInstance->ResetQRView();

		/* Query. */


		/* PreRechargeCheck. */


		/* Write card. */


		/* RechargeFinish. */


		CardRecharger::SelfInstance->AllButtonEnable();
	}

	return ( void* )0;
}



void RechargerHandling::ParsePrecreateMessage(QString &Message)
{
	char rechargerbuffer[ 1024 ];
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

	if( Message_JSON == QString( "" ) )
	{
		return;
	}

	//qDebug() << QObject::tr( "JSON = " ) << Message_JSON;
	//qDebug() << QObject::tr( "MD5 = " ) << Message_MD5;

	/* Parse the JSON. */
	strcpy( rechargerbuffer, Message_JSON.toUtf8().data() );
	cJSON* root = cJSON_Parse( rechargerbuffer );

	int TemperoryCode = cJSON_GetObjectItem( root, "code" )->valueint;
	if( TemperoryCode != 0 )
	{
		cJSON_Delete( root );
		return;
	}

	QString qrcode = cJSON_GetObjectItem( root, "qrcode" )->valuestring;
	QString out_trade_no = cJSON_GetObjectItem( root, "out_trade_no" )->valuestring;
	QString trade_status = cJSON_GetObjectItem( root, "trade_status" )->valuestring;

	cJSON_Delete( root );


	/* Checkout the MD5 */
	if( Message_MD5 == QString( "" ) )
	{
		return;
	}

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

	this->QRCodeAddress = qrcode;
	this->TradeNumber = out_trade_no;
	this->TradeStatus = trade_status;

	//qDebug() << QObject::tr( "qrcode = " ) << this->QRCodeAddress;
	//qDebug() << QObject::tr( "trade number = " ) << this->TradeNumber;
	//qDebug() << QObject::tr( "trade status = " ) << this->TradeStatus;
	pthread_cond_signal( &this->RequestQRCode );

}


void RechargerHandling::DrawQRcodeImage( QRcode* qr, QPainter &painter, int width, int height)
{
	QColor foreground(Qt::black);
	painter.setBrush(foreground);
	const int qr_width = qr->width > 0 ? qr->width : 1;
	double scale_x = width / qr_width;
	double scale_y = height / qr_width;
	for( int y = 0; y < qr_width; y ++)
	{
		for(int x = 0; x < qr_width; x++)
		{
			unsigned char b = qr->data[y * qr_width + x];
			if(b & 0x01)
			{
				QRectF r(x * scale_x + 15, y * scale_y + 15, scale_x, scale_y);
				painter.drawRects(&r, 1);
			}
		}
	}
}


