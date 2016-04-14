#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H


#include <QObject>
#include <QIODevice>
#include <QAudio>
#include <QAudioOutput>

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          AudioThread
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       AudioThread(QAudioOutput * aD) : audioPlayer(aD)
--                   ~AudioThread()
--                   void checkForEnding();
--                   void workFinished(const QString msg);
--                   
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  NOTES:           This class is a thread which waits and alerts all listeners when the song currently
--                   playing has finished.
------------------------------------------------------------------------------------------*/
class AudioThread : public QObject
{
    Q_OBJECT
public:
    AudioThread(QAudioOutput * aD) : audioPlayer(aD) {}
    //AudioThread();
    ~AudioThread() {}


    QAudioOutput * audioPlayer;
    bool forceKill = false;

public slots:
    void checkForEnding();

signals:
    void workFinished(const QString msg);

};

#endif // AUDIOTHREAD_H
