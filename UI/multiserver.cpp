#include "multiserver.h"
#include "ui_multiserver.h"

QThread * audioSenderThread;

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        MultiServer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       MultiServer::MultiServer(QWidget *parent)
--  
--  RETURNS:         void
--  
--  NOTES:           Constructor for the multiserver window class.
------------------------------------------------------------------------------------------*/
MultiServer::MultiServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MultiServer)
{
    ui->setupUi(this);
    if (!QDir(QDir::currentPath() + "/MusicFiles").exists())
        QDir().mkdir(QDir::currentPath() + "/MusicFiles");
    audioManager = new AudioManager(this);
    QDir dir = (QDir::currentPath() + "/MusicFiles/");
    QStringList locals = dir.entryList(QStringList("*.wav"));
    for (int i = 0; i < locals.length(); i++)
        ui->listMusicFiles->addItem(QDir::currentPath() + "/MusicFiles/" + locals.at(i));
    currentQueueIndex = -1;
    //netAudioPlayer = new NetworkAudioPlayer();
    netAudioPlayer = NULL;
    netManager = new NetworkManager();
}

/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        ~MultiServer
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       MultiServer::~MultiServer()
--  
--  RETURNS:         void
--  
--  NOTES:           Destructor for the multi server window class.
------------------------------------------------------------------------------------------*/
MultiServer::~MultiServer()
{
    stopThreadLoop = true;
    if (deviceListener)
        deviceListener->audioPlayer = NULL;
    if (netAudioPlayer)
    netAudioPlayer->endThread = true;
    delete ui;
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
--  INTERFACE:       void MultiServer::on_QueueAddButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will add a song the the playlist queue.
------------------------------------------------------------------------------------------*/
void MultiServer::on_QueueAddButton_released()
{
    QList<QListWidgetItem *> selectedFile = ui->listMusicFiles->selectedItems();
    if (selectedFile.size() < 1)
        return;
    QListWidgetItem * index = selectedFile.front();
    ui->listQueueFiles->addItem(index->text());
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
--  INTERFACE:       void MultiServer::playNextSong() 
--  
--  RETURNS:         void
--  
--  NOTES:           This function will play the next song in the list. It is auto triggered if it finds the
--                   currently playing song has finished.
------------------------------------------------------------------------------------------*/
void MultiServer::playNextSong() {
    if (stopThreadLoop) {
        if (!deviceListener && !netAudioPlayer)
            return;
        disconnect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(AddStatusMessage(QString)) );
        disconnect( deviceListener, SIGNAL(workFinished(const QString)), this, SLOT(playNextSong()) );

            stopThreadLoop = false;
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

    if (audioSenderThread && audioSenderThread->isRunning())
    {
        audioSenderThread->terminate();
        delete audioSenderThread;
        audioSenderThread = NULL;
    }
    audioSenderThread = new QThread();

    //if (netAudioPlayer != NULL)
        //delete netAudioPlayer;

    netAudioPlayer = new NetworkAudioPlayer();

    //}
    netAudioPlayer->setup(new QFile(current->text()));
    netAudioPlayer->moveToThread(audioSenderThread);
    //QAudioOutput * audioOut = netAudioPlayer->playAudio(netManager);

    connect(audioSenderThread, SIGNAL(started()), netAudioPlayer, SLOT(playAudio()));
    connect(netAudioPlayer, SIGNAL(audioStarted(QAudioOutput*)), this, SLOT(checkQueue(QAudioOutput*)));
    connect(netAudioPlayer, SIGNAL(sendToClient(char*,int)), this, SLOT(sendData(char*, int)));
    audioSenderThread->start();
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
--  INTERFACE:       void MultiServer::sendData(char * buffer, int length)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will send data accross the P2P connection to the other client.
------------------------------------------------------------------------------------------*/
void MultiServer::sendData(char * buffer, int length)
{
    netManager->sendMulticast(buffer, length);
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
--  INTERFACE:       void MultiServer::checkQueue(QAudioOutput * audioOut)
--  
--  RETURNS:         void
--  
--  NOTES:           This function checks the playlist for songs and resets the audio manager for
--                   the particular song.
------------------------------------------------------------------------------------------*/
void MultiServer::checkQueue(QAudioOutput * audioOut)
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
--  INTERFACE:       void MultiServer::AddStatusMessage(const QString msg)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will add a status message to the status bar.
------------------------------------------------------------------------------------------*/
void MultiServer::AddStatusMessage(const QString msg) {
    if (!stopThreadLoop)
        ui->StatusBar->addItem(QString(msg));
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
--  INTERFACE:       void MultiServer::on_QueueRemoveButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will remove the selected songs from the queue list when
--                   the user clicks the remove queue button.
------------------------------------------------------------------------------------------*/
void MultiServer::on_QueueRemoveButton_released()
{
    QList<QListWidgetItem *> indexes = ui->listQueueFiles->selectedItems();
    qDeleteAll(indexes.begin(), indexes.end());
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
--  INTERFACE:       void MultiServer::successfulConnection(bool connected)
--  
--  RETURNS:         void
--  
--  NOTES:           This function will be called by the network layer to notify the application layer
--                   with a confirmation message on a successful connection.
--                   param bool connected = if true, successful connection, else, connection failed.
------------------------------------------------------------------------------------------*/
void MultiServer::successfulConnection(bool connected) {
    if (connected)
        AddStatusMessage("Connection Successful!");
    else
        AddStatusMessage("Unable to connect to peer.");
}


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_BroadcastButton_released
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void MultiServer::on_BroadcastButton_released()
--  
--  RETURNS:         void
--  
--  NOTES:           This function will send data accross the P2P connection to the other client.
------------------------------------------------------------------------------------------*/
void MultiServer::on_BroadcastButton_released()
{
    if (isDataSending) {
        ui->BroadcastButton->setText("Broadcast");
        isDataSending = false;
        stopThreadLoop = true;
        if (deviceListener)
            deviceListener->forceKill = true;
        if (netAudioPlayer)
        netAudioPlayer->endThread = true;
        return;
    } else {
        if (isMicrophoneSending) {
            emit stopMicrophoneRecording();
            isMicrophoneSending = false;
        }
        ui->BroadcastButton->setText("Stop Broadcasting");
        isDataSending = true;
    }

    if (ui->listQueueFiles->count() <= 0) {
        AddStatusMessage("No songs in queue.");
        return;
    }

    if (!netManager->startNetwork())
    {
        return;
    }
    int port = ui->linePort->text().toInt();
    switch (netManager->createMulticastServerSocket(ui->IPLine->text().toStdString().c_str(), port)) {
    case 0:
        AddStatusMessage("Invalid connection attempt.");
        return;
    case 1:
        AddStatusMessage("Successful Connection!");
        break;
    }

    playNextSong();

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
--  INTERFACE:       void MultiServer::on_SendMicrophone_released()
--  
--  RETURNS:         void
--  
--  NOTES:           were no longer sending microphone data
------------------------------------------------------------------------------------------*/
void MultiServer::on_SendMicrophone_released()
{
    if (isMicrophoneSending) {
        //were no longer sending microphone data
        emit stopMicrophoneRecording();
        ui->SendMicrophone->setText("Start Recording Microphone");
    } else {
        //were are now sending microphone data
        mic = new MicrophoneManager(this, netManager);
        mic->RecordAudio();
        connect(this, SIGNAL(stopMicrophoneRecording()), mic, SLOT(stopRecording()));
        if (!netManager->startNetwork())
        {
            return;
        }
        int port = ui->linePort->text().toInt();
        switch (netManager->createMulticastServerSocket(ui->IPLine->text().toStdString().c_str(), port)) {
        case 0:
            AddStatusMessage("Invalid connection attempt.");
            return;
        case 1:
            AddStatusMessage("Successful Connection!");
            break;
        }

        if (isDataSending)
            emit on_BroadcastButton_released();
        ui->SendMicrophone->setText("Stop Recording Microphone");
    }
    isMicrophoneSending = !isMicrophoneSending;
}
