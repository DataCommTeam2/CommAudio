#include "networkaudioplayer.h"
#include "networkmanager.h"

NetworkAudioPlayer::NetworkAudioPlayer()
{
    second = false;

    audioBuffer = new CircularBuffer(DATA_BUFSIZE, MAX_BLOCKS);

}

void NetworkAudioPlayer::setParameters()
{
    deviceinfo = QAudioDeviceInfo::defaultOutputDevice();
    wav_hdr wavHeader;
    /*format.setSampleRate(wavHeader.SamplesPerSec);
    format.setChannelCount(wavHeader.NumOfChan);
    format.setSampleSize(wavHeader.bitsPerSample);*/
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);

    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    //outputBuffer[0].open(QIODevice::ReadOnly);
    //outputBuffer[1].open(QIODevice::ReadOnly);
    audio = new QAudioOutput(deviceinfo, format, this);
    audio->setVolume(1.0);
    audio->setNotifyInterval(50);
    audio->setBufferSize(DATA_BUFSIZE * 0.2 * MAX_BLOCKS);
    audioDevice = audio->start();
    audioDevice->open(QIODevice::ReadWrite);
    connect (audioDevice, &QIODevice::bytesWritten, this, &NetworkAudioPlayer::appendAudioData);

}

void NetworkAudioPlayer::playAudio()
{
    char * data = audioBuffer->cbRead(0.1 * MAX_BLOCKS);
}

void NetworkAudioPlayer::appendAudioData()
{
    /*if (bytesWritten == (DATA_BUFSIZE * 0.1 * MAX_BLOCKS))
    {

    }
    EnterCriticalSection(&bufferAccess);
    QByteArray * data = audioBuffer->cbRead(0.1 * MAX_BLOCKS);
    LeaveCriticalSection(&bufferAccess);

    if (second)
    {
        if (outputBuffer[1].isOpen())
        {
            outputBuffer[1].close();
        }
        outputBuffer[1].setBuffer(data);
        outputBuffer[1].open(QIODevice::ReadOnly);
        audioDevice->write(outputBuffer[1].buffer());
        second = false;
    } else {
        if (outputBuffer[0].isOpen())
        {
            outputBuffer[0].close();
        }
        outputBuffer[0].setBuffer(data);
        outputBuffer[0].open(QIODevice::ReadOnly);
        audioDevice->write(outputBuffer[0].buffer());
        second = true;
    }*/

    int emptyBytes = audio->bytesFree();
    //int periodSize = audio->periodSize();

    //int chunks = emptyBytes/periodSize;
    if (emptyBytes > DATA_BUFSIZE)
    {

    }
}

void NetworkAudioPlayer::stopAudio()
{
    disconnect(audioDevice, SIGNAL(aboutToClose()));
}
