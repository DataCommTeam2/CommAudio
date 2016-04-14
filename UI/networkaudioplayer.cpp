#include "networkaudioplayer.h"
#include "networkmanager.h"
#include <QDebug>

#include <iostream>

QIODevice *aDevice;
QThread *aPlayThread;

NetworkAudioPlayer::NetworkAudioPlayer()
{
    audioBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);
    fp = NULL;

}

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

        //connect (audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(checkDeviceStatus(QAudio::State)));

        //connect( this, SIGNAL(finishedReading()), bufferListener, SLOT(stop()) );

        //connect(bufferListener, SIGNAL(finished()), aPlayThread, SLOT(quit()));

        //automatically delete thread and deviceListener object when work is done:
        //connect( aPlayThread, SIGNAL(finished()), bufferListener, SLOT(deleteLater()) );
        //connect( aPlayThread, SIGNAL(finished()), aPlayThread, SLOT(deleteLater()) );
        aPlayThread->start();

    } else {
        unpauseAudio();
    }
    //return audio;
    emit audioStarted(audio);
}

void NetworkAudioPlayer::writeDataToDevice()
{
    //send 8k of file
    /*char tempBuf[DATA_BUFSIZE];
    if (fp == NULL)
    {
        //std::string filen = file->fileName().toStdString();
        //std::cerr << filen;
        const char * filename = file->fileName().toStdString().c_str();
        fp = fopen("honey.wav", "rb");
        fseek(fp, 44*sizeof(char), SEEK_SET);
    }
    int bytesRead = fread(tempBuf, 1, DATA_BUFSIZE, fp);
    if (bytesRead == 0)
    {
        //emit finishedReading();
        //file->close();
        fclose(fp);
    }
    netManager->sendMulticast(tempBuf, bytesRead);*/
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

void NetworkAudioPlayer::unpauseAudio() {
    audio->resume();
}

NetworkAudioPlayer::~NetworkAudioPlayer() {
    file->close();
    delete audio;
}
