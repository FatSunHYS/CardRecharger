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

signals:

public slots:

private:
	static HttpClient* PrivateInstance;
	QHttp* HttpFD;
	int RequestId;

public:
	explicit HttpClient(QObject *parent = 0);
};

#endif // HTTPCLIENT_H
