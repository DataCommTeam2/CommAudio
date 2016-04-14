#include "microphonemanager.h"
#include <stdio.h>


void MicrophoneManager::RecordAudio()
{
    destinationFile.setFileName(QDir::currentPath() + "/MusicFiles/" + "testrec2.raw");
    destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    audio = new QAudioInput(format, parent);
    //connect(audio, SIGNAL(stateChanged(QAudio::State)), parent, SLOT(handleStateChanged(QAudio::State)));
    buffer = new QBuffer();
    if (!buffer->open(QIODevice::ReadWrite))
    {
        qWarning("unable to open buffer");
    }

    //audio->start(&destinationFile);

    audio->start(buffer);
    audioDevice = buffer;
    QThread * recordThread = new QThread();
    AudioRecordThread * micListener = new AudioRecordThread(audioDevice);
    micListener->moveToThread(recordThread);

    connect(audioDevice, SIGNAL(readyRead()), micListener, SLOT(checkMicrophone()));
    connect(micListener, SIGNAL(sendDataFromMic(char *, int)), this, SLOT(sendData(char *, int)));
    recordThread->start();
}
void MicrophoneManager::sendData(char * data, int length)
{
    networkManager->sendP2P(data, length);
}

int MicrophoneManager::RawToWavConvert(const char *rawfn, const char *wavfn, long frequency) {
   long chunksize=0x10;

   struct
   {
       unsigned short    wFormatTag;
       unsigned short    wChannels;
       unsigned long     dwSamplesPerSec;
       unsigned long     dwAvgBytesPerSec;
       unsigned short    wBlockAlign;
       unsigned short    wBitsPerSample;
   } fmt;

   FILE *raw = fopen(rawfn,"rb");
   if(!raw)
       return -2;

   fseek(raw, 0, SEEK_END);
   long bytes = ftell(raw);
   fseek(raw, 0, SEEK_SET);

   long samplecount = bytes/2;
   long riffsize    = samplecount*2+0x24;
   long datasize    = samplecount*2;

   FILE *wav=fopen(wavfn,"wb");
   if(!wav)
   {
       fclose(raw);
       return -3;
   }

   fwrite( "RIFF",     1, 4, wav );
   fwrite( &riffsize,  4, 1, wav );
   fwrite( "WAVEfmt ", 1, 8, wav );
   fwrite( &chunksize, 4, 1, wav );

   fmt.wFormatTag = 1;      // PCM
   fmt.wChannels  = 1;      // MONO
   fmt.dwSamplesPerSec  = frequency*1;
   fmt.dwAvgBytesPerSec = frequency*1*2; // 16 bit
   fmt.wBlockAlign      = 2;
   fmt.wBitsPerSample   = 8;

   fwrite( &fmt,      sizeof(fmt), 1, wav );
   fwrite( "data",    1,           4, wav );
   fwrite( &datasize, 4,           1, wav );
   short buff[1024];
   while( !feof(raw) )
   {
         int cnt=fread(buff,2,1024,raw);
         if( cnt == 0 )
             break;
         fwrite(buff,2,cnt,wav);
   }
   fclose( raw );
   fclose( wav );
}

void MicrophoneManager::stopRecording()
{
    if (audio != NULL) {
        audio->stop();
        destinationFile.close();
        delete audio;
        audio = NULL;
    }
    RawToWavConvert((QDir::currentPath() + "/MusicFiles/" + "testrec2.raw").toStdString().c_str(), (QDir::currentPath() + "/MusicFiles/" + "testrec2.wav").toStdString().c_str(), 8000);
}
