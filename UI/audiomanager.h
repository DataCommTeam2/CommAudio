#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QAudioFormat>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QAudioOutput>
#include "wavheader.h"


#include "circularbuffer.h"
#include "audioplaythread.h"

#define DATA_BUFSIZE    8192
#define MAX_BLOCKS      100

class AudioManager : public QObject
{
    Q_OBJECT
public:
    AudioManager(QObject * p) : parent(p) {
        audio = new QAudioOutput(QAudioFormat(), p);
        //audioBuf = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);
    }

    QAudioOutput * playAudio();
    bool setupAudioPlayer(QFile * file);
    bool setupAudioPlayerNoFile(CircularBuffer *);
    bool setupAudioPlayerP2P(CircularBuffer *);
    void setVolume(double volume);
    bool stopAudio();
    void pauseAudio();
    void unpauseAudio();
    bool isPaused();
    bool isPlaying();
    CircularBuffer * getAudioBuffer();

    ~AudioManager();

private:
    bool PAUSED = false;
    bool PLAYING = false;
    QAudioFormat format;
    QAudioOutput *audio;
    QObject *parent;
    QFile *file = NULL;
    double constantVolume = 1.0;
    CircularBuffer * audioBuf;
    AudioPlayThread * bufferListener;
    long blockCount = 0;

signals:
    void finishedLoading();
    void finishedReading();
    void finishedWriting();
    void killPlayThread();

private slots:
    void loadDataIntoBuffer();
    void writeDataToDevice();
};

#endif // AUDIOMANAGER_H
