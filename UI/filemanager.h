#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include "networkmanager.h"
#include "filereader.h"

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(NetworkManager * nManager);
    void openFileForSending(char * filename);
public slots:
    void writeToFile(char * data, int length);
    void checkBuffer();
    void requestFile(const char * fileName);
signals:
    void errorFromPeer();
    void fileDone();
    void dataRead();
private:
    NetworkManager * networkManager;
    FILE * fp;
    char incomingData[DATA_BUFSIZE];
    FileReader * reader;
};

#endif // FILEMANAGER_H
