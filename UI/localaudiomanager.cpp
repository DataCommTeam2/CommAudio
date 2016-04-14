#include "localaudiomanager.h"
/*--------------------------------------------------------------------------------------------  
--  SOURCE:          LocalAudioManager
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       LocalAudioManager(QObject * p) : parent(p)
--                   QAudioOutput * playAudio();
--                   bool setupAudioPlayer(QFile * file);
--                   void setVolume(double volume);
--                   void stopAudio();
--                   void pauseAudio();
--                   void unpauseAudio();
--                   bool isPaused();
--                   bool isPlaying();
--                   ~LocalAudioManager();
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  NOTES:           This class is for playing audio from locally files. It has nothing to do with the
--                   network or the buffer, and opens files to read. It auto detects the wav header
--                   of the audio file, and uses QAudioOutput to play the audio.
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
--  INTERFACE:       bool LocalAudioManager::setupAudioPlayer(QFile * f)
--  
--  RETURNS:         bool
--  
--  NOTES:           This function sets up the network audio player, and gets ready to send the song
--                   along the network.
------------------------------------------------------------------------------------------*/
bool LocalAudioManager::setupAudioPlayer(QFile * f) {
    file = f;
    wav_hdr wavHeader;
    file->open(QIODevice::ReadOnly);
    int bytesRead = file->read((char*)&wavHeader, sizeof(wav_hdr));
    file->close();
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
    return true;
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
--  INTERFACE:       QAudioOutput *LocalAudioManager::playAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This fuction plays  the audio from the buffer. From whatever buffer data is available
--                   at the time. After sending the data to the QAudioOutput, the fuction requests for more
--                   data.
------------------------------------------------------------------------------------------*/
QAudioOutput *LocalAudioManager::playAudio() {
    if (!PAUSED) {
        file->open(QIODevice::ReadOnly);
        device = file;
        audio->start(device);
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
--  INTERFACE:       void LocalAudioManager::setVolume(double volume)
--  
--  RETURNS:         void
--  
--  NOTES:           This function sets the volume of the QAudioOutput playing the audio.
------------------------------------------------------------------------------------------*/
void LocalAudioManager::setVolume(double volume) {
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
--  INTERFACE:       void LocalAudioManager::stopAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will stop the audio player and clear the buffer from data to allow
--                   a new song to start.
------------------------------------------------------------------------------------------*/
void LocalAudioManager::stopAudio() {
    audio->stop();
    file->close();
    delete audio;
    PAUSED = false;
    PLAYING = false;
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
--  INTERFACE:       void LocalAudioManager::unpauseAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function pauses playing audio from the QAudioOutput player.
------------------------------------------------------------------------------------------*/
void LocalAudioManager::pauseAudio() {
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
--  INTERFACE:       void LocalAudioManager::unpauseAudio()
--  
--  RETURNS:         void
--  
--  NOTES:           This function resumes playing audio from the QAudioOutput player.
------------------------------------------------------------------------------------------*/
void LocalAudioManager::unpauseAudio() {
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
--  INTERFACE:       void LocalAudioManager::isPaused()
--  
--  RETURNS:         void
--  
--  NOTES:           This function returns if the audio is paused.
------------------------------------------------------------------------------------------*/
bool LocalAudioManager::isPaused() {
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
--  INTERFACE:       void LocalAudioManager::isPlaying()
--  
--  RETURNS:         void
--  
--  NOTES:           This function returns if the audio is Playing.
------------------------------------------------------------------------------------------*/
bool LocalAudioManager::isPlaying() {
    return PLAYING;
}
