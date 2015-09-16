#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QUrl>
#include <QHttp>

class HttpClient : public QObject
{
	Q_OBJECT
public:
	static HttpClient* GetHttpClientInstance();
	void Request( QUrl& url );

signals:

public slots:

private slots:
	void RequestFinished( int requestId, bool error );
	void ReadResponseHeader( const QHttpResponseHeader &responseHeader );
	void UpdateDataReadProgress( int bytesread, int totalbytes );
	void HttpDone( bool error );

private:
	static HttpClient* PrivateInstance;
	QHttp* HttpFD;
	int RequestId;
	bool RequestAborted;

	explicit HttpClient(QObject *parent = 0);


};

#endif // HTTPCLIENT_H
