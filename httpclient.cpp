#include <QObject>
#include <QDebug>

#include "httpclient.h"



CURLcode HttpClient::RequestGet( QUrl& url, QString& RespondContent )
{
	CURLcode requestresult;
	std::string TemperoryContent;

	this->HttpFD = curl_easy_init();

	if( this->HttpFD == NULL )
	{
		curl_easy_cleanup( this->HttpFD );
		this->HttpFD = NULL;
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt( this->HttpFD, CURLOPT_URL, url.toString().toUtf8().data() );
	curl_easy_setopt( this->HttpFD, CURLOPT_NOSIGNAL, 1 );
	curl_easy_setopt( this->HttpFD, CURLOPT_WRITEFUNCTION, HttpClientContentReceived );
	curl_easy_setopt( this->HttpFD, CURLOPT_WRITEDATA, ( void* )&TemperoryContent );
	curl_easy_setopt( this->HttpFD, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt( this->HttpFD, CURLOPT_TIMEOUT, 3);
	requestresult = curl_easy_perform( this->HttpFD );
	curl_easy_cleanup( this->HttpFD );
	this->HttpFD = NULL;

	RespondContent = QString::fromStdString( TemperoryContent );

	return requestresult;
}


CURLcode HttpClient::RequestPost( QUrl& url, QString& Postdata, QString& RespondContent )
{
	return CURLE_OK;
}



HttpClient::HttpClient()
{
	this->HttpFD = NULL;
}

size_t HttpClientContentReceived( void *ptr, size_t size, size_t nmemb, void *stream )
{
	std::string* destinationpointer = dynamic_cast< std::string* >( ( std::string* )stream );
	if( NULL == destinationpointer || NULL == ptr )
	{
	return -1;
	}

	char* sourcepointer = (char*)ptr;
	destinationpointer->append( sourcepointer, size * nmemb);

	return nmemb;
}
