#include "peertopeer.h"
#include "ui_peertopeer.h"
#include <QtDebug>

QThread * audioSendThread;

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

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        PeerToPeer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       PeerToPeer::PeerToPeer(QWidget *parent)
--  
--  RETURNS:         void
--  
--  NOTES:           Constructor for the peer to peer UI class. This constructor will setup files to read
--                   and set them up for the user to browse. It will also create necessary objects to 
--                   play and stream audio.
------------------------------------------------------------------------------------------*/
PeerToPeer::PeerToPeer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PeerToPeer)
{
    ui->setupUi(this);
    if (!QDir(QDir::currentPath() + "/MusicFiles").exists())
        QDir().mkdir(QDir::currentPath() + "/MusicFiles");
    audioManager = new AudioManager(this);
    localAudioManager = new LocalAudioManager(this);
    QDir dir = (QDir::currentPath() + "/MusicFiles/");
    QStringList locals = dir.entryList(QStringList("*.wav"));
    for (int i = 0; i < locals.length(); i++)
        ui->listMusicFiles->addItem(QDir::currentPath() + "/MusicFiles/" + locals.at(i));
    currentQueueIndex = -1;
    networkManager = new NetworkManager();
    networkManager->startNetwork();

    fileManager = new FileManager(networkManager);
    QThread * fileCheckerThread = new QThread();
    fileManager->moveToThread(fileCheckerThread);
    connect(fileCheckerThread, SIGNAL(started()), fileManager, SLOT(checkBuffer()));
    connect(fileManager, SIGNAL(fileDone()), fileCheckerThread, SLOT(quit()));
    connect(fileManager, SIGNAL(dataRead()), fileManager, SLOT(checkBuffer()));
    connect(this, SIGNAL(getFileFromPeer(FILE *)), fileManager, SLOT(requestFile(FILE *)));
    fileCheckerThread->start();

    netAudioPlayer = NULL;

    ui->controlsFrame->hide();
    ui->fileTransferControls->hide();
    ui->audioControlsFrame->hide();
    ui->StatusBar->show();
    ui->StatusBar->setMaximumHeight(1000);
    ui->buttonDisconnect->hide();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        startTCP
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void PeerToPeer::startTCP(int port)
--  
--  RETURNS:         void
--  
--  NOTES:           This function starts a TCP connection.
------------------------------------------------------------------------------------------*/
void PeerToPeer::startTCP(int port)
{
    networkManager->createTCPSocket();
    networkManager->startTCPReceiver(port);
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        startP2P
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  INTERFACE:       void PeerToPeer::startP2P(const char * ip, int port)
--  
--  RETURNS:         void
--  
--  NOTES:           This function starts a peer to peer connection using UDP.
------------------------------------------------------------------------------------------*/
void PeerToPeer::startP2P(const char * ip, int port)
{
    //start UDP receiver and sender
    // 1 UDP socket for each????
    if (!networkManager->setupUDPforP2P(ip, port))
    {
        return;
    }

    QThread * playThread = new QThread();
    //audioManager->setupAudioPlayerNoFile(NetworkManager::incBuffer);
    audioManager->setupAudioPlayerP2P(NetworkManager::incBuffer);
    networkManager->startUDPReceiver(NULL);

    bufferListener = new AudioPlayThread(NetworkManager::incBuffer);
    bufferListener->moveToThread(playThread);

    connect (playThread, SIGNAL(started()), bufferListener, SLOT(checkBuffer()));
    connect( bufferListener, SIGNAL(bufferHasData()), audioManager, SLOT(writeDataToDevice()));
    connect( audioManager, SIGNAL(finishedWriting()), bufferListener, SLOT(checkBuffer()));

    playThread->start();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        ~PeerToPeer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::~PeerToPeer();
--  
--  RETURNS:         void
--  
--  NOTES:            Destructor for the peer to peer window class.
------------------------------------------------------------------------------------------*/
PeerToPeer::~PeerToPeer()
{
    delete audioManager;
    delete ui;
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
--  INTERFACE:       void PeerToPeer::on_sliderSound_actionTriggered(int action)
--  
--  RETURNS:         void
--  
--  NOTES:           This function is called when the user moves the sound slider, it'll adjust the
--                   local program sound.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_sliderSound_actionTriggered(int action)
{
    audioManager->setVolume((double)ui->sliderSound->sliderPosition() / 100);
    localAudioManager->setVolume((double)ui->sliderSound->sliderPosition() / 100);
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
--  INTERFACE:       void PeerToPeer::on_buttonConnect_released()
--  
--  RETURNS:         void
--  
--  NOTES:           Triggered on connect button clicked. It will check to see if the fields.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_buttonConnect_released()
{
    if (ui->lineIPAddress->text().length() <= 0)
    {
        QMessageBox::information(this, tr("Peer to Peer Audio"), tr("Enter an IP!"));
            return;
    }

    if (ui->linePort->text().length() <= 0) {
        QMessageBox::information(this, tr("Peer to Peer Audio"), tr("Enter a port number!"));
        return;
    }

    std::string ip(ui->lineIPAddress->text().toUtf8().constData());
    int port = atoi(ui->lineUDPPort->text().toUtf8().constData());
    int tcpPort = atoi(ui->linePort->text().toUtf8().constData());

    // ---- TODO ---- handle connecting to the peer here, use the above 2 strings as parameters for connection
    AddStatusMessage("Attempting to Connect...");
    startTCP(tcpPort);
    startP2P(ip.c_str(), port);
    successfulConnection(true);
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
--  INTERFACE:       void PeerToPeer::on_buttonStopAudio_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the stop button, stop the audio, and stop the queue
--                   thread from attempting to play the next song.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_buttonStopAudio_released()
{
    localAudioManager->stopAudio();
    stopThreadLoop = true;
    currentQueueIndex--;
    if (!audioManager->stopAudio())
        return;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_buttonPauseAudio_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::on_buttonPauseAudio_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the pause button, pause the audio playing.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_buttonPauseAudio_released()
{
    if (localAudioManager->isPlaying())
        localAudioManager->pauseAudio();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_QueueAddButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::on_QueueAddButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the queue add button, it'll add the selected 
--                   items to the queue list to play.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_QueueAddButton_released()
{
    QList<QListWidgetItem *> selectedFile = ui->listMusicFiles->selectedItems();
    if (selectedFile.size() < 1)
        return;
    QListWidgetItem * index = selectedFile.front();
    ui->listQueueFiles->addItem(index->text());
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_QueueRemoveButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::on_QueueRemoveButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will remove the selected songs from the queue list when
--                   the user clicks the remove queue button.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_QueueRemoveButton_released()
{
    QList<QListWidgetItem *> indexes = ui->listQueueFiles->selectedItems();
    qDeleteAll(indexes.begin(), indexes.end());
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
--  INTERFACE:       void PeerToPeer::on_buttonPlay_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function catches a click on the play audio button. If the button is clicked,
--                   it will play the audio from where it left off if its paused, if not, it will play
--                   the next song in the list.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_buttonPlay_released()
{
    if (localAudioManager->isPaused()) {
        localAudioManager->playAudio();
    } else {
        playNextSong();
    }
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
--  INTERFACE:       void PeerToPeer::AddStatusMessage(const QString msg)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will add a status message to the status bar.
------------------------------------------------------------------------------------------*/
void PeerToPeer::AddStatusMessage(const QString msg) {
    if (!stopThreadLoop)
        ui->StatusBar->addItem(QString(msg));
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
--  INTERFACE:       void PeerToPeer::successfulConnection(bool connected)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will be called by the network layer to notify the application layer
--                   with a confirmation message on a successful connection.
--                   param bool connected = if true, successful connection, else, connection failed.
------------------------------------------------------------------------------------------*/
void PeerToPeer::successfulConnection(bool connected) {
    if (connected) {
        ui->fileTransferControls->show();
        ui->controlsFrame->show();
        ui->audioControlsFrame->show();
        ui->IPControls->hide();
        ui->PortControls->hide();
        ui->buttonConnect->hide();
        ui->welcomeLabel->hide();
        ui->buttonDisconnect->show();
        ui->connectionControls->setMaximumHeight(70);
        ui->StatusBar->setMaximumHeight(200);
        ui->portControlsUDP->hide();
        AddStatusMessage("Connection Successful!");
    } else
        AddStatusMessage("Unable to connect to peer.");
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
--  INTERFACE:       void PeerToPeer::on_buttonDisconnect_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will disconnect the user when they click the disconnect button.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_buttonDisconnect_released()
{
    // ---- TODO ---- disconnect this peer here.
    AddStatusMessage("Disconnected from peer.");
    ui->fileTransferControls->hide();
    ui->controlsFrame->hide();
    ui->audioControlsFrame->hide();
    ui->IPControls->show();
    ui->PortControls->show();
    ui->buttonConnect->show();
    ui->StatusBar->show();
    ui->welcomeLabel->show();
    ui->buttonDisconnect->hide();
    ui->portControlsUDP->show();
    ui->StatusBar->setMaximumHeight(1000);
    ui->connectionControls->setMaximumHeight(250);
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_SendMicrophone_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::on_SendMicrophone_released()
--  
--  RETURNS:         void
--  
--  NOTES:           were no longer sending microphone data
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_SendMicrophone_released()
{
    if (isMicrophoneSending) {
        //were no longer sending microphone data
        emit stopMicrophoneRecording();
        ui->SendMicrophone->setText("Start Recording Microphone");
    } else {
        //were are now sending microphone data
        mic = new MicrophoneManager(this, networkManager);
        mic->RecordAudio();
        connect(this, SIGNAL(stopMicrophoneRecording()), mic, SLOT(stopRecording()));
        if (isDataSending)
            on_DataSendingButton_released();
        ui->SendMicrophone->setText("Stop Recording Microphone");
    }
    isMicrophoneSending = !isMicrophoneSending;
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_DataSendingButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::on_DataSendingButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           When the user clicks the data sending button, this button will trigger on
--                   and off for sending data.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_DataSendingButton_released()
{
    if (ui->listQueueFiles->count() <= 0) {
        AddStatusMessage("No songs in queue.");
        return;
    }

    // ---- TODO ---- Add starting and stopping of data here.
    if (isDataSending) {
        isDataSending = false;
        //ui->DataSendingButton->setText("Start Sending Data");
        return;
    } else
        //ui->DataSendingButton->setText("Stop Sending Data");
    isDataSending = !isDataSending;

    for (int i = 0; i < ui->listQueueFiles->count(); i++) {
        ui->listQueueFiles->item(i)->setBackgroundColor(Qt::white);
    }

    currentQueueIndex++;

    //go back to beginning of the list
    if (currentQueueIndex > ui->listQueueFiles->count() - 1) {
        currentQueueIndex = 0;
    }

    QListWidgetItem * current = ui->listQueueFiles->item(currentQueueIndex);
    current->setBackgroundColor(Qt::green);

    if (audioSendThread && audioSendThread->isRunning())
    {
        audioSendThread->terminate();
        delete audioSendThread;
        audioSendThread = NULL;
    }
    audioSendThread = new QThread();

    if (netAudioPlayer == NULL)
    {
        netAudioPlayer = new NetworkAudioPlayer();

    }
    netAudioPlayer->setup(new QFile(current->text()));
    netAudioPlayer->moveToThread(audioSendThread);
    //QAudioOutput * audioOut = netAudioPlayer->playAudio(netManager);

    connect(audioSendThread, SIGNAL(started()), netAudioPlayer, SLOT(playAudio()));
    connect(netAudioPlayer, SIGNAL(audioStarted(QAudioOutput*)), this, SLOT(checkQueue(QAudioOutput*)));
    connect(netAudioPlayer, SIGNAL(sendToClient(char*,int)), this, SLOT(sendData(char*, int)));
    audioSendThread->start();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        sendData
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::sendData(char * buffer, int length)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will send data accross the P2P connection to the other client.
------------------------------------------------------------------------------------------*/
void PeerToPeer::sendData(char * buffer, int length)
{
    networkManager->sendP2P(buffer, length);
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        checkQueue
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::checkQueue(QAudioOutput * audioOut)
--  
--  RETURNS:         void
--  
--  NOTES:           This function checks the playlist for songs and resets the audio manager for
--                   the particular song.
------------------------------------------------------------------------------------------*/
void PeerToPeer::checkQueue(QAudioOutput * audioOut)
{
    QThread * queueThread = new QThread();
    deviceListener = new AudioThread(audioOut);
    deviceListener->moveToThread(queueThread);
    connect( queueThread, SIGNAL(started()), deviceListener, SLOT(checkForEnding()) );
    connect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(AddStatusMessage(QString)) );
    connect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(playNextSong()) );
    connect( deviceListener, SIGNAL(workFinished(const QString)), queueThread, SLOT(quit()) );
    //automatically delete thread and deviceListener object when work is done:
    connect( queueThread, SIGNAL(finished()), deviceListener, SLOT(deleteLater()) );
    connect( queueThread, SIGNAL(finished()), queueThread, SLOT(deleteLater()) );
    queueThread->start();
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        playNextSong
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void PeerToPeer::playNextSong()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will play the next song in the list. It is auto triggered if it finds the
--                   currently playing song has finished.
------------------------------------------------------------------------------------------*/
void PeerToPeer::playNextSong() {
    if (stopThreadLoop) {
        disconnect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(AddStatusMessage(QString)) );
        disconnect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(playNextSong()) );
        stopThreadLoop = false;
        return;
    }

    if (ui->listQueueFiles->count() <= 0) {
        AddStatusMessage("No songs in queue.");
        return;
    }

    for (int i = 0; i < ui->listQueueFiles->count(); i++) {
        ui->listQueueFiles->item(i)->setBackgroundColor(Qt::white);
    }

    currentQueueIndex++;

    //go back to beginning of the list
    if (currentQueueIndex > ui->listQueueFiles->count() - 1) {
        currentQueueIndex = 0;
    }

    QListWidgetItem * current = ui->listQueueFiles->item(currentQueueIndex);
    current->setBackgroundColor(Qt::green);


    localAudioManager->setupAudioPlayer(new QFile(current->text()));
    QAudioOutput * audio = localAudioManager->playAudio();

    QThread *audioThread = new QThread( );
    deviceListener = new AudioThread(audio);
    deviceListener->moveToThread(audioThread);

    connect( audioThread, SIGNAL(started()), deviceListener, SLOT(checkForEnding()) );
    connect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(AddStatusMessage(QString)) );
    connect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(playNextSong()) );
    //automatically delete thread and deviceListener object when work is done:
    connect( audioThread, SIGNAL(finished()), deviceListener, SLOT(deleteLater()) );
    connect( audioThread, SIGNAL(finished()), audioThread, SLOT(deleteLater()) );
    audioThread->start();
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_OpenPathButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       PeerToPeer::on_OpenPathButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           PeerToPeer::on_OpenPathButton_released()
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_OpenPathButton_released()
{
    ui->listMusicFiles->addItems(QFileDialog::getOpenFileNames(this, tr("Open Wav files."),
                                                    QDir::currentPath() + "/MusicFiles",
                                                    tr("Wav Files (*.wav)")));
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_requestFileButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       PeerToPeer::on_requestFileButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function handles adding new files to the files list.
------------------------------------------------------------------------------------------*/
void PeerToPeer::on_requestFileButton_released()
{
    if (!NetworkManager::tcpConnected)
    {
        std::string ip = ui->lineIPAddress->text().toStdString();
        int tcpPort = atoi(ui->linePort->text().toUtf8().constData());

        // attempt to connect to peer via TCP
        if (networkManager->connectToPeer(ip.c_str(), tcpPort))
        {
            AddStatusMessage("Successfully connected to peer.");
        } else {
            AddStatusMessage("Connection to peer failed.");
        }
    }
    //get filename from UI
    std::string name = ui->filenameEdit->text().toStdString();
    AddStatusMessage("Requesting file...");

    fileManager->stopChecking = true;
    QString absName = QDir::currentPath() + "/MusicFiles/" + name.c_str();
    FILE * fp = fopen(absName.toStdString().c_str(), "wb");

    emit getFileFromPeer(fp);
    char msg[256] = {0};
    msg[0] = 2;
    memcpy(&msg[1], name.c_str(),name.length());
    networkManager->sendViaTCP(msg, strlen(msg));
}
