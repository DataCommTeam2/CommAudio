#ifndef LOCALAUDIOMANAGER_H
#define LOCALAUDIOMANAGER_H

#include <QAudioFormat>
#include <QObject>
#include <QFile>
#include <QAudioOutput>
#include "wavheader.h"

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          LocalAudioManager
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       LocalAudioManager(QObject * p) : parent(p)
--                   QAudioOutput * playAudio();
--                   bool setupAudioPlayer(QFile * file);
--                   void setVolume(double volume);
--                   void stopAudio();
--                   void pauseAudio();
--                   void unpauseAudio();
--                   bool isPaused();
--                   bool isPlaying();
--                   ~LocalAudioManager();
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  NOTES:           This class is for playing audio from locally files. It has nothing to do with the
--                   network or the buffer, and opens files to read. It auto detects the wav header
--                   of the audio file, and uses QAudioOutput to play the audio.
------------------------------------------------------------------------------------------*/
class LocalAudioManager
{
public:
    LocalAudioManager(QObject * p) : parent(p) {
        audio = new QAudioOutput(QAudioFormat(), p);
    }

    QAudioOutput * playAudio();
    bool setupAudioPlayer(QFile * file);
    void setVolume(double volume);
    void stopAudio();
    void pauseAudio();
    void unpauseAudio();
    bool isPaused();
    bool isPlaying();

    ~LocalAudioManager();

private:
    bool PAUSED = false;
    bool PLAYING = false;
    QAudioFormat format;
    QAudioOutput *audio;
    QObject *parent;
    QFile *file;
    QIODevice *device;
    double constantVolume = 1.0;
};



#endif // LOCALAUDIOMANAGER_H
