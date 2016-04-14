#ifndef INCOMINGCONNTHREAD_H
#define INCOMINGCONNTHREAD_H

#include <QObject>


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          IncomingConnThread
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       IncomingConnThread(void * s) : socket(s)
--                   ~IncomingConnThread()
--                   void checkForConnection();
--                   void tcpConnected();
--                   
--  
--  DATE:            CommAudio
--  
--  DESIGNERS:       Gabriella Cheung
--  
--  REVISIONS:       Several
--  
--  PROGRAMMERS:     Gabriella Cheung
--  
--  NOTES:           This class is for recieving incomming connections. It is run as a sepereate thread.
------------------------------------------------------------------------------------------*/
class IncomingConnThread : public QObject
{
    Q_OBJECT
public:
    IncomingConnThread(void * s) : socket(s) {}
    ~IncomingConnThread(){}

public slots:
    void checkForConnection();

signals:
    void tcpConnected();

private:
    void * socket;

};

#endif // INCOMINGCONNTHREAD_H
