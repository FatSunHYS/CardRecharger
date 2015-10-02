#ifndef CARDRECHARGER_H
#define CARDRECHARGER_H

#include <QDialog>
#include <QImage>

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
	void SetQRLabel(QImage &image );


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

	void on_UpdateButton_clicked();

private:
	Ui::CardRecharger *ui;

};

#endif // CARDRECHARGER_H
