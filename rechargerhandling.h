#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "httpclient.h"

class RechargerHandling : public QThread
{
	Q_OBJECT
public:
	static RechargerHandling* GetInstance();


protected:
	void run();

signals:

public slots:

private:
	static RechargerHandling* PrivateInstance;
	HttpClient RechargerClient;

	explicit RechargerHandling( QObject* parent = 0 );


};

#endif // RECHARGERHANDLING_H
