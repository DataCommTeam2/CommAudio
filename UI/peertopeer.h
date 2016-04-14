#ifndef PEERTOPEER_H
#define PEERTOPEER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDir>
#include "peertopeer.h"
#include <QFile>
#include <QDir>
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include "audiomanager.h"
#include "localaudiomanager.h"
#include "audiothread.h"
#include "microphonemanager.h"
#include "networkmanager.h"
#include "networkaudioplayer.h"
#include "filemanager.h"

namespace Ui {
class PeerToPeer;
}


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          PeerToPeer
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       explicit PeerToPeer(QWidget *parent = 0);
--                   
--                   ~PeerToPeer();
--                   
--                   void on_sliderSound_actionTriggered(int action);
--                   
--                   void on_buttonConnect_released();
--                   
--                   void on_DataSendingButton_released();
--                   
--                   void on_buttonStopAudio_released();
--                   
--                   void on_buttonPauseAudio_released();
--                   
--                   void on_QueueAddButton_released();
--                   
--                   void playNextSong();
--                   
--                   void AddStatusMessage(const QString msg);
--                   
--                   void on_QueueRemoveButton_released();
--                   
--                   void successfulConnection(bool connected);
--                   
--                   void on_buttonDisconnect_released();
--                   
--                   void on_buttonPlay_released();
--                   
--                   void on_SendMicrophone_released();
--                   
--                   void on_OpenPathButton_released();
--                   
--                   void startP2P(const char * ip, int port);
--                   
--                   void checkQueue(QAudioOutput * audioOut);
--                   
--                   void startTCP(int port);
--                   
--                   void on_requestFileButton_released();
--                   
--                   void sendData(char * buffer, int length);
--                   
--                   void stopMicrophoneRecording();
--                   
--                   void stopAudio();
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  NOTES:           This is the UI class for the Peer to Peer window. This class handles all interactions
--                   with the peer to peer window.
------------------------------------------------------------------------------------------*/
class PeerToPeer : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeerToPeer(QWidget *parent = 0);
    ~PeerToPeer();

private slots:
    void on_sliderSound_actionTriggered(int action);

    void on_buttonConnect_released();

    void on_DataSendingButton_released();

    void on_buttonStopAudio_released();

    void on_buttonPauseAudio_released();

    void on_QueueAddButton_released();

    void playNextSong();

    void AddStatusMessage(const QString msg);

    void on_QueueRemoveButton_released();

    void successfulConnection(bool connected);

    void on_buttonDisconnect_released();

    void on_buttonPlay_released();

    void on_SendMicrophone_released();

    void on_OpenPathButton_released();
    void startP2P(const char * ip, int port);

    void checkQueue(QAudioOutput * audioOut);
    void startTCP(int port);

    void on_requestFileButton_released();

public slots:
    void sendData(char * buffer, int length);

signals:
    void stopMicrophoneRecording();
    void stopAudio();
    void getFileFromPeer(FILE *);
private:
    MicrophoneManager *mic;
    Ui::PeerToPeer *ui;
    bool isDataSending = true;
    AudioManager *audioManager;
    LocalAudioManager *localAudioManager;
    int currentQueueIndex;
    AudioThread *deviceListener;
    bool stopThreadLoop = false;
    bool isMicrophoneSending = false;
    QThread *audioThread;
    QThread *socketThread;
    NetworkManager * networkManager;
    AudioPlayThread * bufferListener;
    NetworkAudioPlayer * netAudioPlayer;
    FileManager * fileManager;
};

#endif // PEERTOPEER_H

