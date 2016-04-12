#include "networkmanager.h"

SOCKET udpSocket;
SOCKET tcpSocket;
struct sockaddr_in udpPeer;
/*---------------------------------------------------------------------------------
--	FUNCTION: startNetwork
--
--	DATE:		April 12, 2016
--
--	REVISIONS:	April 12, 2016
--
--	DESIGNER:	Tom Tang
--
--	PROGRAMMER:	Tom Tang
--
--	INTERFACE:	startNetwork()
--

--	RETURNS:	bool - returns wether or not the network has been started
--
--	NOTES:
--	This function starts the network server for multicast and peer to peer
--
---------------------------------------------------------------------------------*/
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

    if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        //display error
        return false;
    }

    return true;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: connectViaTCP
--
--	DATE:		April 12, 2016
--
--	REVISIONS:	April 12, 2016
--
--	DESIGNER:	Tom Tang
--
--	PROGRAMMER:	Tom Tang
--
--	INTERFACE:	startNetwork()
--
--  PARAMETERS:	char * hostname - hostname of server
--				      int port - port of server
--
--	RETURNS:	void
--
--	NOTES:
--	This function connects to a server via tcp
--
---------------------------------------------------------------------------------*/
void NetworkManager::connectViaTCP(char * hostname, int port)
{
    struct hostent	*hp;

    // Store server's information
    memset((char *)&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);

    if ((hp = gethostbyname(hostname)) == NULL)
    {
        //writeToScreen("Can't get server's IP address");
        return;
    }

    memcpy((char *)&peer.sin_addr, hp->h_addr, hp->h_length);
    if (connect(tcpSocket, (struct sockaddr *)&peer, sizeof(peer)) == -1)
    {
        //writeToScreen("Can't connect to server");
        return;
    }

    tcpConnected = true;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: cleanUp
--
--	DATE:		April 12, 2016
--
--	REVISIONS:	April 12, 2016
--
--	DESIGNER:	Tom Tang
--
--	PROGRAMMER:	Tom Tang
--
--  INTERFACE:	cleanUp()
--
--	RETURNS:	void
--
--	NOTES:
--	This function closes the sockets when called
--
---------------------------------------------------------------------------------*/
void NetworkManager::cleanUp()
{
    closesocket(udpSocket);
    closesocket(tcpSocket);
    WSACleanup();
}

/*---------------------------------------------------------------------------------
--	FUNCTION: setupUDPforP2P
--
--	DATE:		April 12, 2016
--
--	REVISIONS:	April 12, 2016
--
--	DESIGNER:	Tom Tang
--
--	PROGRAMMER:	Tom Tang
--
--  INTERFACE:	setupUDPforP2P()
--
--	RETURNS:	void
--
--	NOTES:
--	This function sets up a UDP server for peet to peer sending 
--
---------------------------------------------------------------------------------*/
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
