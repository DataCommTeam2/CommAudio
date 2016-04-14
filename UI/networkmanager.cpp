#include <WinSock2.h>
#include <WS2tcpip.h>
#include <errno.h>
#include <QThread>
#include "networkmanager.h"

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    DWORD Timeout;

} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

SOCKET udpSocket, udpSendSocket, udpRecvSocket;
SOCKET tcpSocket, tcpSendSocket;
struct sockaddr_in udpPeer, tcpPeer, stDstAddr, stSrcAddr;
BOOL serverRunning = false;
BOOL tcpServerRunning = false;
int uPort, tPort;
HANDLE hWriteFile, hServerLogFile;
WSAEVENT udpEvent, tcpEvent;
struct ip_mreq stMreq;
char incFilename[256];

CircularBuffer * NetworkManager::incBuffer;
CircularBuffer * NetworkManager::tcpBuffer= NULL;
SOCKET NetworkManager::acceptSocket;
bool NetworkManager::tcpConnected = false;

DWORD WINAPI udpThread(LPVOID lpParameter);
DWORD WINAPI startUDPServer(LPVOID n);
void CALLBACK udpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
DWORD WINAPI tcpThread(LPVOID lpParameter);
DWORD WINAPI startTCPServer(LPVOID n);
void CALLBACK tcpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
void sendViaTCP(char *, int);
void sendViaUDP(const char *);
bool connectP2P(const char * hostname, int port);
DWORD WINAPI tcpSelectThread(LPVOID lpParameter);
DWORD WINAPI startConnectedTCPReceive(LPVOID n);

bool NetworkManager::startNetwork()
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2,2);
    int error;

    error = WSAStartup(wVersionRequested, &wsaData);
    if (error != 0)
    {
        //display error
        return false;
    }

    return true;
}

int NetworkManager::createMulticastServerSocket(const char * IP, int port)
{
    SOCKADDR_IN stLclAddr;
    BOOL  fFlag = FALSE;

    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        //display error
        return 0;
    }
    stLclAddr.sin_family = AF_INET;
    stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stLclAddr.sin_port = 0;
    if (bind(udpSocket, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr)) == SOCKET_ERROR)
    {
        //display error
        return 0;
    }

    stMreq.imr_multiaddr.s_addr = inet_addr(IP);
    stMreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        //display error
        return 0;
    }

    /* Disable loopback */
    if (setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag)) == SOCKET_ERROR)
    {
        //display error
        return 0;
    }

    /* Assign our destination address */
    stDstAddr.sin_family = AF_INET;
    stDstAddr.sin_addr.s_addr = inet_addr("234.7.8.9");
    stDstAddr.sin_port = htons(port);

    if (udpSender == NULL)
    {
        QThread * sendThread = new QThread();
        udpSender = new UDPSendThread(udpSocket, stDstAddr);
        udpSender->moveToThread(sendThread);

        connect(this, SIGNAL(sendData(char*, int)), udpSender, SLOT(send(char*, int)));
        connect(this, SIGNAL(stopSender()), sendThread, SLOT(quit()));
        sendThread->start();
    }
    return 1;
}

bool NetworkManager::createMulticastClientSocket(const char * serverAddr, int port)
{
    struct ip_mreq stMreq;
    SOCKADDR_IN stLclAddr;
    BOOL  fFlag = TRUE;

    if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    stLclAddr.sin_family = AF_INET;
    stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stLclAddr.sin_port = htons(port);
    if (bind(udpSocket, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    //stMreq.imr_multiaddr.s_addr = inet_addr(serverAddr);
    stMreq.imr_multiaddr.s_addr = inet_addr("234.7.8.9");
    stMreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    NetworkManager::incBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);
    return true;
}
bool NetworkManager::createTCPSocket()
{
    BOOL  fFlag = TRUE;
    if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }
    NetworkManager::tcpBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);
    return true;
}

bool NetworkManager::connectToPeer(const char * hostname, int port)
{
    closesocket(tcpSocket);
    tcpServerRunning = false;
    tcpConnected = false;
    createTCPSocket();
    NetworkManager::acceptSocket = tcpSocket;
    bool success = connectP2P(hostname, port);
    return success;
}

bool connectP2P(const char * hostname, int port)
{
    struct hostent	*hp;
    HANDLE tcpHandle;
    DWORD tcpThreadId;

    // Store server's information
    memset((char *)&tcpPeer, 0, sizeof(tcpPeer));
    tcpPeer.sin_family = AF_INET;
    tcpPeer.sin_port = htons(port);

    if ((hp = gethostbyname(hostname)) == NULL)
    {
        //writeToScreen("Can't get server's IP address");
        return false;
    }

    memcpy((char *)&tcpPeer.sin_addr, hp->h_addr, hp->h_length);
    if (connect(NetworkManager::acceptSocket, (struct sockaddr *)&tcpPeer, sizeof(tcpPeer)) == -1)
    {
        //writeToScreen("Can't connect to server");
        return false;
    }

    NetworkManager::tcpConnected = true;
    if ((tcpHandle = CreateThread(NULL, 0, startConnectedTCPReceive, 0, 0, &tcpThreadId)) == NULL)
    {
        //display error
        return false;
    }
    tcpServerRunning = true;
    return true;
}

DWORD WINAPI startConnectedTCPReceive(LPVOID n)
{
    DWORD threadId;
    HANDLE threadHandle;
    fd_set fds;
    struct timeval tv;
    int selectRet, error = 0;

    if ((tcpEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        //writeToScreen("WSACreateEvent() failed");
    }

    if ((threadHandle = CreateThread(NULL, 0, tcpSelectThread, (LPVOID)tcpEvent, 0, &threadId)) == NULL)
    {
        //writeToScreen("CreateThread() failed");
    }

    FD_ZERO(&fds);
    FD_SET(tcpSocket, &fds);

    tv.tv_sec = 3600;
    tv.tv_usec = 0;

    while (true)
    {
        selectRet = select(tcpSocket, &fds, NULL, NULL, &tv);
        if (serverRunning)
        {
            if (selectRet == SOCKET_ERROR)
            {
                if ((error = WSAGetLastError()) != WSA_IO_PENDING)
                {
                    break;
                }
            }
            else if (selectRet == 0)
            {
                tv.tv_sec = 3600;
                FD_ZERO(&fds);
                FD_SET(tcpSocket, &fds);

                if (WSASetEvent(tcpEvent) == FALSE)
                {
                    //writeToScreen("Resetting event failed");
                }
            }
            else {
                if (WSASetEvent(tcpEvent) == FALSE)
                {
                    //writeToScreen("Resetting event failed");
                }
                //tv.tv_sec = 1;
            }
        }
        else {
            break;
        }

    }
    //return 0;
    ExitThread(0);
}

void NetworkManager::cleanUp()
{
    closesocket(udpSocket);
    closesocket(tcpSocket);
    WSACleanup();
}

bool NetworkManager::setupUDPforP2P(const char * hostname, int port)
{
    struct hostent	*hp;
    struct sockaddr_in peer;

    if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    memset((char *)&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpSocket, (struct sockaddr *)&peer, sizeof(peer)) == SOCKET_ERROR)
    {
        //writeToScreen("Can't bind name to socket");

        return false;
    }

    memset((char *)&udpPeer, 0, sizeof(udpPeer));
    udpPeer.sin_family = AF_INET;
    udpPeer.sin_port = htons(port);
    if ((hp = gethostbyname(hostname)) == NULL)
    {
        return false;
    }

    memcpy((char*) &udpPeer.sin_addr, hp->h_addr, hp->h_length);

    NetworkManager::incBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);

    if (udpSender == NULL)
    {
        QThread * sendThread = new QThread();
        udpSender = new UDPSendThread(udpSocket, udpPeer);
        udpSender->moveToThread(sendThread);

        connect(this, SIGNAL(sendData(char*,int)), udpSender, SLOT(send(char*,int)));
        connect(this, SIGNAL(stopSender()), sendThread, SLOT(quit()));
        sendThread->start();
    }
    return true;
}

void NetworkManager::sendMulticast(char * buf, int length)
{
    emit sendData(buf, length);
}

void NetworkManager::sendP2P(char * buf, int length)
{
    /*if (sendto(udpSocket, buf, length, 0, (struct sockaddr*)&udpPeer, sizeof(udpPeer)) == -1)
    {
        //sprintf(message, "error: %d", WSAGetLastError());
        //writeToScreen(message);
    }*/
    emit sendData(buf, length);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: sendViaTCP
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void sendViaTCP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--
--	PARAMETERS:	char *hostname - hostname of server
--				int port - port of server
--				int packetSize - size of packet to send
--				int repetition - number of packets to send
--				HANDLE file - handle for file for data to read from
--				HANDLE logFile - handle for client log file.
--
--	RETURNS:	void
--
--	NOTES:
--	This function handles the process of sending packets to the server using TCP.
--  First it creates a TCP socket, then it tries to establish a connection with
--  the server. If the connection was established successfully, it goes in a loop
--  where it gets the data from the getData method and sends the packet using the
--  WSASend method until all the packets to be sent has been sent. Finally it prints
--  out the details of the data transfer to the screen before closing the socket.
--
---------------------------------------------------------------------------------*/
void NetworkManager::sendViaTCP(char * sbuf, int length)
{
    // transmit data
    if (send(acceptSocket, sbuf, length, 0) == -1)
    {
        //sprintf(message, "error: %d", WSAGetLastError());
        //writeToScreen(message);
    }
}

/*---------------------------------------------------------------------------------
--	FUNCTION: sendViaUDP
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void sendViaUDP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--
--	PARAMETERS:	char *hostname - hostname of server
--				int port - port of server
--				int packetSize - size of packet to send
--				int repetition - number of packets to send
--				HANDLE file - handle for file for data to read from
--				HANDLE logFile - handle for client log file.
--
--	RETURNS:	void
--
--	NOTES:
--	This function handles the process of sending packets to the server using UDP.
--  First it creates a UDP socket, then in a loop it gets the data from the getData
--  method and sends the packet using the WSASendTo method until all the packets
--  has been sent. Finally it prints out the details of the data transfer to the
--	 screen before closing the socket.
--
---------------------------------------------------------------------------------*/
void sendViaUDP(const char * sbuf)
{
    //char *sbuf;

    int server_len = sizeof(udpPeer);

    //sbuf = (char*)malloc(DATA_BUFSIZE);

    // transmit data
    if (sendto(udpSocket, sbuf, strlen(sbuf), 0, (struct sockaddr *)&udpPeer, server_len) == -1)
    {
        //sprintf(message, "error: %d", WSAGetLastError());
        //writeToScreen(message);
    }
}

void NetworkManager::startUDPReceiver(CircularBuffer * buffer)
{
    if(buffer != NULL)
    {
        delete NetworkManager::incBuffer;
        NetworkManager::incBuffer = buffer;
    }
    HANDLE udpHandle;
    DWORD udpThreadId;

    if ((udpHandle = CreateThread(NULL, 0, startUDPServer, (LPVOID)0, 0, &udpThreadId)) == NULL)
    {
        //display error
        return;
    }
}

/*---------------------------------------------------------------------------------
--	FUNCTION: startUDPServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI startUDPServer(LPVOID n)
--
--	PARAMETERS:	LPVOID n
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function starts the UDP server. It creates a UDP socket, then binds to it.
--  It then creates a WSAEvent for the WSAWaitForMultipleEvents call in
--  the udpThread method. It creates a thread to run the udpThread method. Finally
--  it calls the select method to set a very large timeout on the socket. When a
--  datagram arrives at the socket, the timeout is set to 1 second as a way to
--  signal when the datagrams stop arriving. When the socket times out, it prints
--	out the statistics collected and the timeout is reset to the original timeout.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI startUDPServer(LPVOID n)
{
    struct	sockaddr_in udpServer;
    DWORD threadId;
    int error = 0, selectRet;
    char buf[MAXLEN];
    HANDLE threadHandle;
    fd_set fds;
    struct timeval tv;

    serverRunning = true;

    if ((udpEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        ////writeToScreen("WSACreateEvent() failed");
        return -1;
    }

    if ((threadHandle = CreateThread(NULL, 0, udpThread, (LPVOID)udpEvent, 0, &threadId)) == NULL)
    {

    }

    char message[256];

    FD_ZERO(&fds);
    FD_SET(udpSocket, &fds);

    tv.tv_sec = 20;
    tv.tv_usec = 0;

    while (true)
    {
        selectRet = select(udpSocket, &fds, NULL, NULL, &tv);
        if (serverRunning)
        {
            if (selectRet == SOCKET_ERROR)
            {
                if ((error = WSAGetLastError()) != WSA_IO_PENDING)
                {
                    sprintf(message, "select failed with error %d", error);
                    ////writeToScreen(message);
                    break;
                }
            }
            else if (selectRet == 0)
            {
                tv.tv_sec = 20;
                FD_ZERO(&fds);
                FD_SET(udpSocket, &fds);

                if (WSASetEvent(udpEvent) == FALSE)
                {
                    ////writeToScreen("Resetting event failed");
                }
            }
            else {
                if (WSASetEvent(udpEvent) == FALSE)
                {
                    //writeToScreen("Resetting event failed");
                }
                //tv.tv_sec = 1;
            }
        }
        else {
            break;
        }

    }
    //return 0;
    ExitThread(0);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: udpThread
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI udpThread(LPVOID lpParameter)
--
--	PARAMETERS:	LPVOID lpParameter - WSAEvent from startUDPServer
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function calls WSAWaitForMultipleEvents. When it receives an event,
--  it creates a socketInfo data structure and calls WSARecvFrom to read data from
--  the socket into socketInfo. When WSARecvFrom has read data successfully, a
--  completion routine is called.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI udpThread(LPVOID lpParameter)
{
    LPSOCKET_INFORMATION socketInfo;
    WSAEVENT eventArray[1];
    DWORD index, flags;
    int error = 0;
    char message[256];
    SOCKADDR_IN	client;
    int clientSize = sizeof(SOCKADDR_IN);

    eventArray[0] = (WSAEVENT)lpParameter;
    while (true)
    {
        while (true)
        {
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            if (index == WSA_WAIT_FAILED)
            {
                //writeToScreen("WSAWaitForMultipleEvents failed");
                break;
            }
            if (index != WAIT_IO_COMPLETION)
            {
                break;
            }
        }

        WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);

        // Create a socket information structure to associate with socket.
        if ((socketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            sprintf(message, "GlobalAlloc() failed with error %d\n", GetLastError());
            //writeToScreen(message);
            return FALSE;
        }

        // Fill in the details of our accepted socket.
        socketInfo->Socket = udpSocket;
        ZeroMemory(&(socketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        socketInfo->DataBuf.len = DATA_BUFSIZE;
        socketInfo->DataBuf.buf = socketInfo->Buffer;
        socketInfo->Timeout = INFINITE;

        flags = 0;
        if (WSARecvFrom(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &flags, (sockaddr *)&client, &clientSize, &(socketInfo->Overlapped), udpRoutine) == SOCKET_ERROR)
        {
            if ((error = WSAGetLastError()) != WSA_IO_PENDING)
            {
                sprintf(message, "WSARecvFrom failed with error %d", error);
                //writeToScreen(message);
            }
        }
    }
}

/*---------------------------------------------------------------------------------
--	FUNCTION: udpRoutine
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void CALLBACK udpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
--
--	PARAMETERS:	DWORD errorCode - errorCode from WSARecvFrom call
--				DWORD bytesTransferred - the number of bytes read by WSARecvFrom
--				LPOVERLAPPED overlapped - the overlapped structure that contains
--											the data read
--				DWORD flags - flags used for WSARecvFrom call
--
--	RETURNS:	none
--
--	NOTES:
--	This function is called when WSARecvFrom successfully reads data from the UDP
--	socket. It updates the statistics and writes the data read to file (if user
--  specified a file to save to).
--
---------------------------------------------------------------------------------*/
void CALLBACK udpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
{
    LPSOCKET_INFORMATION socketInfo = (LPSOCKET_INFORMATION)overlapped;
    DWORD newFlags;
    SOCKADDR_IN	client;
    int clientSize = sizeof(client);
    int error = 0;
    char message[256];

    if (errorCode != 0)
    {
        //writeToScreen("UDP recv error");
    }

    if (bytesTransferred > 0)
    {
        if (!(NetworkManager::incBuffer->cbWrite(socketInfo->DataBuf.buf, socketInfo->DataBuf.len)))
        {
        }
    }
}

void NetworkManager::startTCPReceiver(int port)
{
    HANDLE tcpHandle;
    DWORD tcpThreadId;

    NetworkManager::acceptSocket = NULL;
    tPort = port;

    if ((tcpHandle = CreateThread(NULL, 0, startTCPServer, 0, 0, &tcpThreadId)) == NULL)
    {
        //display error
        return;
    }
    tcpServerRunning = true;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: startTCPServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI startTCPServer(LPVOID n)
--
--	PARAMETERS:	LPVOID n
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function starts the TCP server. It creates a TCP socket, then binds to it.
--  Then it calls listen so the socket will be listening to any incoming connection
--  requests. It then creates a WSAEvent for the WSAWaitForMultipleEvents call in
--  the tcpThread method. It creates a thread to run the tcpThread method. Finally
--  it waits for and accepts incoming connection requests.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI startTCPServer(LPVOID n)
{
    struct	sockaddr_in tcpServer;

    LPSOCKET_INFORMATION tcpSocketInfo;
    DWORD flags, threadId;
    int error = 0;
    char message[256];
    HANDLE threadHandle;

    // Bind an address to the socket
    memset((char *)&tcpServer, 0, sizeof(tcpServer));
    tcpServer.sin_family = AF_INET;
    tcpServer.sin_port = htons(tPort);
    tcpServer.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(tcpSocket, (struct sockaddr *)&tcpServer, sizeof(tcpServer)) == SOCKET_ERROR)
    {
        //writeToScreen("Can't bind name to socket");
    }

    // Create a socket information structure to associate with socket.
    if ((tcpSocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
        sizeof(SOCKET_INFORMATION))) == NULL)
    {
        //sprintf(message, "GlobalAlloc() failed with error %d\n", GetLastError());
        //writeToScreen(message);
        return FALSE;
    }

    if (listen(tcpSocket, 5) == SOCKET_ERROR)
    {
        //writeToScreen("listen failed");
    }

    if ((tcpEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        //writeToScreen("WSACreateEvent() failed");
    }

    if ((threadHandle = CreateThread(NULL, 0, tcpThread, (LPVOID)tcpEvent, 0, &threadId)) == NULL)
    {
        //writeToScreen("CreateThread() failed");
    }

    while (tcpServerRunning)
    {
        NetworkManager::acceptSocket = accept(tcpSocket, NULL, NULL);
        NetworkManager::tcpConnected = true;
        if (WSASetEvent(tcpEvent) == FALSE)
        {
            //sprintf(message, "WSASetEvent failed with error %d\n", WSAGetLastError());
            //writeToScreen(message);
            ExitThread(0);
        }
        //emit connected signal
    }
    ExitThread(0);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: tcpThread
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI tcpThread(LPVOID lpParameter)
--
--	PARAMETERS:	LPVOID lpParameter - WSAEvent from startTCPServer
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function calls WSAWaitForMultipleEvents. When it receives an event,
--  it creates a socketInfo data structure and calls WSARecv to read data from
--  the socket into socketInfo. When WSARecv has read data successfully, a
--  completion routine is called.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI tcpThread(LPVOID lpParameter)
{
    LPSOCKET_INFORMATION socketInfo;
    WSAEVENT eventArray[1];
    DWORD index, flags;
    int error = 0;
    char message[256];
    SOCKADDR_IN	client;
    int clientSize = sizeof(SOCKADDR_IN);

    eventArray[0] = (WSAEVENT)lpParameter;
    while (true)
    {
        while (true)
        {
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            if (index == WSA_WAIT_FAILED)
            {
                //writeToScreen("WSAWaitForMultipleEvents failed");
                break;
            }
            if (index != WAIT_IO_COMPLETION)
            {
                break;
            }
        }

        WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);

        // Create a socket information structure to associate with socket.
        if ((socketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            //sprintf(message, "GlobalAlloc() failed with error %d\n", GetLastError());
           // writeToScreen(message);
            return FALSE;
        }

        // Fill in the details of our accepted socket.
        socketInfo->Socket = NetworkManager::acceptSocket;
        ZeroMemory(&(socketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        socketInfo->DataBuf.len = DATA_BUFSIZE;
        socketInfo->DataBuf.buf = socketInfo->Buffer;
        socketInfo->Timeout = INFINITE;

        flags = 0;
        if (WSARecv(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &flags, &(socketInfo->Overlapped), tcpRoutine) == SOCKET_ERROR)
        {
            if ((error = WSAGetLastError()) != WSA_IO_PENDING)
            {
                //sprintf(message, "WSARecv failed with error %d", error);
                //writeToScreen(message);
            }
        }
    }
}

/*---------------------------------------------------------------------------------
--	FUNCTION: tcpSelectThread
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI tcpSelectThread(LPVOID lpParameter)
--
--	PARAMETERS:	LPVOID lpParameter - WSAEvent from startTCPServer
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function calls WSAWaitForMultipleEvents. When it receives an event,
--  it creates a socketInfo data structure and calls WSARecv to read data from
--  the socket into socketInfo. When WSARecv has read data successfully, a
--  completion routine is called.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI tcpSelectThread(LPVOID lpParameter)
{
    LPSOCKET_INFORMATION socketInfo;
    WSAEVENT eventArray[1];
    DWORD index, flags;
    int error = 0;
    char message[256];
    SOCKADDR_IN	client;
    int clientSize = sizeof(SOCKADDR_IN);

    eventArray[0] = (WSAEVENT)lpParameter;
    while (true)
    {
        while (true)
        {
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            if (index == WSA_WAIT_FAILED)
            {
                //writeToScreen("WSAWaitForMultipleEvents failed");
                break;
            }
            if (index != WAIT_IO_COMPLETION)
            {
                break;
            }
        }

        WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);

        // Create a socket information structure to associate with socket.
        if ((socketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            //sprintf(message, "GlobalAlloc() failed with error %d\n", GetLastError());
           // writeToScreen(message);
            return FALSE;
        }

        // Fill in the details of our accepted socket.
        socketInfo->Socket = NetworkManager::acceptSocket;
        ZeroMemory(&(socketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        socketInfo->DataBuf.len = DATA_BUFSIZE;
        socketInfo->DataBuf.buf = socketInfo->Buffer;
        socketInfo->Timeout = INFINITE;

        flags = 0;
        if (WSARecv(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &flags, &(socketInfo->Overlapped), tcpRoutine) == SOCKET_ERROR)
        {
            if ((error = WSAGetLastError()) != WSA_IO_PENDING)
            {
                //sprintf(message, "WSARecv failed with error %d", error);
                //writeToScreen(message);
            }
        }
    }
}

/*---------------------------------------------------------------------------------
--	FUNCTION: tcpRoutine
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void CALLBACK tcpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
--
--	PARAMETERS:	DWORD errorCode - errorCode from WSARecv call
--				DWORD bytesTransferred - the number of bytes read by WSARecv
--				LPOVERLAPPED overlapped - the overlapped structure that contains
--											the data read
--				DWORD flags - flags used for WSARecv call
--
--	RETURNS:	none
--
--	NOTES:
--	This function is called when WSARecv successfully reads data from the TCP
--	socket. It updates the statistics and writes the data read to file (if user
--  specified a file to save to). If there were no bytes transferred, the statistics
--  are printed to the screen and reset. It then calls WSARecv so the server will
--  be ready to read data when it arrives.
--
---------------------------------------------------------------------------------*/
void CALLBACK tcpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
{
    LPSOCKET_INFORMATION socketInfo = (LPSOCKET_INFORMATION)overlapped;
    DWORD newFlags;
    int error = 0;
    char message[256];

    if (errorCode != 0)
    {
        //writeToScreen("TCP recv error");
    }

    if (bytesTransferred > 0)
    {
        if (!(NetworkManager::tcpBuffer->cbWrite(socketInfo->DataBuf.buf, bytesTransferred)))
        {
        }
    }

    newFlags = 0;
    if (WSARecv(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &newFlags, &(socketInfo->Overlapped), tcpRoutine) == SOCKET_ERROR)
    {
        if ((error = WSAGetLastError()) != WSA_IO_PENDING)
        {
            //sprintf(message, "WSARecv failed with error %d", error);
            //writeToScreen(message);
        }
    }
}

