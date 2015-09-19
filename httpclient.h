#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QUrl>
#include <QHttp>

class HttpClient : public QObject
{
	Q_OBJECT
public:
	explicit HttpClient(QObject *parent = 0);
	void Request( QUrl& url );

signals:

public slots:

private slots:
	void ReadResponseHeader( const QHttpResponseHeader &responseHeader );
	void HttpDone( bool error );

private:
	QHttp* HttpFD;
	int RequestId;
	bool RequestAborted;

};

#endif // HTTPCLIENT_H
