#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QUrl>
#include <QHttp>
#include <QtNetwork>

class HttpClient : public QObject
{
	Q_OBJECT
public:


	explicit HttpClient(QObject *parent = 0);
	void RequestGet( QUrl& url, int messagegroup, int messageappid );
	void RequestPost( QUrl& url, QByteArray& postdata, int messagegroup, int messageappid );

signals:

public slots:

private slots:
	//void ReadResponseHeader( const QHttpResponseHeader &responseHeader );
	//void HttpDone( bool error );
	void ReplyFinish( QNetworkReply* reply );

private:
	QNetworkAccessManager* HttpFD;
	//QHttp* HttpFD;
	int RequestId;
	bool RequestAborted;
	int MessageGroupID;
	int MessageAppID;

};

#endif // HTTPCLIENT_H
