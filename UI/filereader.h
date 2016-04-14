#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include "networkmanager.h"

class FileReader : public QObject
{
    Q_OBJECT
public:
    FileReader(char * name):filename(name){}
    void sendSize();
private:
    char * filename;
    FILE * fp;
    char data[DATA_BUFSIZE] = {0};
signals:
    void fileDone();
    void sendPacket(char *, int);
    void error(char *, int);
public slots:
    void startReading();
    void sendData();
};

#endif // FILEREADER_H
