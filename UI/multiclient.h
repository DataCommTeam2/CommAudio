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

    void on_buttonPauseAudio_released();

    void on_buttonStopAudio_released();

    void successfulConnection(bool connected);

    void AddStatusMessage(const QString msg);

private:
    Ui::MultiClient *ui;
    AudioManager *audioManager;
    AudioThread *deviceListener;
    bool stopThreadLoop = false;
    NetworkManager * netManager;
    AudioPlayThread * bufferListener;
};

#endif // MULTICLIENT_H
