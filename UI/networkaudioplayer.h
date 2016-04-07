#ifndef NETWORKAUDIOPLAYER_H
#define NETWORKAUDIOPLAYER_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QIODevice>
#include "audiomanager.h"
#include "circularbuffer.h"

class NetworkAudioPlayer : public QObject
{
public:
    NetworkAudioPlayer();
    void setParameters();
private:
    QAudioFormat format;
    QAudioOutput *audio;
    QIODevice * audioDevice;
    QAudioDeviceInfo deviceinfo;
    bool second;
    CircularBuffer * audioBuffer;
signals:
public slots:
    void playAudio();
    void appendAudioData();
    void stopAudio();

};

#endif // NETWORKAUDIOPLAYER_H
