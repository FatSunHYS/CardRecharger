#ifndef CARDRECHARGER_H
#define CARDRECHARGER_H

#include <QDialog>

namespace Ui {
	class CardRecharger;
}

class CardRecharger : public QDialog
{
	Q_OBJECT

public:
	explicit CardRecharger(QWidget *parent = 0);
	~CardRecharger();

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

    void AllButtonEnable();
    void AllButtonDisable();

private:
	Ui::CardRecharger *ui;
};

#endif // CARDRECHARGER_H
