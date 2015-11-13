#ifndef CARDRECHARGER_H
#define CARDRECHARGER_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QImage>

#include <pthread.h>

namespace Ui {
	class CardRecharger;
}

class CardRecharger : public QDialog
{
	Q_OBJECT

public:
	static CardRecharger* SelfInstance;
	QString CardRechargerClientID;
	QString CardRechargerClientPassword;
	QString CardRechargerServerURL;
	QString DeviceSerials;

	explicit CardRecharger(QWidget *parent = 0);
	~CardRecharger();

	void AllButtonEnable();
	void AllButtonDisable();
	void SetStatusLabel( QString newstatus );
	void SetQRView(QImage *image );
	void ResetQRView();


private slots:
    void on_Recharge5Button_clicked();
    void on_Recharge10Button_clicked();
    void on_Recharge15Button_clicked();
    void on_Recharge20Button_clicked();
    void on_Recharge30Button_clicked();
    void on_Recharge50Button_clicked();
    void on_Recharge80Button_clicked();
    void on_Recharge100Button_clicked();
    void on_Recharge200Button_clicked();
    void on_GreyRecordButton_clicked();
	void on_PayWay_Ali_clicked();
	void on_PayWay_WeiXin_clicked();

protected:
	int TimerID;

	void timerEvent( QTimerEvent *event );

private:
	Ui::CardRecharger *ui;

	pthread_mutex_t QRImageLocker;
	QGraphicsScene* QRcodeScene;
	bool QRcodeSceneIsEmpty;
	QImage *qrimage;



};

#endif // CARDRECHARGER_H
