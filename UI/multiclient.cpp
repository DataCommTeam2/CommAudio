#include "multiclient.h"
#include "ui_multiclient.h"

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


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        ~MultiClient
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::~MultiClient();
--  
--  RETURNS:         void
--  
--  NOTES:            Destructor for the MultiClient window class.
------------------------------------------------------------------------------------------*/
MultiClient::~MultiClient()
{
    delete ui;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_buttonPlay_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::on_buttonPlay_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function catches a click on the play audio button. If the button is clicked,
--                   it will play the audio from where it left off if its paused, if not, it will play
--                   the next song in the list.
------------------------------------------------------------------------------------------*/
void MultiClient::on_buttonPlay_released()
{
    audioManager->allowWrite = true;
    AddStatusMessage("Audio Started.");
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_buttonConnect_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::on_buttonConnect_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the data sending button, this button will trigger on
--                   and off for sending data.
------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        successfulConnection
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::successfulConnection(bool connected)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will be called by the network layer to notify the application layer
--                   with a confirmation message on a successful connection.
--                   param bool connected = if true, successful connection, else, connection failed.
------------------------------------------------------------------------------------------*/
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


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_buttonDisconnect_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::on_buttonDisconnect_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will disconnect the user when they click the disconnect button.
------------------------------------------------------------------------------------------*/
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


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_sliderSound_actionTriggered
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::on_sliderSound_actionTriggered(int action)
--  
--  RETURNS:         void
--  
--  NOTES:           This function is called when the user moves the sound slider, it'll adjust 
--                   the local program sound.
------------------------------------------------------------------------------------------*/
void MultiClient::on_sliderSound_actionTriggered(int action)
{
    audioManager->setVolume((double)ui->sliderSound->sliderPosition() / 100);
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        AddStatusMessage
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::AddStatusMessage(const QString msg)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will add a status message to the status bar.
------------------------------------------------------------------------------------------*/
void MultiClient::AddStatusMessage(const QString msg) {
    if (!stopThreadLoop)
        ui->StatusBar->addItem(QString(msg));
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_buttonStopAudio_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiClient::on_buttonStopAudio_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the stop button, stop the audio, and stop the queue
--                   thread from attempting to play the next song.
------------------------------------------------------------------------------------------*/
void MultiClient::on_buttonStopAudio_released()
{
    audioManager->allowWrite = false;
    AddStatusMessage("Audio Stopped.");
}
