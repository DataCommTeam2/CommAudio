#ifndef AUDIOPLAYTHREAD_H
#define AUDIOPLAYTHREAD_H

#include <QObject>
#include "circularbuffer.h"


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          AudioRecordThread
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       AudioPlayThread(CircularBuffer * buf);
--                   bool stopChecking;
--                   void addMoreData();
--                   void finished();
--                   void bufferHasData();
--                   void checkBuffer();
--                   void forceKill();
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--                   Gabrialla Cheung
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--                   Gabrialla Cheung
--  
--  NOTES:           This thread is for processing audio playing from a buffer and passing it into a 
--                   local audio manager to play the audio locally.
------------------------------------------------------------------------------------------*/
class AudioPlayThread : public QObject
{
    Q_OBJECT
public:
    AudioPlayThread(CircularBuffer * buf);
    //AudioPlayThread();
    bool stopChecking;

signals:
    void addMoreData();
    void finished();
    void bufferHasData();

public slots:
    void checkBuffer();

    void forceKill();

private:
    CircularBuffer * buffer;
};

#endif // AUDIOPLAYTHREAD_H
