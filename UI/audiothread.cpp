#include "audiothread.h"
#include <QDebug>


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        checkForEnding
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioThread::checkForEnding()
--  
--  RETURNS:         void
--  
--  NOTES:           This thread loops until the song has finished playing. After the song is done, it
--                   will send a message to the UI thread indicating that the song has finished.
------------------------------------------------------------------------------------------*/
void AudioThread::checkForEnding() {
    while (!forceKill && audioPlayer != NULL && audioPlayer->state() != QAudio::ActiveState) {}
    while (!forceKill && audioPlayer != NULL && (audioPlayer->state() != QAudio::IdleState && audioPlayer->state() != QAudio::StoppedState)) {}
    emit workFinished(QString("Song finished, playing next song."));
}
