#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H


#include <QObject>
#include <QIODevice>
#include <QAudio>
#include <QAudioOutput>


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
