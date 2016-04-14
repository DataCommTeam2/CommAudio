#ifndef NETWORKAUDIOPLAYER_H
#define NETWORKAUDIOPLAYER_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include "audiomanager.h"
#include "circularbuffer.h"
#include "networkmanager.h"

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
