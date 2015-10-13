#ifndef ICCARDDRIVER_H
#define ICCARDDRIVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>


class ICCardDriver
{
public:
    static ICCardDriver* GetICCardDirverInstance();
    int readwatercard_arm(unsigned char *device,
                          unsigned char keymode,
                          unsigned char secnum,
                          unsigned char *key,
                          unsigned char *kh,
                          unsigned char *balance,
                          unsigned char *dwmm,
                          unsigned char *daytime,
                          unsigned char *cardtype,
                          unsigned int Delayms);
    int beep_arm(unsigned char* device, unsigned char time);
    int writecard( unsigned char* device,
                   unsigned char keymode,
                   unsigned char secnum,
                   unsigned char* key,
                   unsigned char* kh,
                   unsigned char* balance,
                   unsigned char* dwmm,
                   unsigned char* daytime,
                   unsigned char* cardtype,
                   unsigned char mode,
                   unsigned int Delayms);
    int readserialnumber(unsigned char *comdevice,
                         unsigned char secnum,
                         unsigned char *CARDPassword,
                         unsigned int Delayms,
                         unsigned int &serialnum);


private:

    static const int speed_arr[ 16 ];
    static const int name_arr[ 16 ];
    static ICCardDriver* PrivateICCardDriverInstance;

    int COM_FD;
    fd_set fds;
    struct timeval tv;

    ICCardDriver();
    ~ICCardDriver();

    int set_speed( int speed );
    int set_parity( int databits, int stopbits, int parity );
    int set_timeout();
    int pacarddll_arm(unsigned char* device,
                      unsigned char sendlen,
                      unsigned char* sendbuf,
                      unsigned char* reclen,
                      unsigned char* recbuf,
                      unsigned int Delayms);


};




#endif // ICCARDDRIVER_H
