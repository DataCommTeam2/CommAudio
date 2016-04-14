#include "multiclient.h"
#include "ui_multiclient.h"


MultiClient::MultiClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MultiClient)
{
    ui->setupUi(this);
    audioManager = new AudioManager(this);
    ui->IPControls->show();
    ui->PortControls->show();
    ui->buttonConnect->show();
    ui->welcomeLabel->show();
    ui->buttonDisconnect->hide();
    ui->AudioControls->hide();
    ui->ConnectionControls->setMaximumHeight(250);
}

MultiClient::~MultiClient()
{
    delete ui;
}

void MultiClient::on_buttonPlay_released()
{
    audioManager->allowWrite = true;
    AddStatusMessage("Audio Started.");
}




void MultiClient::on_buttonConnect_released()
{
    CircularBuffer * incomingBuffer;
    if (!netManager->startNetwork())
    {
        successfulConnection(false);
        return;
    }
    std::string serverAddr = ui->lineIPAddress->text().toStdString();
    int port = ui->linePort->text().toInt();
    if (!netManager->createMulticastClientSocket(serverAddr.c_str(), port))
    {
        successfulConnection(false);
        return;
    }
    successfulConnection(true);

    playThread = new QThread();
    audioManager->setupAudioPlayerNoFile(NetworkManager::incBuffer);
    //audioManager->setupAudioPlayerP2P(NetworkManager::incBuffer);
    //incomingBuffer = audioManager->getAudioBuffer();
    //netManager->startUDPReceiver(incomingBuffer); //pass in circular buffer here
    netManager->startUDPReceiver(NULL);

    bufferListener = new AudioPlayThread(NetworkManager::incBuffer);
    bufferListener->moveToThread(playThread);

    connect (playThread, SIGNAL(started()), bufferListener, SLOT(checkBuffer()));
    connect( bufferListener, SIGNAL(bufferHasData()), audioManager, SLOT(writeDataToDevice()));
    connect( audioManager, SIGNAL(finishedWriting()), bufferListener, SLOT(checkBuffer()));

    playThread->start();
}

/*
 * This function will be called by the network layer to notify the application layer
 * with a confirmation message on a successful connection.
 * param bool connected = if true, successful connection, else, connection failed.
 */
// ---- TODO ---- call this function on successful connection
void MultiClient::successfulConnection(bool connected) {
    if (connected) {
        ui->IPControls->hide();
        ui->PortControls->hide();
        ui->AudioControls->show();
        ui->buttonConnect->hide();
        ui->welcomeLabel->hide();
        ui->buttonDisconnect->show();
        ui->ConnectionControls->setMaximumHeight(70);
        AddStatusMessage("Connection Successful!");
    } else
        AddStatusMessage("Unable to connect to server.");
}

void MultiClient::on_buttonDisconnect_released()
{
    // ---- TODO ---- disconnect this client here.
    AddStatusMessage("Disconnected from server.");
    ui->IPControls->show();
    ui->PortControls->show();
    ui->buttonConnect->show();
    ui->welcomeLabel->show();
    ui->buttonDisconnect->hide();
    ui->AudioControls->hide();
    ui->ConnectionControls->setMaximumHeight(250);
    //netManager->cleanUp();
    //audioManager->allowWrite = false;
}


/*
 * This function is called when the user moves the sound slider, it'll adjust the local program sound.
 */
void MultiClient::on_sliderSound_actionTriggered(int action)
{
    audioManager->setVolume((double)ui->sliderSound->sliderPosition() / 100);
}

/*
 * This function will add a status message to the status bar.
 */
void MultiClient::AddStatusMessage(const QString msg) {
    if (!stopThreadLoop)
        ui->StatusBar->addItem(QString(msg));
}

void MultiClient::on_buttonStopAudio_released()
{
    audioManager->allowWrite = false;
    AddStatusMessage("Audio Stopped.");
}
