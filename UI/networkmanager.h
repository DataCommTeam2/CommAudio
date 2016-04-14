#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "circularbuffer.h"
#include "udpsendthread.h"

#define UDP_PORT        7000
#define TCP_PORT        8000
#define DATA_BUFSIZE    8192
#define MAXLEN          256
#define MAX_BLOCKS      100

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    //NetworkManager();
    bool startNetwork();
    bool connectToPeer(const char * hostname, int port);
    void cleanUp();
    bool setupUDPforP2P(const char * hostname, int port);
    int createMulticastServerSocket(const char * IP, int port);
    bool createMulticastClientSocket(const char* serverAddr, int port);
    void startUDPReceiver(CircularBuffer *);
    void startTCPReceiver(int port);
    void sendP2P(char * buf, int length);
    bool createTCPSocket();

    static CircularBuffer * incBuffer;
    static CircularBuffer * tcpBuffer;
    static SOCKET acceptSocket;
    static bool tcpConnected;
private:
    UDPSendThread * udpSender;

public slots:
    void sendMulticast(char * buf, int length);
    void sendViaTCP(char * sbuf, int length);

signals:
    void sendData(char *, int);
    void stopSender();
};

#endif // NETWORKMANAGER_H
