#ifndef UDPSENDTHREAD_H
#define UDPSENDTHREAD_H

#include <QObject>
#include <winsock.h>


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          UDPSendThread
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       UDPSendThread(SOCKET sock, struct sockaddr_in dest) : socket(sock), destination(dest)
--                   void send(char * buf, int length);
--                   
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  NOTES:           This class is a thread for sending out UDP data.
------------------------------------------------------------------------------------------*/
class UDPSendThread : public QObject
{
    Q_OBJECT
public:
    UDPSendThread(SOCKET sock, struct sockaddr_in dest) : socket(sock), destination(dest){}

public slots:
    void send(char * buf, int length);

private:
    SOCKET socket;
    struct sockaddr_in destination;
};

#endif // UDPSENDTHREAD_H
