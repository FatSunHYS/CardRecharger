#include "httpclient.h"


HttpClient* HttpClient::PrivateInstance = NULL;


HttpClient* HttpClient::GetHttpClientInstance()
{
	if( HttpClient::PrivateInstance == NULL )
	{
		HttpClient::PrivateInstance = new HttpClient();
	}

	return HttpClient::PrivateInstance;
}


HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
	this->HttpFD = NULL;
	this->RequestId = -1;

}

