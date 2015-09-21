#include "cardrecharger.h"
#include <QApplication>
#include <QTextCodec>

#ifdef EMBEDDED_BOARD
#include <QWSServer>
#endif

#include "messagehandling.h"

MessageHandling* DebugMessageHandlingInstance;

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

	DebugMessageHandlingInstance = MessageHandling::GetInstance();
	MessageHandling::GetInstance()->start();

    CardRecharger w;
    w.setWindowFlags( Qt::FramelessWindowHint );
    w.show();



	return a.exec();
}
