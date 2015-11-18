#include "advertisementhandling.h"

#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QImage>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "cardrecharger.h"
#include "timestamphandling.h"
#include "cJSON.h"

#define BUFFERLENGTH 16384

AdvertisementHandling* AdvertisementHandling::PrivateInstance = NULL;

AdvertisementHandling::AdvertisementHandling()
{
    this->JSONBuffer = new char[ BUFFERLENGTH ];
    this->CommandBuffer = new char[ BUFFERLENGTH ];
    this->DownloadIsDone = false;
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
    cJSON* root;
    cJSON* AdvertiseList;
    cJSON* AdvertiseItem;

    if( arg != NULL )
    {

    }

    qDebug() << QObject::tr( "AdvertisementDownloadHandler is running..." );

    memset( Handler->JSONBuffer, 0, BUFFERLENGTH );
    memset( Handler->CommandBuffer, 0, BUFFERLENGTH );

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

        strcpy( Handler->JSONBuffer, RespondContent.toUtf8().data() );
        root = cJSON_Parse( Handler->JSONBuffer );

        if( cJSON_GetObjectItem( root, "code" )->valueint != 0 )
        {
            qDebug() << QObject::tr( "AdvertisementDownload Request error! Retry after 3 second..." );
            sleep( 3 );
            continue;
        }

        break;
    }

    AdvertiseList = cJSON_GetObjectItem( root, "rows" );
    Handler->AdvertisementTotal = cJSON_GetArraySize( AdvertiseList );

    for( int i = 0; i < Handler->AdvertisementTotal; ++i )
    {
        AdvertiseItem = cJSON_GetArrayItem( AdvertiseList, i );
        sprintf( Handler->CommandBuffer, "wget -q -O ./Advertisement/%d %s\n", i, cJSON_GetObjectItem( AdvertiseItem, "adPicUrl" )->valuestring );
        qDebug( "%s", Handler->CommandBuffer );
        system( Handler->CommandBuffer );
    }

    cJSON_Delete( root );

    qDebug() << QObject::tr( "AdvertisementDownloadHandler is finished..." );

    Handler->DownloadIsDone = true;

    return ( void* )0;
}


void* AdvertisementShowHandler( void* arg )
{
    int i;
    AdvertisementHandling* Handler = AdvertisementHandling::GetInstance();
    QImage* image;

    if( arg != NULL )
    {

    }

    qDebug() << QObject::tr( "AdvertisementShowHandler is running..." );

    while( Handler->DownloadIsDone == false )
    {

    }

    while( true )
    {
        if( Handler->AdvertisementTotal == 0 )
        {
            CardRecharger::SelfInstance->ResetAdvertisementView();
            sleep( 900 );
            continue;
        }

        for( i = 0; i < Handler->AdvertisementTotal; ++i )
        {
            qDebug( "Playing the %i advertisements.", i );
            image = new QImage();
            sprintf( Handler->CommandBuffer, "./Advertisement/%d", i );
            image->load( QString( Handler->CommandBuffer ) );
            CardRecharger::SelfInstance->SetAdvertisementView( image );
            sleep( 30 );
            CardRecharger::SelfInstance->ResetAdvertisementView();
        }

    }

    return ( void* )0;
}
















