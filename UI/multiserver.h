#ifndef MULTISERVER_H
#define MULTISERVER_H

#include <QMainWindow>
#include <QtMultimedia/QAudioOutput>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QMessageBox>
#include "audiomanager.h"
#include "audiothread.h"
#include "networkmanager.h"
#include "networkaudioplayer.h"
#include "microphonemanager.h"

namespace Ui {
class MultiServer;
}


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          MultiServer
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       explicit MultiServer(QWidget *parent = 0);
--                   
--                   ~MultiServer();
--                   
--                   void stopMicrophoneRecording();
--                   
--                   void on_QueueAddButton_released();
--                   
--                   void on_QueueRemoveButton_released();
--                   
--                   void playNextSong();
--                   
--                   void AddStatusMessage(QString msg);
--                   
--                   void successfulConnection(bool connected);
--                   
--                   void on_BroadcastButton_released();
--                   
--                   void on_SendMicrophone_released();
--                   
--                   void checkQueue(QAudioOutput * audioOut);
--                   
--                   void sendData(char * buffer, int length);
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
--  NOTES:           This is the UI class for the multicast server UI window. It handles all buttons
--                   and user interactions with the window.
------------------------------------------------------------------------------------------*/
class MultiServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MultiServer(QWidget *parent = 0);
    ~MultiServer();

signals:
    void stopMicrophoneRecording();

private slots:
    void on_QueueAddButton_released();

    void on_QueueRemoveButton_released();

    void playNextSong();

    void AddStatusMessage(QString msg);

    void successfulConnection(bool connected);

    void on_BroadcastButton_released();

    void on_SendMicrophone_released();

    void checkQueue(QAudioOutput * audioOut);

    void sendData(char * buffer, int length);

private:
    MicrophoneManager *mic;
    bool isDataSending = false;
    Ui::MultiServer *ui;
    AudioManager *audioManager;
    int currentQueueIndex;
    AudioThread *deviceListener;
    bool stopThreadLoop = false;
    NetworkAudioPlayer * netAudioPlayer;
    NetworkManager * netManager;
    bool isMicrophoneSending = false;
};

#endif // MULTISERVER_H

