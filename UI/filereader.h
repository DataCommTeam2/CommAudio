#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include "networkmanager.h"

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          FileReader
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       FileReader(char * name):filename(name);
--                   char * filename;
--                   FILE * fp;
--                   char data[DATA_BUFSIZE];
--                   void fileDone();
--                   void sendPacket(char *, int);
--                   void error(char *, int);
--                   void startReading();
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
--  NOTES:           This class is for reading files from the network and saving them to the computer.
------------------------------------------------------------------------------------------*/
class FileReader : public QObject
{
    Q_OBJECT
public:
    FileReader(char * name):filename(name){}
private:
    char * filename;
    FILE * fp;
    char data[DATA_BUFSIZE];
signals:
    void fileDone();
    void sendPacket(char *, int);
    void error(char *, int);
public slots:
    void startReading();
};

#endif // FILEREADER_H
