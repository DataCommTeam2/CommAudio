#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include "networkmanager.h"

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[DATA_BUFSIZE];
    int head = 0;
    int tail = 0;
    WSABUF DataBuf;
    DWORD Timeout;

} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

SOCKET udpSocket;
SOCKET tcpSocket;
struct sockaddr_in udpPeer, tcpPeer;
SOCKET tcpAcceptSocket;
BOOL serverRunning = false;
int uPort, tPort;
HANDLE hWriteFile, hServerLogFile;
WSAEVENT udpEvent, tcpEvent;

DWORD WINAPI udpThread(LPVOID lpParameter);
DWORD WINAPI startUDPServer(LPVOID n);
void CALLBACK udpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
DWORD WINAPI tcpThread(LPVOID lpParameter);
DWORD WINAPI startTCPServer(LPVOID n);
void CALLBACK tcpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
void sendViaTCP();
void sendViaUDP();


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

    /*if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    if ((udpSocket = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }*/

    /*if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }*/

    return true;
}

bool NetworkManager::createMulticastServerSocket()
{
    struct ip_mreq stMreq;
    SOCKADDR_IN stLclAddr, stDstAddr;
    BOOL  fFlag = FALSE;

    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }
    stLclAddr.sin_family = AF_INET;
    stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stLclAddr.sin_port = 0;
    if (bind(udpSocket, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    stMreq.imr_multiaddr.s_addr = inet_addr("234.5.6.7");
    stMreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    /* Disable loopback */
    if (setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag)) == SOCKET_ERROR)
    {
        //display error
        return false;
    }

    /* Assign our destination address */
    stDstAddr.sin_family = AF_INET;
    stDstAddr.sin_addr.s_addr = inet_addr("234.5.6.7");
    stDstAddr.sin_port = htons(7000);

    return true;
}

void NetworkManager::connectViaTCP(char * hostname, int port)
{
    struct hostent	*hp;

    // Store server's information
    memset((char *)&tcpPeer, 0, sizeof(tcpPeer));
    tcpPeer.sin_family = AF_INET;
    tcpPeer.sin_port = htons(port);

    if ((hp = gethostbyname(hostname)) == NULL)
    {
        //writeToScreen("Can't get server's IP address");
        return;
    }

    memcpy((char *)&tcpPeer.sin_addr, hp->h_addr, hp->h_length);
    if (connect(tcpSocket, (struct sockaddr *)&tcpPeer, sizeof(tcpPeer)) == -1)
    {
        //writeToScreen("Can't connect to server");
        return;
    }

    tcpConnected = true;
}

void NetworkManager::cleanUp()
{
    closesocket(udpSocket);
    closesocket(tcpSocket);
    WSACleanup();
}

void NetworkManager::setupUDPforP2P()
{
    memset((char *)&udpPeer, 0, sizeof(udpPeer));
    udpPeer.sin_family = AF_INET;
    udpPeer.sin_port = htons(UDP_PORT);
    udpPeer.sin_addr.s_addr = htonl(INADDR_ANY);
    //udpPeer.sin_addr.s_addr = peer.sin_addr.s_addr; //does this work??

    if (bind(udpSocket, (struct sockaddr *)&udpPeer, sizeof(udpPeer)) == SOCKET_ERROR)
    {
        //writeToScreen("Can't bind name to socket");
        return;
    }
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
void sendViaTCP()
{
    char *sbuf;
    char message[256];

    sbuf = (char*)malloc(DATA_BUFSIZE);

    // transmit data
    if (send(tcpSocket, sbuf, strlen(sbuf), 0) == -1)
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
void sendViaUDP()
{
    char *sbuf;

    int server_len = sizeof(udpPeer);

    sbuf = (char*)malloc(DATA_BUFSIZE);

    // transmit data
    if (sendto(udpSocket, sbuf, strlen(sbuf), 0, (struct sockaddr *)&udpPeer, server_len) == -1)
    {
        //sprintf(message, "error: %d", WSAGetLastError());
        //writeToScreen(message);
    }
}

void NetworkManager::startUDPReceiver(int port)
{
    HANDLE udpHandle;
    DWORD udpThreadId;

    uPort = port;

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

    // Create a datagram socket
    /*if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        ////writeToScreen("Can't create a socket");
        return -1;
    }*/

    // Bind an address to the socket
    /*memset((char *)&udpServer, 0, sizeof(udpServer));
    udpServer.sin_family = AF_INET;
    udpServer.sin_port = htons(uPort);
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpSocket, (struct sockaddr *)&udpServer, sizeof(udpServer)) == SOCKET_ERROR)
    {
        ////writeToScreen("Can't bind name to socket");
        return -1;
    }*/

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
        /*socketInfo->DataBuf.buf = socketInfo->Buffer+Head*DATA_BUFSIZE;
        if(++socketInfo->Head == NUM_OF_BUFFERS){
            socketInfo->Head = 0;
        }*/
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
        //audioBuffer->cbWrite(socketInfo->DataBuf.buf, socketInfo->DataBuf.len);
    }
}

void NetworkManager::startTCPReceiver(int port)
{
    HANDLE tcpHandle;
    DWORD tcpThreadId;

    tPort = port;

    if ((tcpHandle = CreateThread(NULL, 0, startTCPServer, (LPVOID)0, 0, &tcpThreadId)) == NULL)
    {
        //display error
        return;
    }
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

    // Create a stream socket
    /*if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //writeToScreen("Can't create a socket");
        //exit(1);
    }*/

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

    while (serverRunning)
    {
        tcpAcceptSocket = accept(tcpSocket, NULL, NULL);

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
        socketInfo->Socket = tcpAcceptSocket;
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
    char data[MAXLEN] = { 0 };
    int error = 0;
    char message[256];

    if (errorCode != 0)
    {
        //writeToScreen("TCP recv error");
    }

    if (bytesTransferred > 0)
    {
        memcpy(data, socketInfo->DataBuf.buf, socketInfo->DataBuf.len);
        switch (data[0])
        {
            //case stop sending
            //emit stop sending signal
            //case start sending
            //case start sending signal
            default:
                break;
        }

    }
    else {
        return;
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

