#include "cardrecharger.h"
#include "iccarddriver.h"
#include "httpclient.h"

#ifndef GLOBALPARAMETER
#define GLOBALPARAMETER

char ClientIDStr[100];
char ClientPWStr[100];
char ServerBaseURLStr[100];
char AdvtServerURLStr[100];
char ICReadDevStr[100];
int  HeartBeatInteral;
char EthDevStr[100];
char WirelessDevStr[100];
char GPRSDevStr[100];
char VersionStr[010];

unsigned char CardSecurityCode[6];
unsigned char CardSecurityMode;
unsigned char CardSectorNum;

ICCardDriver m_ICCardDriver;
HttpClient   m_HttpClient;
CardRecharger* P_Cardrecharger;

#endif // GLOBALPARAMETER

