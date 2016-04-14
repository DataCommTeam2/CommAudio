#ifndef AUDIOPLAYTHREAD_H
#define AUDIOPLAYTHREAD_H

#include <QObject>
#include "circularbuffer.h"

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
