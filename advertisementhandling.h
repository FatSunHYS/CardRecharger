#ifndef ADVERTISEMENTHANDLING_H
#define ADVERTISEMENTHANDLING_H

#include <QString>

#include <pthread.h>
#include "httpclient.h"

class AdvertisementHandling
{
public:
    HttpClient AdvertisementClient;
    int AdvertisementTotal;
    char* JSONBuffer;
    char* CommandBuffer;
    bool DownloadIsDone;

    static AdvertisementHandling* GetInstance();
    bool CreatePThread();


private:
    static AdvertisementHandling* PrivateInstance;
    pthread_t AdvertisementHandling1PthreadID;
    pthread_t AdvertisementHandling2PthreadID;



    AdvertisementHandling();
};

void* AdvertisementDownloadHandler( void* arg );
void* AdvertisementShowHandler( void* arg );

#endif // ADVERTISEMENTHANDLING_H
