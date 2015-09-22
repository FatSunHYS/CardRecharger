#include "rechargerhandling.h"
#include "httpclient.h"
#include "messagehandling.h"

#include <sys/time.h>


RechargerHandling* RechargerHandling::PrivateInstance = NULL;

RechargerHandling::RechargerHandling(QObject *parent) : QThread( parent )
{
	this->IsClientGetServerTime = false;
}


RechargerHandling* RechargerHandling::GetInstance()
{
	if( RechargerHandling::PrivateInstance == NULL )
	{
		RechargerHandling::PrivateInstance = new RechargerHandling();
	}

	return RechargerHandling::PrivateInstance;
}

void RechargerHandling::run()
{
	//HttpClient RechargerClient;

	/* Step 1, get the server system time. */






	while( true )
	{

	}


}


void RechargerHandling::GetServerTimeSuccessfully()
{

}
