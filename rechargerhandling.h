#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QCryptographicHash>
#include <QString>
#include <QPainter>


#include <pthread.h>

#include "httpclient.h"
#include "qrencode.h"

class RechargerHandling
{
public:
	enum _PayWay
	{
		AliPay = 0,
		WeiXinPay
	}PayWay;

	HttpClient RechargerClient;
	QCryptographicHash* EncrpytMD5;
    pthread_mutex_t MD5Locker;
	pthread_mutex_t RechargerChargeLocker;
    pthread_cond_t ChargeActionCondition;
    pthread_mutex_t RechargerReadBalanceLocker;
    pthread_cond_t WaitReadBalanceCondition;
    pthread_mutex_t RechargerAshRecordLocker;
    pthread_cond_t WaitAshRecordCondition;

	int DeviceID;
	QString DeviceToken;
	bool DeviceIsLogin;
	bool IsKeepAlived;
	bool PreRechargeCheckIsSuccessfully;
	bool IsRechargeFinish;

    unsigned char PasswordEdition;
    unsigned char CardPassword[ 6 ];
    unsigned char SectionNumber;
    int CardNumber;
    double CardBalance;
    double ConsumptionRecord;
    unsigned int CardSequenceNumber;
	QString QRCodeAddress;
	QString TradeNumber;
	QString TradeStatus;

	int RechargeValue;
	_PayWay RechargePayWay;

	static RechargerHandling* GetInstance();
	bool CreatePThread();
	void ParseLoginMessage( QString& Message );
	void ParseKeepAlivedMessage( QString& Message );
	void ParsePrecreateMessage( QString& Message );
	void ParseQueryMessage( QString& Message );
	void ParsePreRechargeCheckMessage( QString& Message );
	void ParseRechargeFinishMessage( QString& Message );
	void DrawQRcodeImage( QRcode* qr, QPainter &painter, int width, int height );
	void SendUpdateQRImageSignal();

private:
	static RechargerHandling* PrivateInstance;
	pthread_t RechargerHandling1PthreadID;
	pthread_t RechargerHandling2PthreadID;
    pthread_t RechargerHandling3PthreadID;
    pthread_t RechargerHandling4PthreadID;


	RechargerHandling();


};

void* RechargerLoginHandler( void* arg );
void* RechargerChargeHandler( void* arg );
void* RechargerReadBalanceHandler( void* arg );
void* RechargerAshRecordHandler( void* arg );

#endif // RECHARGERHANDLING_H
