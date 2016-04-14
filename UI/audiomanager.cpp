#include "audiomanager.h"
#include <iostream>
#include <QDebug>

QIODevice *device = NULL;
QThread *playThread;


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          AudioManager
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       AudioManager(QObject * p) : parent(p);
--                   QAudioOutput * playAudio();
--                   bool setupAudioPlayer(QFile * file);
--                   bool setupAudioPlayerNoFile(CircularBuffer *);
--                   bool setupAudioPlayerP2P(CircularBuffer *);
--                   void setVolume(double volume);
--                   bool stopAudio();
--                   void pauseAudio();
--                   void unpauseAudio();
--                   bool isPaused();
--                   bool isPlaying();
--                   CircularBuffer * getAudioBuffer();
--                   ~AudioManager();
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
--  NOTES:           This class handles local audio playing through a buffer using QAudioOutput.
------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        setupAudioPlayer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       bool AudioManager::setupAudioPlayer(QFile * f)
--  
--  RETURNS:         bool
--  
--  NOTES:           This function sets up the network audio player, and gets ready to send the song
--                   along the network.
------------------------------------------------------------------------------------------*/
bool AudioManager::setupAudioPlayer(QFile * f) {
    file = f;
    wav_hdr wavHeader;
    file->open(QIODevice::ReadOnly);
    int bytesRead = file->read((char*)&wavHeader, sizeof(wav_hdr));
   // file->close();
    if (bytesRead <= 0)
        return false;

    format.setSampleRate(wavHeader.SamplesPerSec);
    format.setChannelCount(wavHeader.NumOfChan);
    format.setSampleSize(wavHeader.bitsPerSample);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioOutput(format, parent);
    audio->setVolume(constantVolume);
    audio->setBufferSize(DATA_BUFSIZE * 10);

    audioBuf = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);

    return true;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        setupAudioPlayerNoFile
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
--  INTERFACE:       bool AudioManager::setupAudioPlayerNoFile(CircularBuffer * buffer)
--  
--  RETURNS:         bool
--  
--  NOTES:           This function will setup audio playing for multicast.
------------------------------------------------------------------------------------------*/
bool AudioManager::setupAudioPlayerNoFile(CircularBuffer * buffer) {
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioOutput(format, parent);
    audio->setVolume(constantVolume);
    audio->setBufferSize(DATA_BUFSIZE*10);
    audioBuf = buffer;
    return true;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        setupAudioPlayerP2P
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
--  INTERFACE:       bool AudioManager::setupAudioPlayerP2P(CircularBuffer * buffer)
--  
--  RETURNS:         bool
--  
--  NOTES:           This function will setup audio playing for P2P.
------------------------------------------------------------------------------------------*/
bool AudioManager::setupAudioPlayerP2P(CircularBuffer * buffer) {
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioOutput(format, parent);
    audio->setVolume(constantVolume);
    //audioBuf = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);

    audio->setBufferSize(DATA_BUFSIZE * 10);
    audioBuf = buffer;
    return true;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        loadDataIntoBuffer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void AudioManager::loadDataIntoBuffer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function loads the data from the file and puts it into the circular buffer.
------------------------------------------------------------------------------------------*/
void AudioManager::loadDataIntoBuffer()
{
    char tempBuf[DATA_BUFSIZE];
    if(!audioBuf->isFull())
    {
        int bytesRead = file->read((char*)&tempBuf, sizeof(tempBuf));
        if (bytesRead <= 0)
        {
            emit finishedReading();
        }
        audioBuf->cbWrite(tempBuf, bytesRead);
    }
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        writeDataToDevice
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void AudioManager::writeDataToDevice()
--  
--  RETURNS:         void
--  
--  NOTES:           This function attempts to send data from the buffer into the QAudioOutput file for local
--                   audio playing.
------------------------------------------------------------------------------------------*/
void AudioManager::writeDataToDevice() {
    if (PAUSED)
        return;
    if (audio == NULL) {
        file->close();
        return;
    }
    if (device == NULL)
    {
        device = audio->start();
    }
    int freeSpace = audio->bytesFree();
    if (freeSpace < DATA_BUFSIZE)
    {
        if (!PAUSED)
            emit finishedWriting();
        return;
    } else {
        char * data = audioBuf->cbRead(1);
        int length = audioBuf->getLastBytesWritten();
        if (allowWrite)
            device->write(data, length);
        blockCount++;
        if (blockCount <= 10)
        {
            audio->suspend();
        } else {
            audio->resume();
        }
        audio->setVolume(constantVolume);
        emit finishedWriting();
    }
    if (file != NULL) {
        if(!file->atEnd())
        {
           loadDataIntoBuffer();
        } else if (!PAUSED){
            file->close();
        }
    }
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        playAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--                   Jaegar Sarauer
--  
--  INTERFACE:       QAudioOutput *AudioManager::playAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This fuction plays  the audio from the buffer. From whatever buffer data is available
--                   at the time. After sending the data to the QAudioOutput, the fuction requests for more
--                   data.
------------------------------------------------------------------------------------------*/
QAudioOutput *AudioManager::playAudio() {
    //if (!PLAYING && !PAUSED)
    //    return NULL;
    if (!PAUSED) {
        if (!playThread)
        {

            playThread = new QThread( );
        } else {
            if (playThread->isRunning())
            {
                //stop thread
                playThread->terminate();
                delete playThread;
                playThread = NULL;
                playThread = new QThread();
            }
        }
            device = audio->start();
        bufferListener = new AudioPlayThread(audioBuf);
        bufferListener->moveToThread(playThread);

        connect( playThread, SIGNAL(started()), this, SLOT(loadDataIntoBuffer()) );
        connect( playThread, SIGNAL(started()), bufferListener, SLOT(checkBuffer()) );
        connect( this, SIGNAL(finishedWriting()), bufferListener, SLOT(checkBuffer()) );
        connect( this, SIGNAL(killPlayThread()), bufferListener, SLOT(forceKill()) );
        connect( bufferListener, SIGNAL(bufferHasData()), this, SLOT(writeDataToDevice()));

        //automatically delete thread and deviceListener object when work is done:
        //connect( playThread, SIGNAL(finished()), bufferListener, SLOT(deleteLater()) );
        //connect( playThread, SIGNAL(finished()), playThread, SLOT(deleteLater()) );
        playThread->start();

    } else {
        unpauseAudio();
    }
    PLAYING = true;
    PAUSED = false;
    return audio;
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        setVolume
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::setVolume(double volume)
--  
--  RETURNS:         void
--  
--  NOTES:           This function sets the volume of the QAudioOutput playing the audio.
------------------------------------------------------------------------------------------*/
void AudioManager::setVolume(double volume) {
    constantVolume = volume;
    audio->setVolume(volume);
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        stopAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::stopAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will stop the audio player and clear the buffer from data to allow
--                   a new song to start.
------------------------------------------------------------------------------------------*/
bool AudioManager::stopAudio() {
    PAUSED = false;
    PLAYING = false;
    emit finishedWriting();
    if (audio == NULL)
        return false;
    audio->destroyed();
    audio = NULL;
    if (playThread != NULL) {
        emit killPlayThread();
        playThread->terminate();
        playThread = NULL;
    }
    return true;
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        pauseAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::unpauseAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function pauses playing audio from the QAudioOutput player.
------------------------------------------------------------------------------------------*/
void AudioManager::pauseAudio() {
    PAUSED = true;
    audio->suspend();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        unpauseAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::unpauseAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function resumes playing audio from the QAudioOutput player.
------------------------------------------------------------------------------------------*/
void AudioManager::unpauseAudio() {
    audio->resume();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        isPaused
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::isPaused()
--  
--  RETURNS:         void
--  
--  NOTES:           This function returns if the audio is paused.
------------------------------------------------------------------------------------------*/
bool AudioManager::isPaused() {
    return PAUSED;
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        isPlaying
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::isPlaying()
--  
--  RETURNS:         void
--  
--  NOTES:           This function returns if the audio is Playing.
------------------------------------------------------------------------------------------*/
bool AudioManager::isPlaying() {
    return PLAYING;
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        ~AudioManager
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void AudioManager::~NetworkAudioPlayer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function stops playing audio from the QAudioOutput player and destroys it.
------------------------------------------------------------------------------------------*/
AudioManager::~AudioManager()
{
        audio = NULL;
        emit killPlayThread();
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        isPlaying
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void AudioManager::getAudioBuffer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function returns the audio buffer.
------------------------------------------------------------------------------------------*/
CircularBuffer * AudioManager::getAudioBuffer()
{
    return audioBuf;
}
