#include "httpclient.h"

#include <QDebug>

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
	connect( this->HttpFD, SIGNAL( responseHeaderReceived( const QHttpResponseHeader& )), this, SLOT(ReadResponseHeader( const QHttpResponseHeader&)));


}


void HttpClient::ReadResponseHeader(const QHttpResponseHeader &responseHeader)
{
	if( responseHeader.statusCode() != 200 )
	{
		qDebug() << tr( "Request ID") << this->RequestId << tr( ": ") << endl;
		qDebug() << tr( "Download Failed: %1.").arg( responseHeader.reasonPhrase() ) << endl;
		this->RequestAborted = true;
		this->HttpFD->abort();
	}
}

void HttpClient::HttpDone(bool error)
{
	qDebug() << tr( "Request ID") << this->RequestId << tr( ": ") << endl;

	if( error )
	{
		qDebug() << tr( "Error!") << qPrintable( this->HttpFD->errorString()) << endl;
	}
	else
	{
		QString TemperoryString( this->HttpFD->readAll() );
		qDebug() << tr( "Received: ") << TemperoryString << endl;
	}
}



