#ifndef NETWORKAUDIOPLAYER_H
#define NETWORKAUDIOPLAYER_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include "audiomanager.h"
#include "circularbuffer.h"
#include "networkmanager.h"


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          NetworkAudioPlayer
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       NetworkAudioPlayer();
--                   bool setup(QFile *);
--                   void unpauseAudio();
--                   ~NetworkAudioPlayer();
--                   void finishedReading();
--                   void finishedWriting();
--                   void audioStarted(QAudioOutput *);
--                   void sendToClient(char *, int);
--                   void loadDataIntoBuffer();
--                   void writeDataToDevice();
--                   void playAudio();
--                   
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--                   Gabriella Cheung
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--                   Gabriella Cheung
--  
--  NOTES:           This class handles playing audio that is retrieved from the network through a buffer.
--                   It uses QAudioOutput to play the audio on the client's side.
------------------------------------------------------------------------------------------*/
class NetworkAudioPlayer : public QObject
{
    Q_OBJECT
public:
    NetworkAudioPlayer();
    bool setup(QFile *);
    void unpauseAudio();
    ~NetworkAudioPlayer();

    bool endThread = false;
private:
    QAudioFormat format;
    QAudioOutput *audio;
    QIODevice * audioDevice;
    QAudioDeviceInfo deviceinfo;
    CircularBuffer * audioBuffer;
    NetworkManager * netManager;
    QFile * file;
    FILE * fp;
    AudioPlayThread * bufferListener;
    bool PAUSED = false;
    bool PLAYING = false;
signals:
    void finishedReading();
    void finishedWriting();
    void audioStarted(QAudioOutput *);
    void sendToClient(char *, int);
private slots:
    void loadDataIntoBuffer();
    void writeDataToDevice();
public slots:
    void playAudio();
};

#endif // NETWORKAUDIOPLAYER_H
