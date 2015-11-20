#include "cardrecharger.h"
#include "ui_cardrecharger.h"

#include <QMessageBox>
#include <QDebug>

#include <pthread.h>

#include "rechargerhandling.h"


CardRecharger* CardRecharger::SelfInstance = NULL;

CardRecharger::CardRecharger(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CardRecharger)
{
    ui->setupUi(this);
    this->AllButtonDisable();
#ifdef CHINESE_OUTPUT
    ui->StatusLabel->setText( tr( "正在校准时钟" ) );
#else
    ui->StatusLabel->setText( tr( "Calibrating Clock" ) );
#endif

    ui->PayWay_Ali->setChecked( true );
    ui->PayWay_WeiXin->setChecked( false );

    this->QRcodeScene = NULL;
    this->QRcodeSceneIsEmpty = true;
    this->qrimage = NULL;
    pthread_mutex_init( &this->QRImageLocker, NULL );

    this->AdvertisementScene = NULL;
    this->AdvertisementImage = NULL;
    this->IsChangeAdvertisement = false;
    pthread_mutex_init( &this->AdvertisementLocker, NULL );

    this->TimerID = this->startTimer( 500 );
    if( this->TimerID == 0 )
    {
        qDebug() << QObject::tr( "Start Timer error!" );
    }

}

CardRecharger::~CardRecharger()
{
    delete ui;
}

void CardRecharger::timerEvent(QTimerEvent *event)
{
    pthread_mutex_lock( &this->QRImageLocker );

    if( this->qrimage == NULL )
    {
        if( QRcodeSceneIsEmpty == false )
        {
            if( this->QRcodeScene != NULL )
            {
                delete this->QRcodeScene;
            }
            this->QRcodeScene = new QGraphicsScene();
            ui->QRCodeImageGraphics->setScene( this->QRcodeScene );
            this->ui->QRCodeImageGraphics->show();

            QRcodeSceneIsEmpty = true;
        }
    }
    else
    {
        if( QRcodeSceneIsEmpty == true )
        {
            if( this->QRcodeScene != NULL )
            {
                delete this->QRcodeScene;
            }
            this->QRcodeScene = new QGraphicsScene();
            ui->QRCodeImageGraphics->setScene( this->QRcodeScene );
            this->QRcodeScene->addPixmap( QPixmap::fromImage( *( this->qrimage ) ) );
            this->ui->QRCodeImageGraphics->show();

            QRcodeSceneIsEmpty = false;
        }
    }

    pthread_mutex_unlock( &this->QRImageLocker );

    pthread_mutex_lock( &this->AdvertisementLocker );

    if( this->IsChangeAdvertisement == true )
    {
        if( this->AdvertisementScene != NULL )
        {
            delete this->AdvertisementScene;
        }
        this->AdvertisementScene = new QGraphicsScene();
        ui->AdvertisementGraphics->setScene( this->AdvertisementScene );
        this->AdvertisementScene->addPixmap( QPixmap::fromImage( *( this->AdvertisementImage ) ) );
        this->ui->AdvertisementGraphics->show();

        this->IsChangeAdvertisement = false;
    }
    else
    {
        if( this->AdvertisementImage == NULL )
        {
            if( this->AdvertisementScene != NULL )
            {
                delete this->AdvertisementScene;
            }
            this->AdvertisementScene = new QGraphicsScene();
            ui->AdvertisementGraphics->setScene( this->AdvertisementScene );
            this->ui->AdvertisementGraphics->show();
        }
    }

    pthread_mutex_unlock( &this->AdvertisementLocker );

}

void CardRecharger::on_Recharge5Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值5元,是否确认？" );
#else
    InfoText += QObject::tr( "5 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 5;
        //RechargerHandling::GetInstance()->RechargeValue = 1;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge10Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值10元,是否确认？" );
#else
    InfoText += QObject::tr( "10 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 10;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge15Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值15元,是否确认？" );
#else
    InfoText += QObject::tr( "15 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 15;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge20Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值20元,是否确认？" );
#else
    InfoText += QObject::tr( "20 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 20;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge30Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值30元,是否确认？" );
#else
    InfoText += QObject::tr( "30 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 30;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge50Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值50元,是否确认？" );
#else
    InfoText += QObject::tr( "50 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 50;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge80Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值80元,是否确认？" );
#else
    InfoText += QObject::tr( "80 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 80;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge100Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值100元,是否确认？" );
#else
    InfoText += QObject::tr( "100 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        RechargerHandling::GetInstance()->RechargeValue = 100;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_Recharge200Button_clicked()
{
    QString InfoText;
    this->AllButtonDisable();

    if( ui->PayWay_Ali->isChecked() == true )
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用支付宝" );
#else
        InfoText = QObject::tr( "Use Ali-Pay," );
#endif
    }
    else
    {
#ifdef CHINESE_OUTPUT
        InfoText = QObject::tr( "您将使用微信支付" );
#else
        InfoText = QObject::tr( "Use WeiXin-Pay," );
#endif
    }

#ifdef CHINESE_OUTPUT
    InfoText += QObject::tr( "充值200元,是否确认？" );
#else
    InfoText += QObject::tr( "200 RMB." );
#endif

#ifdef CHINESE_OUTPUT
    if( QMessageBox::information( this, QObject::tr( "支付提醒" ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#else
    if( QMessageBox::information( this, QObject::tr( "Pay Infomation." ), InfoText, QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
#endif

    {
        //RechargerHandling::GetInstance()->RechargeValue = 200;
        RechargerHandling::GetInstance()->RechargeValue = 1;
        pthread_cond_signal( &( RechargerHandling::GetInstance()->ChargeActionCondition ) );
    }
    else
    {
        this->AllButtonEnable();
    }
}

void CardRecharger::on_GreyRecordButton_clicked()
{
    this->AllButtonDisable();

#if 0
#ifdef CHINESE_OUTPUT
    QMessageBox::information( this, QObject::tr( "支付提醒" ), QObject::tr( "本功能暂未提供！" ), QMessageBox::Ok );
#else
    QMessageBox::information( this, QObject::tr( "Pay Infomation." ), QObject::tr( "Not avilable now!"), QMessageBox::Ok );
#endif
    this->AllButtonEnable();
#endif

    pthread_cond_signal( &( RechargerHandling::GetInstance()->WaitAshRecordCondition ) );
}


void CardRecharger::AllButtonDisable()
{
    ui->Recharge5Button->setEnabled( false );
    ui->Recharge10Button->setEnabled( false );
    ui->Recharge15Button->setEnabled( false );
    ui->Recharge20Button->setEnabled( false );
    ui->Recharge30Button->setEnabled( false );
    ui->Recharge50Button->setEnabled( false );
    ui->Recharge80Button->setEnabled( false );
    ui->Recharge100Button->setEnabled( false );
    ui->Recharge200Button->setEnabled( false );
    ui->GreyRecordButton->setEnabled( false );
    ui->PayWay_Ali->setEnabled( false );
    ui->PayWay_WeiXin->setEnabled( false );
    ui->ReadBalanceButton->setEnabled( false );
}

void CardRecharger::AllButtonEnable()
{
    ui->Recharge5Button->setEnabled( true );
    ui->Recharge10Button->setEnabled( true );
    ui->Recharge15Button->setEnabled( true );
    ui->Recharge20Button->setEnabled( true );
    ui->Recharge30Button->setEnabled( true );
    ui->Recharge50Button->setEnabled( true );
    ui->Recharge80Button->setEnabled( true );
    ui->Recharge100Button->setEnabled( true );
    ui->Recharge200Button->setEnabled( true );
    ui->GreyRecordButton->setEnabled( true );
    ui->PayWay_Ali->setEnabled( true );
    ui->PayWay_WeiXin->setEnabled( true );
    ui->ReadBalanceButton->setEnabled( true );
}


void CardRecharger::SetStatusLabel(QString newstatus)
{
    ui->StatusLabel->setText( newstatus );
}


void CardRecharger::on_PayWay_Ali_clicked()
{
    ui->PayWay_Ali->setChecked( true );
    ui->PayWay_WeiXin->setChecked( false );
    RechargerHandling::GetInstance()->PayWay = RechargerHandling::AliPay;
}

void CardRecharger::on_PayWay_WeiXin_clicked()
{
    ui->PayWay_Ali->setChecked( false );
    ui->PayWay_WeiXin->setChecked( true );
    RechargerHandling::GetInstance()->PayWay = RechargerHandling::WeiXinPay;
}

void CardRecharger::SetQRView(QImage *image)
{
    pthread_mutex_lock( &this->QRImageLocker );

    this->qrimage = image;

    pthread_mutex_unlock( &this->QRImageLocker );
}


void CardRecharger::ResetQRView()
{
    pthread_mutex_lock( &this->QRImageLocker );

    if( this->qrimage != NULL )
    {
        delete this->qrimage;
    }

    this->qrimage = NULL;

    pthread_mutex_unlock( &this->QRImageLocker );
}


void CardRecharger::SetAdvertisementView(QImage *image)
{
    pthread_mutex_lock( &this->AdvertisementLocker );

    this->AdvertisementImage = image;
    this->IsChangeAdvertisement = true;

    pthread_mutex_unlock( &this->AdvertisementLocker );
}

void CardRecharger::ResetAdvertisementView()
{
    pthread_mutex_lock( &this->AdvertisementLocker );

    if( this->AdvertisementImage != NULL )
    {
        delete this->AdvertisementImage;
    }

    this->AdvertisementImage = NULL;

    pthread_mutex_unlock( &this->AdvertisementLocker );
}



void CardRecharger::on_ReadBalanceButton_clicked()
{
    this->AllButtonDisable();

    pthread_cond_signal( &( RechargerHandling::GetInstance()->WaitReadBalanceCondition ) );

}


void CardRecharger::SetBalanceLabel(QString newbalance)
{
    ui->BalanceShowLabel->setText( newbalance );
}
