#include <QThread>
#include "filemanager.h"
#include <QDir>

FileManager::FileManager(NetworkManager *nManager)
{
    networkManager = nManager;
}

void FileManager::requestFile(FILE * f)
{
    //QString absName = QDir::currentPath() + "/MusicFiles/" + fileName;
    fp = f;
    stopChecking = false;
    while(bytesRemaining > 0)
    {
        checkBuffer();
    }
    fclose(fp);
    fp = NULL;
}

void FileManager::writeToFile(char * data, int length)
{
    fwrite(data, length, 1, fp);
    bytesLastWritten = length;
    bytesRemaining -= length;
}

void FileManager::checkBuffer()
{
    int bytesInBuffer;
    while(NetworkManager::tcpBuffer == NULL || NetworkManager::tcpBuffer->getBlocksUnread() == 0)
    {
        if (stopChecking)
        {
            return;
        }
    }
    if (NetworkManager::tcpBuffer == NULL)
    {
        return;
    }
    bytesInBuffer = NetworkManager::tcpBuffer->getLastBytesWritten();
    memcpy(incomingData, NetworkManager::tcpBuffer->cbRead(1), bytesInBuffer);
    if (receivingData)
    {
        writeToFile(incomingData, bytesInBuffer);
    }else {
        switch(incomingData[0])
        {
        case 1:
            setFileSize(&incomingData[1]);
            break;
        case 2:
            //process file request
            openFileForSending(&incomingData[1]);
            break;
        case 3:
            writeToFile(&incomingData[1], bytesInBuffer);
            break;
        case 4:
            emit readData();
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
    connect(this, SIGNAL(readData()), reader, SLOT(sendData()));
    fileReaderThread->start();
}

void FileManager::setFileSize(char * sizeStr)
{
    char data[8];
    data[0] = 4;
    bytesRemaining = atoi(sizeStr);
    receivingData = true;
    networkManager->sendViaTCP(data, 1);
}
