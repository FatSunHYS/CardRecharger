#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QString>
#include <QUrl>

#include <curl/curl.h>
#include <iostream>
using namespace std;

class HttpClient
{
public:

	HttpClient();
	CURLcode RequestGet( QUrl& url, QString& RespondContent );
	CURLcode RequestPost( QUrl& url, QString& Postdata, QString& RespondContent );

private:
	CURL* HttpFD;

};

size_t HttpClientContentReceived( void *ptr, size_t size, size_t nmemb, void *stream );

#endif // HTTPCLIENT_H
