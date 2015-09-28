#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QCryptographicHash>

#include <pthread.h>
#include <curl/curl.h>

class RechargerHandling
{
public:
	CURL* RechargerClient;

	static RechargerHandling* GetInstance();
	bool CreatePThread();

private:
	static RechargerHandling* PrivateInstance;
	QCryptographicHash* EncrpytMD5;
	pthread_t RechargerHandlingPthreadID;

	RechargerHandling();


};

#endif // RECHARGERHANDLING_H
