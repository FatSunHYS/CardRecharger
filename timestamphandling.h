/*
 * This class is responsed to calibrate the system unix timestamp and provide the unix timestamp in order to communicate with the server.
 * The system unix timestamp will be calibrated per hour.
 *
 * */


#ifndef TIMESTAMPHANDLING_H
#define TIMESTAMPHANDLING_H

#include <pthread.h>
#include <curl/curl.h>

#include "httpclient.h"

class TimestampHandling
{
public:
	HttpClient TimestampClient;

	static TimestampHandling* GetInstance();
	void CalibrateTimestamp( double newtimestamp );
	double GetTimestamp();
	bool IsFirstInitialed();
	bool CreatePThread();
	bool GetTimestampRefreshState();
	void SetTimestampRefreshState( bool newstate );
	void FirstInitialDone();
	void RefreshTimestamp();


private:
	static TimestampHandling* PrivateInstace;
	bool TimestampIsRefreshed;
	double unixtimestamp;
	bool FirstInitialed;
	pthread_t TimestampHandlingPthreadID;

	TimestampHandling();


};


void* TimestampHandler( void* arg );
size_t TimestampMessageReceived( void *ptr, size_t size, size_t nmemb, void *stream );


#endif // TIMESTAMPHANDLING_H
