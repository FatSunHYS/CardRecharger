#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QCryptographicHash>
#include <QString>

#include <pthread.h>

#include "httpclient.h"

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
	pthread_mutex_t RechargeLocker;
	pthread_cond_t ChargeToCard;
	int DeviceID;
	QString DeviceToken;
	bool DeviceIsLogin;
	bool IsKeepAlived;

	int RechargeValue;
	_PayWay RechargePayWay;

	static RechargerHandling* GetInstance();
	bool CreatePThread();
	void ParseLoginMessage( QString& Message );
	void ParseKeepAlivedMessage( QString& Message );

private:
	static RechargerHandling* PrivateInstance;
	pthread_t RechargerHandling1PthreadID;
	pthread_t RechargerHandling2PthreadID;


	RechargerHandling();


};

void* RechargerLoginHandler( void* arg );
void* RechargerChargeHandler( void* arg );

#endif // RECHARGERHANDLING_H
