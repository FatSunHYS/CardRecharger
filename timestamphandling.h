/*
 * This class is responsed to calibrate the system unix timestamp and provide the unix timestamp in order to communicate with the server.
 * The system unix timestamp will be calibrated per hour.
 *
 * */


#ifndef TIMESTAMPHANDLING_H
#define TIMESTAMPHANDLING_H

#include <QObject>
#include <QThread>
#include <QtNetwork>

class TimestampHandling : public QThread
{
	Q_OBJECT
public:
	static TimestampHandling* GetInstance();
	void CalibrateTimestamp( double newtimestamp );
	double GetTimestamp();
	bool IsFirstInitialed();

protected:
	void run();

signals:

public slots:

private slots:
	void ReplyFinish( QNetworkReply* reply );

private:
	static TimestampHandling* PrivateInstace;
	QNetworkAccessManager* HttpFD;
	QNetworkRequest* HttpRequest;
	bool TimestampIsInitialized;
	double unixtimestamp;
	bool FirstInitialed;

	explicit TimestampHandling(QObject *parent = 0);


};


//extern HttpClient* TimestampHttpClient;


#endif // TIMESTAMPHANDLING_H
