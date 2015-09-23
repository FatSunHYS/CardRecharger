/*
 * This class is responsed to calibrate the system unix timestamp and provide the unix timestamp in order to communicate with the server.
 * The system unix timestamp will be calibrated per hour.
 *
 * */


#ifndef TIMESTAMPHANDLING_H
#define TIMESTAMPHANDLING_H

#include <QObject>
#include <QThread>

#include "httpclient.h"


class TimestampHandling : public QThread
{
	Q_OBJECT
public:
	static TimestampHandling* GetInstance();
	void CalibrateTimestamp( double newtimestamp );
	double GetTimestamp();

protected:
	void run();

signals:

public slots:

private:
	static TimestampHandling* PrivateInstace;
	bool TimestampIsInitialized;
	HttpClient TimestampClient;
	double unixtimestamp;

	explicit TimestampHandling(QObject *parent = 0);

};

#endif // TIMESTAMPHANDLING_H
