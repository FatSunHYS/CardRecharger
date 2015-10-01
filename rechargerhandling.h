#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QCryptographicHash>
#include <QString>

#include <pthread.h>

#include "httpclient.h"

class RechargerHandling
{
public:
	HttpClient RechargerClient;
	QCryptographicHash* EncrpytMD5;
	pthread_mutex_t MD5Locker;
	int DeviceID;
	QString DeviceToken;
	bool DeviceIsLogin;
	bool IsKeepAlived;

	static RechargerHandling* GetInstance();
	bool CreatePThread();
	void ParseLoginMessage( QString& Message );
	void ParseKeepAlivedMessage( QString& Message );

private:
	static RechargerHandling* PrivateInstance;
	pthread_t RechargerHandlingPthreadID;


	RechargerHandling();


};

void* RechargerLoginHandler( void* arg );

#endif // RECHARGERHANDLING_H
