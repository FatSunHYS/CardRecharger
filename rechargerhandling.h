#ifndef RECHARGERHANDLING_H
#define RECHARGERHANDLING_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

class RechargerHandling : public QThread
{
	Q_OBJECT
public:
	static RechargerHandling* GetInstance();

	void GetServerTimeSuccessfully();


protected:
	void run();

signals:

public slots:

private:
	static RechargerHandling* PrivateInstance;
	bool IsClientGetServerTime;

	explicit RechargerHandling( QObject* parent = 0 );


};

#endif // RECHARGERHANDLING_H
