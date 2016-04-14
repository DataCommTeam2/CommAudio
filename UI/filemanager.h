#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include "networkmanager.h"
#include "filereader.h"

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          FileManager
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       FileManager(NetworkManager * nManager);
--                   bool requestFile(const char * fileName);
--                   void openFileForSending(char * filename);
--                   void writeToFile(char * data, int length);
--                   void checkBuffer();
--                   void errorFromPeer();
--                   void fileDone();
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
class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(NetworkManager * nManager);
    bool requestFile(const char * fileName);
    void openFileForSending(char * filename);
public slots:
    void writeToFile(char * data, int length);
    void checkBuffer();
signals:
    void errorFromPeer();
    void fileDone();
private:
    NetworkManager * networkManager;
    FILE * fp;
    char incomingData[DATA_BUFSIZE];
    FileReader * reader;
};

#endif // FILEMANAGER_H
