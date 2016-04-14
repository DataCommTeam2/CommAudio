#ifndef AUDIORECORDTHREAD_H
#define AUDIORECORDTHREAD_H

#include <QObject>
#include <QIODevice>

#define MAX_LEN 8192

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          AudioRecordThread
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       AudioRecordThread(QIODevice * buf): buffer(buf), position(0)
--                   void checkMicrophone()
--                   void sendDataFromMic(char *, int);
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
--  NOTES:           This thread is for processing audio recordings into the circular buffer for sending
--                   to clients.
------------------------------------------------------------------------------------------*/
class AudioRecordThread : public QObject
{
    Q_OBJECT
public:
    AudioRecordThread(QIODevice * buf): buffer(buf), position(0) {}

private:
    QIODevice * buffer;
    long int position;
public slots:
    void checkMicrophone();
signals:
    void sendDataFromMic(char *, int);
};

#endif // AUDIORECORDTHREAD_H
