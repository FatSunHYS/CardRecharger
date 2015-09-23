#include "cardrecharger.h"
#include "ui_cardrecharger.h"

#include <QMessageBox>

#include "httpclient.h"
#include "messagequeuenode.h"

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
}

CardRecharger::~CardRecharger()
{
	delete ui;
}

void CardRecharger::on_Recharge5Button_clicked()
{

	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "5" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge10Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "10" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge15Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "15" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge20Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "20" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge30Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "30" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge50Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "50" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge80Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "80" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge100Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "100" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_Recharge200Button_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "200" );
	emit this->AllButtonEnable();
}

void CardRecharger::on_GreyRecordButton_clicked()
{
	emit this->AllButtonDisable();
    QMessageBox::information( this, "a", "grey" );
	emit this->AllButtonEnable();
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
}


void CardRecharger::SetStatusLabel(QString newstatus)
{
	ui->StatusLabel->setText( newstatus );
}





