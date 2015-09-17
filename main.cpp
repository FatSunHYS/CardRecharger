#include "cardrecharger.h"
#include <QApplication>
#include <QTextCodec>

#ifdef EMBEDDED_BOARD
#include <QWSServer>
#endif

#include "globalparameter.h"
#include "inifile.h"

/*
 * 初始化相关变量
 */
int init()
{

    //初始化各类参数
    if(!read_profile_string("ClientIdentifier", "ClientID", ClientIDStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : ClientIdentifier, Programme exit!\n");
        return 0;
    }

    if(!read_profile_string("ClientPassword", "ClientPW", ClientPWStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : ClientPassword, Programme exit!\n");
        return 0;
    }


    if(!read_profile_string("ServerBASEURL", "ServerBURL", ServerBaseURLStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : ServerBASEURL, Programme exit!\n");
        return 0;
    }


    if(!read_profile_string("AdvertisementURL", "AdServerURL", AdvtServerURLStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : AdvertisementURL, Programme exit!\n");
        return 0;
    }


    if(!read_profile_string("ICDevice", "Serials", ICReadDevStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : ICDevice, Programme exit!\n");
        return 0;
    }


    HeartBeatInteral = read_profile_int("Interval","ISeconds",-1,"DevInfo.ini");
    if(-1 == HeartBeatInteral)
    {
        qDebug("Read ini file failed : Interval, Programme exit!\n");
        return 0;
    }

    if(!read_profile_string("EthName", "ETH", EthDevStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : EthName, Programme exit!\n");
        return 0;
    }


    if(!read_profile_string("WirelessName", "WierLess", WirelessDevStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : WirelessName, Programme exit!\n");
        return 0;
    }

    if(!read_profile_string("GPRSName", "GPRS", GPRSDevStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : GPRSName, Programme exit!\n");
        return 0;
    }

    if(!read_profile_string("Version", "Currentv", VersionStr, 100, "Error", "DevInfo.ini"))
    {
        qDebug("Read ini file failed : Version, Programme exit!\n");
        return 0;
    }

    return 1;
}


/*
 * 开启网络处理线程
 */
int Login_Server()
{
    return 1;
}

/*
 * 开启时间戳处理线程
 */
int GetSet_TimeStamp()
{
    return 1;
}

/*
 * 开启读卡器线程
 */
int Connect_ICReader()
{
    return 1;
}

int SystemStart()
{
    if(!init())
    {
        qDebug("Read ini file failed, Programme exit!\n");
        return 0;
    }

    if(!Connect_ICReader())
    {
        qDebug("Can not Connect to IC Reader and start WR card thread!\n");
        return 0;
    }

    if(!Login_Server())
    {
        qDebug("Can not login to Server and start SD data thread!\n");
        return 0;
    }

    if(!GetSet_TimeStamp())
    {
        qDebug("Can not Start TimeStampThread!\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef EMBEDDED_BOARD
    QWSServer::setCursorVisible( false );
#endif

	//changed.

    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    CardRecharger w;
    if(SystemStart())
    {
        qDebug("Start System successfully!");
    }
    w.setWindowFlags( Qt::FramelessWindowHint );
    w.show();

    return a.exec();
}
