#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QCryptographicHash>

#include <pthread.h>

#include "httpclient.h"

class RechargerHandling
{
public:
	HttpClient RechargerClient;
	QCryptographicHash* EncrpytMD5;
	pthread_mutex_t MD5Locker;

	static RechargerHandling* GetInstance();
	bool CreatePThread();
	void ParseLoginMessage( QString& Message );

private:
	static RechargerHandling* PrivateInstance;
	pthread_t RechargerHandlingPthreadID;


	RechargerHandling();


};



void* RechargerHandler( void* arg );

#endif // RECHARGERHANDLING_H
