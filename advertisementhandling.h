#ifndef ADVERTISEMENTHANDLING_H
#define ADVERTISEMENTHANDLING_H


#include <pthread.h>
#include "httpclient.h"

class AdvertisementHandling
{
public:
    HttpClient AdvertisementClient;

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
