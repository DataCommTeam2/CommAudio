#include "audioplaythread.h"
#include <QDebug>

AudioPlayThread::AudioPlayThread(CircularBuffer * buf)
{
    buffer = buf;
    stopChecking = false;
}

void AudioPlayThread::forceKill() {
    stopChecking = true;
}

void AudioPlayThread::checkBuffer() {
    while (!stopChecking && buffer->getBlocksUnread() == 0)
    {
    }
    if (!stopChecking)
        emit bufferHasData();
    qDebug() << "STILL";
}
