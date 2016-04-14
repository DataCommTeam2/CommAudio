#ifndef MULTICLIENT_H
#define MULTICLIENT_H

#include <QMainWindow>
#include <QAudio>
#include "audiomanager.h"
#include "audiothread.h"
#include "networkmanager.h"

namespace Ui {
class MultiClient;
}

/*--------------------------------------------------------------------------------------------  
--  SOURCE:          MultiClient
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       explicit MultiClient(QWidget *parent = 0);
--                   
--                   ~MultiClient();
--                   
--                   void on_buttonPlay_released();
--                   
--                   void on_buttonConnect_released();
--                   
--                   void on_buttonDisconnect_released();
--                   
--                   void on_buttonStopAudio_released();
--                   
--                   void successfulConnection(bool connected);
--                   
--                   void AddStatusMessage(const QString msg);
--                   
--                   void on_sliderSound_actionTriggered(int action);
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
--  NOTES:           This is the UI class for the multicast client UI window. It handles all buttons
--                   and user interactions with the window.
------------------------------------------------------------------------------------------*/
class MultiClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit MultiClient(QWidget *parent = 0);
    ~MultiClient();

private slots:
    void on_buttonPlay_released();
    //void handleStateChanged(QAudio::State ok);

    void on_buttonConnect_released();

    void on_buttonDisconnect_released();

    void on_buttonStopAudio_released();

    void successfulConnection(bool connected);

    void AddStatusMessage(const QString msg);

    void on_sliderSound_actionTriggered(int action);

private:
    Ui::MultiClient *ui;
    AudioManager *audioManager;
    AudioThread *deviceListener;
    bool stopThreadLoop = false;
    NetworkManager * netManager;
    AudioPlayThread * bufferListener;
    QThread * playThread;
};

#endif // MULTICLIENT_H
