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
    pthread_cond_init( &this->ChargeActionCondition, NULL );
    pthread_cond_init( &this->RequestQRCodeCondition, NULL );
    pthread_cond_init( &this->QueryResultCondition, NULL );
    pthread_cond_init( &this->PreRechargeCheckCondition, NULL );
    pthread_cond_init(&this->RechargeFinishCondition, NULL );
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
    unsigned char CardMM[ 12 ];

    RegularExpression.indexIn( Message );
    QStringList list = RegularExpression.capturedTexts();

    if( list.size() != 3 )
    {
        pthread_cond_signal( &this->LoginCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->LoginCondition );
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
        pthread_cond_signal( &this->LoginCondition );
        return;
    }

    int TemperoryDeviceID = cJSON_GetObjectItem( root, "devId" )->valueint;
    QString TemperoryDeviceToken = QString( cJSON_GetObjectItem( root, "token" )->valuestring );
    QString SecNum = QString( cJSON_GetObjectItem( root, "SECNUM" )->valuestring );
    memset( CardMM, 0, sizeof( CardMM ) );
    memcpy( CardMM, cJSON_GetObjectItem( root, "CARDMM" )->valuestring, 12 );
    QString MMEdition = QString( cJSON_GetObjectItem( root, "MMEdtion" )->valuestring );

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->LoginCondition );
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
        pthread_cond_signal( &this->LoginCondition );
        return;
    }


    this->DeviceID = TemperoryDeviceID;
    this->DeviceToken = TemperoryDeviceToken;
    this->SectionNumber = SecNum;
    this->PasswordEdition = MMEdition;
    memset( this->CardPassword, 0, sizeof( this->CardPassword ) );

    for( int i = 0; i < 6; ++i )
    {
        if( ( CardMM[ i * 2 ] >= 'a' ) && ( CardMM[ i * 2 ] <= 'f' ) )
        {
            this->CardPassword[ i ] = CardMM[ i * 2 ] - 'a' + 0x0A;
        }
        else if( ( CardMM[ i * 2 ] >= 'A' ) && ( CardMM[ i * 2 ] <= 'F' ) )
        {
            this->CardPassword[ i ] = CardMM[ i * 2 ] - 'A' + 0x0A;
        }
        else if( ( CardMM[ i * 2 ] >= '0' ) && ( CardMM[ i * 2 ] <= '9' ) )
        {
            this->CardPassword[ i ] = CardMM[ i * 2 ] - '0';
        }
        else
        {
            qDebug() << QObject::tr( "Parse Card Password failed!" );
            pthread_cond_signal( &this->LoginCondition );
            return;
        }

        this->CardPassword[ i ] <<= 4;

        if( ( CardMM[ i * 2 + 1 ] >= 'a' ) && ( CardMM[ i * 2 + 1 ] <= 'f' ) )
        {
            this->CardPassword[ i ] += CardMM[ i * 2 + 1 ] - 'a' + 0x0A;
        }
        else if( ( CardMM[ i * 2 + 1 ] >= 'A' ) && ( CardMM[ i * 2 + 1 ] <= 'F' ) )
        {
            this->CardPassword[ i ] += CardMM[ i * 2 + 1 ] - 'A' + 0x0A;
        }
        else if( ( CardMM[ i * 2 + 1 ] >= '0' ) && ( CardMM[ i * 2 + 1 ] <= '9' ) )
        {
            this->CardPassword[ i ] += CardMM[ i * 2 + 1 ] - '0';
        }
        else
        {
            qDebug() << QObject::tr( "Parse Card Password failed!" );
            pthread_cond_signal( &this->LoginCondition );
            return;
        }
    }

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
        pthread_cond_signal( &this->HeartPackageCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->HeartPackageCondition );
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
        pthread_cond_signal( &this->HeartPackageCondition );
        return;
    }

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->HeartPackageCondition );
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
        pthread_cond_signal( &this->HeartPackageCondition );
        return;
    }

    this->IsKeepAlived = true;
    pthread_cond_signal( &this->HeartPackageCondition );
}



void* RechargerChargeHandler(void *arg)
{
    RechargerHandling* Handler = RechargerHandling::GetInstance();
    ICCardDriver* ICCardHandler = ICCardDriver::GetICCardDirverInstance();
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
    bool QueryResult;
    int RemainingTime;
    int CardStatus;
    unsigned char CardNumberBuffer[ 8 ];
    unsigned char CardBalanceBuffer[ 8 ];
    unsigned char CardCompanyPasswordBuffer[ 8 ];
    unsigned char CardSynumBuffer[ 8 ];		//Unknown meanings variable
    unsigned char CardDayTimeBuffer[ 8 ];
    unsigned char CardTypeBuffer[ 8 ];
    unsigned char CardCzmmBuffer[ 8 ];		//Unknown meanings variable
    int TemperoryIntegerNumber;
    double TemperoryDoubleNumber;

    if( arg != NULL )
    {

    }

    pthread_mutex_lock( &Handler->RechargerChargeLocker );

    /* Wait for login. */
    while( Handler->DeviceIsLogin == false )
    {

    }

    while( true )
    {
        CardRecharger::SelfInstance->ResetQRView();
        CardRecharger::SelfInstance->AllButtonEnable();

#ifdef CHINESE_OUTPUT
        CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "请点击充值金额"));
#else
        CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Click the recharger button to recharge"));
#endif

        pthread_cond_wait( &Handler->ChargeActionCondition, &Handler->RechargerChargeLocker );

        while( Handler->DeviceIsLogin == false )
        {
            qDebug() << QObject::tr( "Device is not login!" );

#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络断开，请通知维护人员！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable!" ) );
#endif
            sleep( 5 );
            continue;
        }

        /* Testing code 'if' */
        if( Handler->PayWay == RechargerHandling::WeiXinPay )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "目前暂不支持微信支付!" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "WeiXinPay is unavilable!" ) );
#endif
            sleep( 5 );
            continue;
        }



        /* Read card. */

        memset( CardNumberBuffer, 0, sizeof( CardNumberBuffer ) );
        memset( CardBalanceBuffer, 0, sizeof( CardBalanceBuffer ) );
        memset( CardCompanyPasswordBuffer, 0, sizeof( CardCompanyPasswordBuffer ) );
        memset( CardSynumBuffer, 0, sizeof( CardSynumBuffer ) );
        memset( CardDayTimeBuffer, 0, sizeof( CardDayTimeBuffer ) );
        memset( CardTypeBuffer, 0, sizeof( CardTypeBuffer ) );
        memset( CardCzmmBuffer, 0, sizeof( CardCzmmBuffer ) );
        CardStatus = ICCardHandler->readwatercard_arm( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                                       ( unsigned char )( *Handler->PasswordEdition.toUtf8().data() ),
                                                       ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                                       Handler->CardPassword,
                                                       CardNumberBuffer,
                                                       CardBalanceBuffer,
                                                       CardCompanyPasswordBuffer,
                                                       CardDayTimeBuffer,
                                                       CardTypeBuffer,
                                                       100
                                                       );

        if( CardStatus != 0 )
        {
            qDebug() << QObject::tr( "CardStatus = " ) << CardStatus;

#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "读卡错误，请插好卡片！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Read Card Error!" ) );
#endif
            sleep( 5 );
            continue;
        }

        qDebug() << QObject::tr( "Read Card Information OK!" );

        CardStatus = ICCardHandler->readserialnumber( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                                      ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                                      Handler->CardPassword,
                                                      100,
                                                      Handler->CardSequenceNumber );

        if( CardStatus != 0 )
        {
            qDebug() << QObject::tr( "CardStatus = " ) << CardStatus;

#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "读卡错误，请插好卡片！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Read Card Sequence Number Error!" ) );
#endif
            sleep( 5 );
            continue;
        }

        qDebug() << QObject::tr( "Read Card Sequence Number OK!" );

        TemperoryIntegerNumber = ( int )CardBalanceBuffer[ 0 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int ) CardBalanceBuffer[ 1 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int )CardBalanceBuffer[ 2 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int )CardBalanceBuffer[ 3 ];

        TemperoryDoubleNumber = ( double )TemperoryIntegerNumber / 100.0;
        Handler->CardBalance = TemperoryDoubleNumber;

        TemperoryIntegerNumber = ( int )CardNumberBuffer[ 0 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int ) CardNumberBuffer[ 1 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int )CardNumberBuffer[ 2 ];
        TemperoryIntegerNumber <<= 8;
        TemperoryIntegerNumber += ( int )CardNumberBuffer[ 3 ];

        Handler->CardNumber = QString::number( TemperoryIntegerNumber );
        //Handler->CardSequenceNumber = Handler->CardNumber;

        qDebug() << QObject::tr( "CardBalance = " ) << Handler->CardBalance;
        qDebug() << QObject::tr( "CardNumber = " ) << Handler->CardNumber;
        qDebug() << QObject::tr( "CardSequenceNumber = " ) << QString::number( Handler->CardSequenceNumber );

#if 0

        TemperoryIntegerNumber = ( 10000 / 100 );
        CardBalanceBuffer[ 3 ] = ( unsigned char )( TemperoryIntegerNumber & 0x000000FF );
        CardBalanceBuffer[ 2 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x0000FF00 ) >> 8 );
        CardBalanceBuffer[ 1 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x00FF0000 ) >> 16 );
        CardBalanceBuffer[ 0 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0xFF000000 ) >> 24 );
        ICCardHandler->writecard( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                  ( unsigned char )( *Handler->PasswordEdition.toUtf8().data() ),
                                  ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                  ( unsigned char* )( Handler->CardPassword.toUtf8().data() ),
                                  CardNumberBuffer,
                                  CardBalanceBuffer,
                                  CardCompanyPasswordBuffer,
                                  CardDayTimeBuffer,
                                  CardTypeBuffer,
                                  1,
                                  100
                                  );

        qDebug() << QObject::tr( "Rewrite done." );

#endif

#if 1
        sleep( 5 );
        continue;
#endif

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
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络错误，请通知维护人员检查网络！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable!" ) );
#endif
            sleep( 5 );
            continue;
        }

        TemperoryNode = new MessageQueueNode();
        TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
        TemperoryNode->MessageAppID = MessageHandling::Precreate;
        TemperoryNode->MessageContent = RespondContent;

        MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
        TemperoryNode = NULL;

        pthread_cond_wait( &Handler->RequestQRCodeCondition, &Handler->RechargerChargeLocker );
        if( Handler->TradeStatus != QString( "PRECREATE_SUCCESS" ) )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "申请支付二维码失败！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Request QRCode Failed" ) );
#endif
            sleep( 5 );
            continue;
        }

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

        /* Query the status every 4s, 120s in total. */
        QueryResult = false;
        RemainingTime = 120;
        for( int i = 0; i < 30; ++i )
        {
            for( int j = 0; j < 4; ++j )
            {
#ifdef CHINESE_OUTPUT
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "请扫描二维码！ " ) + QString::number( RemainingTime ) );
#else
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Please Scan the QRCode! " )  + QString::number( RemainingTime ) );
#endif
                --RemainingTime;
                sleep( 1 );
            }

            CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

            RequestParameters = ( QObject::tr( "devId=" ) + QString::number( Handler->DeviceID, 10 ) );
            RequestParameters += ( QObject::tr( "&out_trade_no=" ) + Handler->TradeNumber );
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
            url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/trade/query?" ) + RequestParameters );
            qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

            RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );
            if( RequestResult != CURLE_OK )
            {
#ifdef CHINESE_OUTPUT
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "网络错误，请通知维护人员检查网络！" ) );
#else
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable!" ) );
#endif
                break;
            }

            TemperoryNode = new MessageQueueNode();
            TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
            TemperoryNode->MessageAppID = MessageHandling::Query;
            TemperoryNode->MessageContent = RespondContent;

            MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
            TemperoryNode = NULL;

            pthread_cond_wait( &Handler->QueryResultCondition, &Handler->RechargerChargeLocker );
            if( Handler->TradeStatus == QString( "TRADE_CLOSED" ) )
            {
#ifdef CHINESE_OUTPUT
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "付款超时，充值失败！" ) );
#else
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "TRADE_CLOSED!" ) );
#endif
                break;
            }

            if( ( Handler->TradeStatus == QString( "TRADE_SUCCESS" ) ) || ( Handler->TradeStatus == QString( "TRADE_FINISHED" ) ) )
            {
#ifdef CHINESE_OUTPUT
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "付款成功，准备写卡充值，请稍候！" ) );
#else
                CardRecharger::SelfInstance->SetStatusLabel( Handler->TradeStatus );
#endif
                QueryResult = true;
                break;
            }
        }

        if( QueryResult == false )
        {
            if( Handler->TradeStatus == QString( "PRECREATE_SUCCESS" ) )
            {
#ifdef CHINESE_OUTPUT
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "付款超时，充值失败！" ) );
#else
                CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "TRADE_CLOSED!" ) );
#endif
            }

            sleep( 5 );
            CardRecharger::SelfInstance->ResetQRView();
            continue;
        }

        /* PreRechargeCheck. */
        Handler->PreRechargeCheckIsSuccessfully = false;
        CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

        RequestParameters = ( QObject::tr( "balance=" ) + QString::number( Handler->CardBalance, '.', 0 ) );
        RequestParameters += ( QObject::tr( "&cardNum=" ) + Handler->CardNumber );
        RequestParameters += ( QObject::tr( "&devId=" ) + QString::number( Handler->DeviceID, 10 ) );
        RequestParameters += ( QObject::tr( "&out_trade_no=" ) + Handler->TradeNumber );
        RequestParameters += ( QObject::tr( "&sequenceNum=" ) + Handler->CardSequenceNumber );
        RequestParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );
        RequestParameters += ( QObject::tr( "&token=" ) + Handler->DeviceToken );
        RequestParameters += ( QObject::tr( "&total_amount=" ) + QString::number( Handler->RechargeValue ) );

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
        url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/trade/preRechargeCheck?" ) + RequestParameters );
        qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

        RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );
        if( RequestResult != CURLE_OK )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "因网络错误交易失败，请到服务中心进行灰记录处理！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable! PreRechargerCheck failed!" ) );
#endif
            break;
        }

        TemperoryNode = new MessageQueueNode();
        TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
        TemperoryNode->MessageAppID = MessageHandling::PreRechargeCheck;
        TemperoryNode->MessageContent = RespondContent;

        MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
        TemperoryNode = NULL;

        pthread_cond_wait( &Handler->PreRechargeCheckCondition, &Handler->RechargerChargeLocker );
        if( Handler->PreRechargeCheckIsSuccessfully == false )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "交易失败，请到服务中心进行灰记录处理！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "PreRechargerCheck failed!" ) );
#endif
            sleep( 5 );
            continue;
        }

        /* Write card. */
#ifdef CHINESE_OUTPUT
        CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "正在写卡中，请稍候！" ) );
#else
        CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Writing card!" ) );
#endif

        Handler->CardBalance += ( double )Handler->RechargeValue;
        TemperoryIntegerNumber = ( int )( Handler->CardBalance * 100 );
        CardNumberBuffer[ 3 ] = ( unsigned char )( TemperoryIntegerNumber & 0x000000FF );
        CardNumberBuffer[ 2 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x0000FF00 ) >> 8 );
        CardNumberBuffer[ 1 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x00FF0000 ) >> 16 );
        CardNumberBuffer[ 0 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0xFF000000 ) >> 24 );

        CardStatus = ICCardHandler->writecard( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                               ( unsigned char )( *Handler->PasswordEdition.toUtf8().data() ),
                                               ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                               Handler->CardPassword,
                                               CardNumberBuffer,
                                               CardBalanceBuffer,
                                               CardCompanyPasswordBuffer,
                                               CardDayTimeBuffer,
                                               CardTypeBuffer,
                                               1,
                                               100
                                               );

        qDebug() << QObject::tr( "CardStatus = " ) << CardStatus;
        sleep( 5 );

        /* RechargeFinish. */
        Handler->IsRechargeFinish = false;
        CurrentTimestampString = QString::number( TimestampHandling::GetInstance()->GetTimestamp(), '.', 0 );

        RequestParameters = ( QObject::tr( "balance=" ) + QString::number( Handler->CardBalance, '.', 0 ) );
        RequestParameters += ( QObject::tr( "&cardNum=" ) + Handler->CardNumber );
        RequestParameters += ( QObject::tr( "&devId=" ) + QString::number( Handler->DeviceID, 10 ) );
        RequestParameters += ( QObject::tr( "&out_trade_no=" ) + Handler->TradeNumber );
        if( CardStatus == 0 )
        {
            RequestParameters += ( QObject::tr( "&recharge_status=" ) + QObject::tr( "SUCCESS" ) );
        }
        else
        {
            RequestParameters += ( QObject::tr( "&recharge_status=" ) + QObject::tr( "FAIL" ) );
        }
        RequestParameters += ( QObject::tr( "&sequenceNum=" ) + Handler->CardSequenceNumber );
        RequestParameters += ( QObject::tr( "&timestamp=" ) + CurrentTimestampString );
        RequestParameters += ( QObject::tr( "&token=" ) + Handler->DeviceToken );
        RequestParameters += ( QObject::tr( "&total_amount=" ) + QString::number( Handler->RechargeValue ) );

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
        url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/trade/rechargeFinish?" ) + RequestParameters );
        qDebug() << QObject::tr( "RequestGet:" ) << url.toString();

        RequestResult = Handler->RechargerClient.RequestGet( url, RespondContent );
        if( RequestResult != CURLE_OK )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "因网络错误交易失败，请到服务中心进行灰记录处理！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "Network is unavilable! RechargeFinish failed!" ) );
#endif

            /* ReWrite the card. */
            Handler->CardBalance -= ( double )Handler->RechargeValue;
            TemperoryIntegerNumber = ( int )( Handler->CardBalance * 100 );
            CardNumberBuffer[ 3 ] = ( unsigned char )( TemperoryIntegerNumber & 0x000000FF );
            CardNumberBuffer[ 2 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x0000FF00 ) >> 8 );
            CardNumberBuffer[ 1 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x00FF0000 ) >> 16 );
            CardNumberBuffer[ 0 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0xFF000000 ) >> 24 );
            ICCardHandler->writecard( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                      ( unsigned char )( *Handler->PasswordEdition.toUtf8().data() ),
                                      ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                      Handler->CardPassword,
                                      CardNumberBuffer,
                                      CardBalanceBuffer,
                                      CardCompanyPasswordBuffer,
                                      CardDayTimeBuffer,
                                      CardTypeBuffer,
                                      1,
                                      100
                                      );

            sleep( 5 );
            break;
        }

        TemperoryNode = new MessageQueueNode();
        TemperoryNode->MessageGroupID = MessageHandling::RechargerMessages;
        TemperoryNode->MessageAppID = MessageHandling::RechargeFinish;
        TemperoryNode->MessageContent = RespondContent;

        MessageHandling::GetInstance()->MessageQueuePointer->MessageEnqueue( TemperoryNode );
        TemperoryNode = NULL;

        pthread_cond_wait( &Handler->RechargeFinishCondition, &Handler->RechargerChargeLocker );
        if( Handler->IsRechargeFinish == false )
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "交易失败，请到服务中心进行灰记录处理！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "RechargeFinish failed!" ) );
#endif

            /* ReWrite the card. */

            Handler->CardBalance -= ( double )Handler->RechargeValue;
            TemperoryIntegerNumber = ( int )( Handler->CardBalance * 100 );
            CardNumberBuffer[ 3 ] = ( unsigned char )( TemperoryIntegerNumber & 0x000000FF );
            CardNumberBuffer[ 2 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x0000FF00 ) >> 8 );
            CardNumberBuffer[ 1 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0x00FF0000 ) >> 16 );
            CardNumberBuffer[ 0 ] = ( unsigned char )( ( TemperoryIntegerNumber & 0xFF000000 ) >> 24 );
            ICCardHandler->writecard( ( unsigned char* )( CardRecharger::SelfInstance->DeviceSerials.toUtf8().data() ),
                                      ( unsigned char )( *Handler->PasswordEdition.toUtf8().data() ),
                                      ( unsigned char )( *Handler->SectionNumber.toUtf8().data() ),
                                     Handler->CardPassword,
                                      CardNumberBuffer,
                                      CardBalanceBuffer,
                                      CardCompanyPasswordBuffer,
                                      CardDayTimeBuffer,
                                      CardTypeBuffer,
                                      1,
                                      100
                                      );

            sleep( 5 );
            continue;
        }
        else
        {
#ifdef CHINESE_OUTPUT
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "交易成功！" ) );
#else
            CardRecharger::SelfInstance->SetStatusLabel( QObject::tr( "RechargeFinish Successfully!" ) );
#endif
        }

        sleep( 5 );
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
        pthread_cond_signal( &this->RequestQRCodeCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->RequestQRCodeCondition );
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
        pthread_cond_signal( &this->RequestQRCodeCondition );
        return;
    }

    QString qrcode = cJSON_GetObjectItem( root, "qrcode" )->valuestring;
    QString out_trade_no = cJSON_GetObjectItem( root, "out_trade_no" )->valuestring;
    QString trade_status = cJSON_GetObjectItem( root, "trade_status" )->valuestring;

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->RequestQRCodeCondition );
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
        pthread_cond_signal( &this->RequestQRCodeCondition );
        return;
    }

    this->QRCodeAddress = qrcode;
    this->TradeNumber = out_trade_no;
    this->TradeStatus = trade_status;

    //qDebug() << QObject::tr( "qrcode = " ) << this->QRCodeAddress;
    //qDebug() << QObject::tr( "trade number = " ) << this->TradeNumber;
    //qDebug() << QObject::tr( "trade status = " ) << this->TradeStatus;
    pthread_cond_signal( &this->RequestQRCodeCondition );

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



void RechargerHandling::ParseQueryMessage(QString &Message)
{
    char rechargerbuffer[ 1024 ];
    QRegExp RegularExpression;
    RegularExpression.setPattern( QObject::tr( "(\\{.*\\})([0-9a-z]{32})"));

    RegularExpression.indexIn( Message );
    QStringList list = RegularExpression.capturedTexts();

    if( list.size() != 3 )
    {
        pthread_cond_signal( &this->QueryResultCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->QueryResultCondition );
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
        pthread_cond_signal( &this->QueryResultCondition );
        return;
    }

    QString trade_status = cJSON_GetObjectItem( root, "trade_status" )->valuestring;

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->QueryResultCondition );
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
        pthread_cond_signal( &this->QueryResultCondition );
        return;
    }

    this->TradeStatus = trade_status;

    //qDebug() << QObject::tr( "trade status = " ) << this->TradeStatus;
    pthread_cond_signal( &this->QueryResultCondition );
}


void RechargerHandling::ParsePreRechargeCheckMessage(QString &Message)
{
    char rechargerbuffer[ 1024 ];
    QRegExp RegularExpression;
    RegularExpression.setPattern( QObject::tr( "(\\{.*\\})([0-9a-z]{32})"));

    RegularExpression.indexIn( Message );
    QStringList list = RegularExpression.capturedTexts();

    if( list.size() != 3 )
    {
        pthread_cond_signal( &this->PreRechargeCheckCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->PreRechargeCheckCondition );
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
        pthread_cond_signal( &this->PreRechargeCheckCondition );
        return;
    }

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->PreRechargeCheckCondition );
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
        pthread_cond_signal( &this->PreRechargeCheckCondition );
        return;
    }

    this->PreRechargeCheckIsSuccessfully = true;
    pthread_cond_signal( &this->PreRechargeCheckCondition );
}


void RechargerHandling::ParseRechargeFinishMessage(QString &Message)
{
    char rechargerbuffer[ 1024 ];
    QRegExp RegularExpression;
    RegularExpression.setPattern( QObject::tr( "(\\{.*\\})([0-9a-z]{32})"));

    RegularExpression.indexIn( Message );
    QStringList list = RegularExpression.capturedTexts();

    if( list.size() != 3 )
    {
        pthread_cond_signal( &this->RechargeFinishCondition );
        return;
    }

    QString Message_JSON = list.at( 1 );
    QString Message_MD5 = list.at( 2 );

    if( Message_JSON == QString( "" ) )
    {
        pthread_cond_signal( &this->RechargeFinishCondition );
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
        pthread_cond_signal( &this->RechargeFinishCondition );
        return;
    }

    cJSON_Delete( root );


    /* Checkout the MD5 */
    if( Message_MD5 == QString( "" ) )
    {
        pthread_cond_signal( &this->RechargeFinishCondition );
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
        pthread_cond_signal( &this->RechargeFinishCondition );
        return;
    }

    this->IsRechargeFinish = true;
    pthread_cond_signal( &this->RechargeFinishCondition );
}

