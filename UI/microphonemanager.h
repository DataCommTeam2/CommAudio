#ifndef MICROPHONEMANAGER_H
#define MICROPHONEMANAGER_H

#include <QObject>
#include <QAudioInput>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QThread>
#include "audiorecordthread.h"
#include "networkmanager.h"

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          MicrophoneManager
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       MicrophoneManager(QObject *p, NetworkManager* netManager) : parent(p), networkManager(netManager)
--                   void RecordAudio();
--                   int RawToWavConvert(const char *rawfn, const char *wavfn, long frequency);
--                   void stopRecording();
--                   void sendData(char * data, int length);
--                   
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  NOTES:           This class is for recording audio from a microphone.
------------------------------------------------------------------------------------------*/
class MicrophoneManager : public QObject
{
    Q_OBJECT

public:

    MicrophoneManager(QObject *p, NetworkManager* netManager) : parent(p), networkManager(netManager){}
	
	
	/*--------------------------------------------------------------------------------------------  
	--  FUNCTION:        RecordAudio
	--  
	--  DATE:            April 14th, 2016
	--  
	--  DESIGNERS:       Jaegar Sarauer
	--  
	--  REVISIONS:       NONE
	--  
	--  PROGRAMMERS:     Jaegar Sarauer
	--  
	--  INTERFACE:       void MicrophoneManager::RecordAudio()
	--  
	--  RETURNS:         void
	--  
	--  NOTES:           This function begins recording audio from a connected microphone.
	------------------------------------------------------------------------------------------*/
    void RecordAudio();
	
	
	/*--------------------------------------------------------------------------------------------  
	--  FUNCTION:        RawToWavConvert
	--  
	--  DATE:            April 14th, 2016
	--  
	--  DESIGNERS:       
	--  
	--  REVISIONS:       NONE
	--  
	--  PROGRAMMERS:     
	--  
	--  INTERFACE:       int MicrophoneManager::RawToWavConvert(const char *rawfn, const char *wavfn, long frequency) 
	--  
	--  RETURNS:         int
	--  
	--  NOTES:           This function converts raw audio format from the microphone to wav format to 
	--                   be reloaded and played locally or resent over the network.
	------------------------------------------------------------------------------------------*/
    int RawToWavConvert(const char *rawfn, const char *wavfn, long frequency);

private slots:

	/*--------------------------------------------------------------------------------------------  
	--  FUNCTION:        stopRecording
	--  
	--  DATE:            April 14th, 2016
	--  
	--  DESIGNERS:       Jaegar Sarauer
	--  
	--  REVISIONS:       NONE
	--  
	--  PROGRAMMERS:     Jaegar Sarauer
	--  
	--  INTERFACE:       void MicrophoneManager::stopRecording()
	--  
	--  RETURNS:         void
	--  
	--  NOTES:           This function ends the audio recording when being called.
	------------------------------------------------------------------------------------------*/
    void stopRecording();
	
	/*--------------------------------------------------------------------------------------------  
	--  FUNCTION:        sendData
	--  
	--  DATE:            April 14th, 2016
	--  
	--  DESIGNERS:       Gabriella Cheung
	--  
	--  REVISIONS:       NONE
	--  
	--  PROGRAMMERS:     Gabriella Cheung
	--  
	--  INTERFACE:       void MicrophoneManager::sendData(char * data, int length)
	--  
	--  RETURNS:         void
	--  
	--  NOTES:           This function sends the microphone data to the network manager, to send the
	--                   data to clients.
	------------------------------------------------------------------------------------------*/
    void sendData(char * data, int length);

private:
    QObject *parent;
    QFile destinationFile;   // Class member
    QAudioInput* audio; // Class member

    QIODevice * audioDevice;
    QBuffer *buffer;
    NetworkManager * networkManager;
};

#endif // MICROPHONEMANAGER_H
