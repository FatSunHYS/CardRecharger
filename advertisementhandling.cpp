#include "advertisementhandling.h"

#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>

#include <unistd.h>

#include "cardrecharger.h"
#include "timestamphandling.h"
#include "cJSON.h"

AdvertisementHandling* AdvertisementHandling::PrivateInstance = NULL;

AdvertisementHandling::AdvertisementHandling()
{

}

AdvertisementHandling* AdvertisementHandling::GetInstance()
{
    if( AdvertisementHandling::PrivateInstance == NULL )
    {
        AdvertisementHandling::PrivateInstance = new AdvertisementHandling();
    }

    return AdvertisementHandling::PrivateInstance;
}

bool AdvertisementHandling::CreatePThread()
{
    int err = pthread_create( &( this->AdvertisementHandling1PthreadID ), NULL, AdvertisementDownloadHandler, NULL );

    if( err != 0 )
    {
        qDebug() << QObject::tr( "Create pthread AdvertisementDownloadHandler error!" );
        return false;
    }
    else
    {
        qDebug() << QObject::tr( "Create pthread AdvertisementDownloadHandler Successfully." );
    }

    err = pthread_create( &( this->AdvertisementHandling2PthreadID ), NULL, AdvertisementShowHandler, NULL );

    if( err != 0 )
    {
        qDebug() << QObject::tr( "Create pthread AdvertisementShowHandler error!" );
        return false;
    }
    else
    {
        qDebug() << QObject::tr( "Create pthread AdvertisementShowHandler Successfully." );
    }

    return true;
}


void* AdvertisementDownloadHandler( void* arg )
{
    QUrl url;
    CURLcode RequestResult;
    QString RespondContent;
    AdvertisementHandling* Handler = AdvertisementHandling::GetInstance();

    if( arg != NULL )
    {

    }

    qDebug() << QObject::tr( "AdvertisementDownloadHandler is running..." );

    // http://120.25.81.161:8080/XYJR/clientapi/adpicture/getAdPitureList

    url.clear();
    url.setUrl( CardRecharger::SelfInstance->CardRechargerServerURL + QObject::tr( "/clientapi/adpicture/getAdPitureList" ) );

    while( true )
    {
        RequestResult = Handler->AdvertisementClient.RequestGet( url, RespondContent );

        if( RequestResult != CURLE_OK )
        {
            qDebug() << QObject::tr( "AdvertisementDownload Request error! Retry after 3 second..." );
            sleep( 3 );
            continue;
        }
        else
        {
            break;
        }
    }


    qDebug() << QObject::tr( "AdvertisementDownloadHandler is finished..." );

    return ( void* )0;
}


void* AdvertisementShowHandler( void* arg )
{
    if( arg != NULL )
    {

    }

    qDebug() << QObject::tr( "AdvertisementShowHandler is running..." );

    while( true )
    {

        sleep( 120 );
    }

    return ( void* )0;
}





















