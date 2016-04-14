#include "networkaudioplayer.h"
#include "networkmanager.h"
#include <QDebug>

#include <iostream>

QIODevice *aDevice;
QThread *aPlayThread;


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          NetworkAudioPlayer
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       NetworkAudioPlayer();
--                   bool setup(QFile *);
--                   void unpauseAudio();
--                   ~NetworkAudioPlayer();
--                   void finishedReading();
--                   void finishedWriting();
--                   void audioStarted(QAudioOutput *);
--                   void sendToClient(char *, int);
--                   void loadDataIntoBuffer();
--                   void writeDataToDevice();
--                   void playAudio();
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
--  NOTES:           This class handles playing audio that is retrieved from the network through a buffer.
--                   It uses QAudioOutput to play the audio on the client's side.
------------------------------------------------------------------------------------------*/
NetworkAudioPlayer::NetworkAudioPlayer()
{
    audioBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);
    fp = NULL;

}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        setup
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       bool NetworkAudioPlayer::setup(QFile * f)
--  
--  RETURNS:         bool
--  
--  NOTES:           This function sets up the network audio player, and gets ready to send the song
--                   along the network.
------------------------------------------------------------------------------------------*/
bool NetworkAudioPlayer::setup(QFile * f) {
    file = f;
    wav_hdr wavHeader;
    file->open(QIODevice::ReadOnly);
    int bytesRead = file->read((char*)&wavHeader, sizeof(wav_hdr));
    if (bytesRead <= 0)
        return false;


    format.setSampleRate(wavHeader.SamplesPerSec);
    format.setChannelCount(wavHeader.NumOfChan);
    format.setSampleSize(wavHeader.bitsPerSample);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    audio = new QAudioOutput(format, this);
    audio->setBufferSize(DATA_BUFSIZE * 10);
    //audio->setNotifyInterval(500);
    //audioDevice = audio->start();
    //audioDevice->open(QIODevice::ReadWrite);

    //connect (audio, SIGNAL(notify()), this, SLOT(sendData()));
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
--  INTERFACE:       void NetworkAudioPlayer::loadDataIntoBuffer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function loads the data from the file and puts it into the circular buffer.
------------------------------------------------------------------------------------------*/
void NetworkAudioPlayer::loadDataIntoBuffer()
{
    char tempBuf[DATA_BUFSIZE];
    if(!audioBuffer->isFull())
    {
        int bytesRead = file->read((char*)&tempBuf, sizeof(tempBuf));
        if (bytesRead <= 0)
        {
            emit finishedReading();
            //file->close();
        }
        audioBuffer->cbWrite(tempBuf, bytesRead);
        //emit finishedLoading();
    }
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        playAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--                   Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--                   Jaegar Sarauer
--  
--  INTERFACE:       void NetworkAudioPlayer::playAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This fuction plays  the audio from the buffer. From whatever buffer data is available
--                   at the time. After sending the data to the QAudioOutput, the fuction requests for more
--                   data.
------------------------------------------------------------------------------------------*/
void NetworkAudioPlayer::playAudio()
{
    //netManager = manager;
    if (!PAUSED) {
        if (!aPlayThread)
        {
            aPlayThread = new QThread( );
        } else {
            if (aPlayThread->isRunning())
            {
                //stop thread
                aPlayThread->terminate();
                delete aPlayThread;
                aPlayThread = new QThread();
            }
        }
        aDevice = NULL;
        bufferListener = new AudioPlayThread(audioBuffer);
        bufferListener->moveToThread(aPlayThread);

        connect( aPlayThread, SIGNAL(started()), this, SLOT(loadDataIntoBuffer()) );
        connect( aPlayThread, SIGNAL(started()), bufferListener, SLOT(checkBuffer()) );
        connect( this, SIGNAL(finishedWriting()), bufferListener, SLOT(checkBuffer()) );
        connect( bufferListener, SIGNAL(bufferHasData()), this, SLOT(writeDataToDevice()));
        aPlayThread->start();

    } else {
        unpauseAudio();
    }
    //return audio;
    emit audioStarted(audio);
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
--  INTERFACE:       void NetworkAudioPlayer::writeDataToDevice()
--  
--  RETURNS:         void
--  
--  NOTES:           This function attempts to send data from the buffer into the QAudioOutput file for local
--                   audio playing.
------------------------------------------------------------------------------------------*/
void NetworkAudioPlayer::writeDataToDevice()
{
    if (endThread)
        return;
    if (aDevice == NULL)
    {
        aDevice = audio->start();
    }
    int freeSpace = audio->bytesFree();

    if (file->atEnd())
    {
        file->close();
        //signal to peer2peer to load next file?????
    }

    if (freeSpace < DATA_BUFSIZE)
    {
        emit finishedWriting();
        return;
    } else {
        char * data = audioBuffer->cbRead(1);
        int length = audioBuffer->getLastBytesWritten();
        aDevice->write(data, length);
        audio->setVolume(0.0);
        emit sendToClient(data, length);
        emit finishedWriting();
        if (!file->atEnd())
        {
           loadDataIntoBuffer();
        } else {
            file->close();
            //signal to peer2peer to load next file?????
        }
    }
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        unpauseAudio
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void NetworkAudioPlayer::unpauseAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function resumes playing audio from the QAudioOutput player.
------------------------------------------------------------------------------------------*/
void NetworkAudioPlayer::unpauseAudio() {
    audio->resume();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        NetworkAudioPlayer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void NetworkAudioPlayer::~NetworkAudioPlayer()
--  
--  RETURNS:         void
--  
--  NOTES:           This function stops playing audio from the QAudioOutput player and destroys it.
------------------------------------------------------------------------------------------*/
NetworkAudioPlayer::~NetworkAudioPlayer() {
    file->close();
    delete audio;
}
