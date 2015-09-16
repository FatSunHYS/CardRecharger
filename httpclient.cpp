#include "httpclient.h"

#include <QDebug>

HttpClient* HttpClient::PrivateInstance = NULL;


HttpClient* HttpClient::GetHttpClientInstance()
{
	if( HttpClient::PrivateInstance == NULL )
	{
		HttpClient::PrivateInstance = new HttpClient();
	}

	return HttpClient::PrivateInstance;
}

void HttpClient::Request(QUrl &url)
{
	this->HttpFD->setHost( url.host(), url.port() );
	this->RequestId = this->HttpFD->get( url.path() );
	this->RequestAborted = false;
}


HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
	this->HttpFD = new QHttp( this );
	this->RequestId = -1;
	this->RequestAborted = false;

	connect( this->HttpFD, SIGNAL( done( bool ) ), this, SLOT( HttpDone(bool)) );
	connect( this->HttpFD, SIGNAL( requestFinished(int,bool)), this, SLOT(RequestFinished(int,bool)) );
	connect( this->HttpFD, SIGNAL( dataReadProgress(int,int)), this, SLOT(UpdateDataReadProgress(int,int)));
	connect( this->HttpFD, SIGNAL( responseHeaderReceived( const QHttpResponseHeader& )), this, SLOT(ReadResponseHeader( const QHttpResponseHeader&)));


}


void HttpClient::ReadResponseHeader(const QHttpResponseHeader &responseHeader)
{
	if( responseHeader.statusCode() != 200 )
	{
		qDebug() << tr( "Download Failed: %1.").arg( responseHeader.reasonPhrase() ) << endl;
		this->RequestAborted = true;
		this->HttpFD->abort();
	}
}


void HttpClient::RequestFinished(int requestId, bool error)
{
	if( this->RequestAborted )
	{
		return;
	}

	if( requestId != this->RequestId )
	{
		return;
	}

	if( error )
	{
		qDebug() << tr( "Download Failed： %1.").arg( HttpFD->errorString()) << endl;
	}
	else
	{
		qDebug() << tr( "Download Successfully!" ) << endl;
	}
}


void HttpClient::UpdateDataReadProgress(int bytesread, int totalbytes)
{
	qDebug() << tr( "Received： " ) << bytesread << tr( " Bytes.");
}

void HttpClient::HttpDone(bool error)
{
	if( error )
	{
		qDebug() << tr( "Error!") << qPrintable( HttpFD->errorString()) << endl;
	}
	else
	{
		QString TemperoryString( HttpFD->readAll() );
		qDebug() << tr( "Received: ") << TemperoryString << endl;
	}
}



