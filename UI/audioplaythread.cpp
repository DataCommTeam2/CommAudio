#include "audioplaythread.h"
#include <QDebug>


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        AudioPlayThread
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--				     Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--                   Gabriella Cheung
--
--  INTERFACE:       void AudioPlayThread::AudioPlayThread(CircularBuffer * buf)
--  
--  RETURNS:         void
--  
--  NOTES:           Cosntructor for this thread.
------------------------------------------------------------------------------------------*/
AudioPlayThread::AudioPlayThread(CircularBuffer * buf)
{
    buffer = buf;
    stopChecking = false;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        forceKill
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioPlayThread::forceKill()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will force the thread from executing any longer.
------------------------------------------------------------------------------------------*/
void AudioPlayThread::forceKill() {
    stopChecking = true;
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        checkBuffer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--                   Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--                   Gabriella Cheung
--  
--  INTERFACE:       void AudioPlayThread::checkBuffer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function loops until there is data available to be read.
------------------------------------------------------------------------------------------*/
void AudioPlayThread::checkBuffer() {
    while (!stopChecking && buffer->getBlocksUnread() == 0)
    {
    }
    if (!stopChecking)
        emit bufferHasData();
}
