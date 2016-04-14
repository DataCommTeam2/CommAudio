#include "audiothread.h"
#include <QDebug>

//AudioThread::AudioThread() {

//}

void AudioThread::checkForEnding() {
    while (!forceKill && audioPlayer != NULL && audioPlayer->state() != QAudio::ActiveState) {}
    while (!forceKill && audioPlayer != NULL && (audioPlayer->state() != QAudio::IdleState && audioPlayer->state() != QAudio::StoppedState)) {}
    emit workFinished(QString("Song finished, playing next song."));
}
