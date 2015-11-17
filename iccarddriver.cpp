#include "iccarddriver.h"

#include <QObject>
#include <QtDebug>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



ICCardDriver* ICCardDriver::PrivateICCardDriverInstance = NULL;
const int ICCardDriver::speed_arr[ 16 ] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
const int ICCardDriver::name_arr[] = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200, 9600, 4800, 2400, 1200, 300 };




ICCardDriver::ICCardDriver()
{

}

ICCardDriver::~ICCardDriver()
{

}


ICCardDriver* ICCardDriver::GetICCardDirverInstance()
{
    if( ICCardDriver::PrivateICCardDriverInstance == NULL )
    {
        ICCardDriver::PrivateICCardDriverInstance = new ICCardDriver();
    }

    return ICCardDriver::PrivateICCardDriverInstance;
}

int ICCardDriver::set_speed( int speed )
{
    int i;
    int status;
    struct termios Opt;

    tcgetattr( this->COM_FD, &Opt);

    for ( i= 0; i < ( int )( sizeof( ICCardDriver::speed_arr ) / sizeof( int ) ); i++)
    {

        if (speed == ICCardDriver::name_arr[i])
        {
            tcflush( this->COM_FD, TCIOFLUSH);
            cfsetispeed(&Opt, ICCardDriver::speed_arr[i]);
            cfsetospeed(&Opt, ICCardDriver::speed_arr[i]);
            status = tcsetattr( this->COM_FD, TCSANOW, &Opt);
            if (status != 0)
            {
                qDebug() << QObject::tr( "Failed: Set speed error!\n" );
                return status;
            }
            qDebug() << QObject::tr( "SUCCESS: Set speed ok!\n" );
            tcflush( this->COM_FD,TCIOFLUSH);
            return 0;
        }
    }
    qDebug() << QObject::tr( "Failed: Speed out of range!\n" );
    return 1000;
}


int ICCardDriver::set_parity(int databits, int stopbits, int parity)
{
    struct termios options;

    if(tcgetattr( this->COM_FD,&options) != 0)
    {
        qDebug() << QObject::tr( "Failed: Get parity error!\n" );
        return 1;
    }
    options.c_cflag &= ~CSIZE;

    switch (databits) /*set data bit*/
    {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        qDebug() << QObject::tr( "Failed: Unsupported data sizen\n" );
        return 2;
    }

    switch (parity)
    {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB; /* Clear parity enable */
        options.c_iflag &= ~INPCK; /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB); /* odd parity check*/
        options.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB; /* Enable parity */
        options.c_cflag &= ~PARODD; /* even parity check*/
        options.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case 'S':
    case 's': /*as no parity*/
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        qDebug() << QObject::tr( "Failed: Unsupported parity\n" );
        return 3;
    }

    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        qDebug() << QObject::tr( "Failed: Unsupported stop bits\n" );
        return 3;
    }

    /* Set input parity option */
    if ((parity != 'n')&&(parity != 'N'))
        options.c_iflag |= INPCK;
    /*
    options.c_lflag &=~(ICANON|ECHO|ECHOE|ISIG);
    options.c_oflag &= ~OPOST;
    options.c_oflag &= ~(INLCR|IGNCR|ICRNL);
    options.c_oflag &=~(ONLCR|OCRNL);
*/
    options.c_cflag |= CLOCAL | CREAD;
    options.c_lflag &=~(ICANON|ECHO|ECHOE|ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    tcflush( this->COM_FD,TCIFLUSH);

    options.c_cc[VTIME] = 5; /* set timeout 5 seconds*/
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

    if (tcsetattr( this->COM_FD,TCSANOW,&options) != 0)
    {
        qDebug() << QObject::tr( "Failed: Set Comm options error!\n" );
        return 4;
    }

    qDebug() << QObject::tr( "SUCCESS: Set Comm options ok!\n" );
    return 0;
}


int ICCardDriver::set_timeout()
{
    FD_ZERO( &this->fds);
    FD_SET( this->COM_FD, &this->fds );

    this->tv.tv_sec = 5;
    this->tv.tv_usec = 0;

    if(select( this->COM_FD + 1, &this->fds, NULL, NULL, &this->tv) <= 0)
    {
        qDebug() << QObject::tr( "Failed: set timeout error!\n" );
        return 1;
    }

    qDebug() << QObject::tr( "SUCCESS: set timeout ok!\n" );
    return 0;
}

int ICCardDriver::pacarddll_arm(unsigned char* device,
                                unsigned char sendlen,
                                unsigned char* sendbuf,
                                unsigned char* reclen,
                                unsigned char* recbuf,
                                unsigned int Delayms)
{
    //DCB dcb; /* windows com properity struct */
    //COMMTIMEOUTS timeout={0,1,1000,1,1000}; /* windowos com timeout properity */
    unsigned char buf[30];
    unsigned char i,tmplen;
    char com_addr[20] = "/dev/ttySAC0";
    unsigned char jym;
    unsigned char head = 0xa7;
    int nwrite, nread;

    memset(buf, 0, 20);
    memcpy(com_addr, device, 12);
    //sleep( 100 );
    usleep( Delayms );

    //begin to set COM properity
    this->COM_FD = open(com_addr, O_RDWR | O_NOCTTY);
    if(-1 == this->COM_FD)
    {
        qDebug() << QObject::tr( "Failed: Open Comm Error!\n" );
        return 100;
    }

    /*	if(0 != set_timeout(COM_FD))
    {
        if(DEBUG_INFO)
            printf("Failed: Set timeout error!\n");
        return 1;
    }*/

    if(0 != this->set_speed( 57600 ) )
    {
        qDebug() << QObject::tr( "Failed: Set speed error!\n" );
        return 2;
    }

    if(0 != this->set_parity( 8, 1, 'N' ) )
    {
        qDebug() << QObject::tr( "Failed: Set parity error!\n" );
        return 3;
    }

    //	sleep(Delayms);
    //end to set COM propertiy

    //begin to send data
    buf[0] = head;
    nwrite = write( this->COM_FD, buf, 1);
    if(1 != nwrite)
    {
        qDebug() << QObject::tr( "Failed: Send machineno error!\n" );
        close( this->COM_FD );
        return 101;
    }

    qDebug() << QObject::tr( "SUCCESS: Send machineno ok!\n" );
    jym = buf[0];
    usleep(1*1000);

    buf[0] = sendlen;
    nwrite = write( this->COM_FD, buf, 1);
    if(1 != nwrite)
    {
        qDebug() << QObject::tr( "Failed: Send data length error!\n" );
        close( this->COM_FD );
        return 101;
    }

    qDebug() << QObject::tr( "SUCCESS: Send data length ok!\n" );
    jym = jym ^ buf[0];
    usleep(1*1000);




    for(i = 0; i <sendlen; i++)
    {
        buf[0] = sendbuf[i];

        nwrite = write( this->COM_FD, buf, 1);
        if(1 != nwrite)
        {
            qDebug() << QObject::tr( "Failed: Send data error!\n" );
            close( this->COM_FD );
            return 101;
        }

        qDebug( "\t success: send data sendbuf[%d] ok, sendbuf[%d] is: 0x%02x\n", i, i, buf[0]);
        jym = jym ^ buf[0];
        //		sleep(Delayms);
    }

    buf[0] = jym;
    nwrite = write(this->COM_FD, buf, 1);
    if(1 != nwrite)
    {
        qDebug("Failed: Send parity error!\n");
        close( this->COM_FD );
        return 101;
    }
    qDebug("SUCCESS: Send parity ok!\n");
    usleep(Delayms*1000);
    //end to send data

    //begin to receive data
    nread = read( this->COM_FD ,buf,1);
    qDebug( "test read length: %d\n", nread);
    if(1 != nread)
    {
        qDebug("Failed: Receive data head error!\n");
        close( this->COM_FD );
        return 102;
    }
    else
    {
        qDebug("test head char ok\n");
    }

    if(buf[0] != head)
    {
        qDebug("Failed: Receive wrong data head error!\n");
        close( this->COM_FD );
        return 103;
    }
    qDebug( "SUCCESS: Receive data head ok! head char is ; 0x%02x\n", buf[0]);
    jym = head;

    nread = read( this->COM_FD, buf, 1);
    if(1 != nread)
    {
        qDebug("Failed: Receive data length error!\n");
        close( this->COM_FD );
        return 104;
    }

    qDebug( "SUCESS: Receive data length ok!, length is 0x%02x.\n", buf[0]);
    tmplen = buf[0];
    jym = jym^buf[0];



    for(i = 0; i < tmplen; i++)
    {
        nread = read( this->COM_FD, buf, 1);
        if(1 != nread)
        {
            qDebug("Failed: Receive data error!\n");
            close( this->COM_FD );
            return 105;
        }

        qDebug( "\t success: Receive data recbuf[%d] ok!, recbuf[%d] is 0x%02x\n", i, i, buf[0]);
        recbuf[i] = buf[0];
        jym = jym^buf[0];
    }

    nread = read( this->COM_FD, buf, 1);
    if(1 != nread)
    {
        qDebug( "Failed: Receive parity error!\n");
        close( this->COM_FD );
        return 106;
    }

    qDebug( "\t success: Receive parity ! parity is 0x%02x\n", buf[0]);
    qDebug( "\t success: jym parity ! parity is 0x%02x\n", jym);
    if(jym != buf[0])
    {
        qDebug("Failed: Parity  check error!\n");
        close( this->COM_FD );
        return 107;
    }

    qDebug( "SUCCESS: Parity ok!\n");
    reclen[0] = tmplen;
    close( this->COM_FD );
    //end to receive data

    return 0;
}



int ICCardDriver::readwatercard_arm(unsigned char *device,
                                    unsigned char keymode,
                                    unsigned char secnum,
                                    unsigned char *key,
                                    unsigned char *kh,
                                    unsigned char *balance,
                                    unsigned char *dwmm,
                                    unsigned char *daytime,
                                    unsigned char *cardtype,
                                    unsigned int Delayms)
{
    unsigned char tmpsendbuf[30],tmpreclen,recbuf[30];
    unsigned char i;
    unsigned int  status;

    tmpsendbuf[0]=secnum;
    tmpsendbuf[1]=0x60;
    tmpsendbuf[2]=50;
    tmpsendbuf[3]=keymode;
    for(i=0;i<6;i++)
    {
        tmpsendbuf[4+i]=key[i];
    }
    status=this->pacarddll_arm(device,10,tmpsendbuf,&tmpreclen,recbuf,Delayms);
    qDebug() << QObject::tr( "Pacarddll_arm status = " ) << status;
    dwmm[0]=recbuf[17];
    dwmm[1]=recbuf[18];

    kh[0]=0;
    kh[1]=recbuf[5];
    kh[2]=recbuf[6];
    kh[3]=recbuf[7];

    balance[0]= recbuf[8];
    balance[1]= recbuf[9];
    balance[2]= recbuf[10];
    balance[3]= recbuf[11];

    daytime[0]=recbuf[12];
    daytime[1]=recbuf[13];
    daytime[2]=recbuf[14];
    daytime[3]=recbuf[15];
    daytime[4]=recbuf[16];

    cardtype[0]=recbuf[4];


    if(status)
    {
        return(status);
    }
    else
    {
        return(recbuf[2]);
    }
    return(0);
}

int ICCardDriver::beep_arm(unsigned char *device, unsigned char time)
{
    unsigned char tmpsendbuf[30],tmpreclen,tmprecbuf[30];
    unsigned int  status;

    tmpsendbuf[0]=0x00;
    tmpsendbuf[1]=time;
    tmpsendbuf[2]=0x05;
    tmprecbuf[2]=0x80;
    status=this->pacarddll_arm(device,3,tmpsendbuf,&tmpreclen,tmprecbuf,1);
    if(status)
    {
        return(status);
    }
    else
    {
        //return(tmprecbuf[2]);
        return(0);
    }
}


int ICCardDriver::writecard(unsigned char *device,		//串口号
                            unsigned char keymode,		//密码版本
                            unsigned char secnum,		//使用卡扇区
                            unsigned char *key,			//密码因子
                            unsigned char *kh,			//卡号
                            unsigned char *balance,		//金额
                            unsigned char *dwmm,		//单位密码
                            unsigned char *daytime,		//日期
                            unsigned char *cardtype,	//卡类型
                            unsigned char mode,			//写卡模式，0：表示第一次发卡；1：表示充值；其他无效
                            unsigned int Delayms)		//延时参数
{
    unsigned char tmpsendbuf[30],tmpreclen,tmprecbuf[30];
    unsigned char i;
    unsigned int  status;

    tmpsendbuf[0]=secnum;
    tmpsendbuf[1]=0x60;
    tmpsendbuf[2]=51;
    tmpsendbuf[3]=keymode;
    for(i=0;i<6;i++)
    {
        tmpsendbuf[4+i]=key[i];
    }
    //卡类
    tmpsendbuf[10]=cardtype[0];

    //卡号
    tmpsendbuf[11]=kh[1];
    tmpsendbuf[12]=kh[2];
    tmpsendbuf[13]=kh[3];

    //金额
    tmpsendbuf[14]=balance[0];
    tmpsendbuf[15]=balance[1];
    tmpsendbuf[16]=balance[2];
    tmpsendbuf[17]=balance[3];

    //写卡模式
    tmpsendbuf[18]=mode;

    //单位密码
    tmpsendbuf[19]=dwmm[0];
    tmpsendbuf[20]=dwmm[1];

    //年月日时分
    tmpsendbuf[21]=daytime[0];
    tmpsendbuf[22]=daytime[1];
    tmpsendbuf[23]=daytime[2];
    tmpsendbuf[24]=daytime[3];
    tmpsendbuf[25]=daytime[4];

    status= this->pacarddll_arm(device,26,tmpsendbuf,&tmpreclen,tmprecbuf,Delayms);
    qDebug() << QObject::tr( "Pacarddll_arm status = " ) << status;
    if(status)
    {
        return(status);
    }
#if 1
    else
    {
        return(tmprecbuf[2]);
    }
#endif

    return 0;
}





int ICCardDriver::readserialnumber( unsigned char *comdevice, unsigned char secnum, unsigned char* CARDPassword, unsigned int Delayms, unsigned int& serialnum )
{
    int status;
    unsigned char sendbuf[26];
    unsigned char sendbuflen = 26;
    unsigned int senddelay = Delayms;
    unsigned char recbuf[140];
    unsigned char recbuflen;
    unsigned char pblock = (secnum+1)*4+2;
    unsigned char pmimamode = 0x60;
    unsigned char pcommand = 0x06;
    unsigned char pkeymode = 0x03;
    unsigned char pkey[6];
    memcpy(pkey,CARDPassword,6);

    //ZeroMemory(sendbuf,26);
    memset( sendbuf, 0, 26 );
    sendbuf[0] = pblock;//扇区×4+块号
    sendbuf[1] = pmimamode;//固定为0x60
    sendbuf[2] = pcommand;//命令字：1为读，2为写
    sendbuf[3] = pkeymode;
    sendbuf[4] = pkey[0];
    sendbuf[5] = pkey[1];
    sendbuf[6] = pkey[2];
    sendbuf[7] = pkey[3];
    sendbuf[8] = pkey[4];
    sendbuf[9] = pkey[5];
    status = this->pacarddll_arm(comdevice,sendbuflen,sendbuf,&recbuflen,recbuf,senddelay);
    if(!status)
    {
        if(0x00 == recbuf[2])
        {
            //unsigned int serialnum;
            memcpy(&serialnum, &recbuf[3], 4);
            //cout<<"serialnum of card is : "<<serialnum<<endl;
            qDebug() << QObject::tr( "serialnum of card is :" ) << serialnum;
            //return serialnum;
        }
        else
        {
            //cout<<"error in get serial number of card"<<endl;
            qDebug() << QObject::tr( "error in get serial number of card" );
        }
    }

    return status;

}



int ICCardDriver::readconsumptionrecord(unsigned char *comdevice, unsigned char secnum, unsigned char *CARDPassword, unsigned int Delayms, double &record)
{
    int status;
    unsigned char sendbuf[26];
    unsigned char sendbuflen = 26;
    unsigned int senddelay = Delayms;
    unsigned char recbuf[140];
    unsigned char recbuflen;
    unsigned char pblock = (secnum+1)*4+2;
    unsigned char pmimamode = 0x60;
    unsigned char pcommand = 0x01;
    unsigned char pkeymode = 0x03;
    unsigned char pkey[6];
    memcpy(pkey,CARDPassword,6);
    memset( sendbuf, 0, 26 );

    sendbuf[0] = pblock;//扇区×4+块号
    sendbuf[1] = pmimamode;//固定为0x60
    sendbuf[2] = pcommand;//命令字：1为读，2为写
    sendbuf[3] = pkeymode;
    sendbuf[4] = pkey[0];
    sendbuf[5] = pkey[1];
    sendbuf[6] = pkey[2];
    sendbuf[7] = pkey[3];
    sendbuf[8] = pkey[4];
    sendbuf[9] = pkey[5];

    status = this->pacarddll_arm(comdevice,sendbuflen,sendbuf,&recbuflen,recbuf,senddelay);

    if((0 == status)&&(0 == recbuf[2]))
    {
        //入库
        unsigned char precbuf[16];
        memset( precbuf, 0, 16 );
        memcpy(precbuf, &recbuf[10], 16);
        if((0x55 == precbuf[14])&&(0xaa == precbuf[15]))
        {
//            double xf1;
//            double xf2;
//            int jqh;
//            int cjkh;
            int xf1_t = 0;
            int xf2_t = 0;
            for( int i = 0; i < 4; ++i )
            {
                xf1_t = ( xf1_t << 8 ) + precbuf[ 3 - i ];
                xf2_t = ( xf2_t << 8 ) + precbuf[ 7 - i ];
            }

//            xf1 = ((double)precbuf[0]+(double)precbuf[1]*256+
//                    (double)precbuf[2]*256*256+(double)precbuf[3]*256*256*256)/100;
//            xf2 = ((double)(precbuf[4]^0xff)+(double)(precbuf[5]^0xff)*256+
//                    (double)(precbuf[6]^0xff)*256*256+(double)(precbuf[7]^0xff)*256*256*256)/100;

            if(xf1_t == xf2_t)
            {
                record = ( double )xf1_t;
                record /= 100;
//                //入库操作
//                ReChargeADOQ->Close();
//                String tjsqlstr = "insert into WTJ values(:JH,:CJKH,:XFZE,:SCSJ)";
//                ReChargeADOQ->SQL->Clear();
//                ReChargeADOQ->SQL->Add(tjsqlstr);
//                ReChargeADOQ->Parameters->ParamByName("JH")->Value = jqh;//机器号
//                ReChargeADOQ->Parameters->ParamByName("CJKH")->Value = cjkh;//采集卡号
//                ReChargeADOQ->Parameters->ParamByName("XFZE")->Value = xf1;//消费金额
//                ReChargeADOQ->Parameters->ParamByName("SCSJ")->Value = DateTimeToStr(Now());//采集时间
//                ReChargeADOQ->ExecSQL();
                return 0;
            }
        }
    }

    return -1;

}























