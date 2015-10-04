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
	pthread_mutex_t RechargerLoginLocker;
	pthread_cond_t LoginCondition;
	pthread_cond_t HeartPackageCondition;
	pthread_mutex_t RechargerChargeLocker;
	pthread_cond_t ChargeActionCondition;
	pthread_cond_t RequestQRCodeCondition;
	pthread_cond_t QueryResultCondition;
	pthread_cond_t PreRechargeCheckCondition;
	pthread_cond_t RechargeFinishCondition;
	int DeviceID;
	QString DeviceToken;
	bool DeviceIsLogin;
	bool IsKeepAlived;
	bool PreRechargeCheckIsSuccessfully;
	bool IsRechargeFinish;

	QString PasswordEdition;
	QString CardPassword;
	QString SectionNumber;
	QString CardNumber;
	QString CardBalance;
	QString CardSequenceNumber;
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


signals:
	void UpdateQRImageClick();

private:
	static RechargerHandling* PrivateInstance;
	pthread_t RechargerHandling1PthreadID;
	pthread_t RechargerHandling2PthreadID;


	RechargerHandling();


};

void* RechargerLoginHandler( void* arg );
void* RechargerChargeHandler( void* arg );

#endif // RECHARGERHANDLING_H
