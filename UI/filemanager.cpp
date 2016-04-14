#include <QThread>
#include "filemanager.h"
#include <QDir>

FileManager::FileManager(NetworkManager *nManager)
{
    networkManager = nManager;
}

bool FileManager::requestFile(const char * fileName)
{
    QString absName = QDir::currentPath() + "/MusicFiles/" + fileName;
    fp = fopen(absName.toStdString().c_str(), "wb");
    return fp == NULL? false : true;
}

void FileManager::writeToFile(char * data, int length)
{
    fwrite(data, 1, length, fp);
    if (length > DATA_BUFSIZE)
    {
        fclose(fp);
        fp = NULL;
    }
}

void FileManager::checkBuffer()
{
    int bytesInBuffer = NetworkManager::tcpBuffer->getLastBytesWritten();
    while(NetworkManager::tcpBuffer->getBlocksUnread() > 0)
    {
        bytesInBuffer = NetworkManager::tcpBuffer->getLastBytesWritten();
        memcpy(incomingData, NetworkManager::tcpBuffer->cbRead(1), bytesInBuffer);
        switch(incomingData[0])
        {
        case 1:
            //error
            emit errorFromPeer();
            break;
        case 2:
            //process file request
            openFileForSending(&incomingData[1]);
            break;
        case 3:
            writeToFile(&incomingData[1], bytesInBuffer);
            break;
        default:
            break;
        }
    }
}

void FileManager::openFileForSending(char * filename)
{
    QThread * fileReaderThread = new QThread();
    reader = new FileReader(filename);
    reader->moveToThread(fileReaderThread);

    connect(fileReaderThread, SIGNAL(started()), reader, SLOT(startReading()));
    connect(reader, SIGNAL(sendPacket(char*,int)), networkManager, SLOT(sendViaTCP(char*, int)));
    connect(reader, SIGNAL(fileDone()), fileReaderThread, SLOT(quit()));
    connect(reader, SIGNAL(fileDone()), this, SIGNAL(fileDone()));
    connect(reader, SIGNAL(error(char*, int)), fileReaderThread, SLOT(quit()));
    connect(reader, SIGNAL(error(char*, int)), networkManager, SLOT(sendViaTCP(char*, int)));
    fileReaderThread->start();
}
