#include "startwindow.h"
#include "ui_startwindow.h"
#include "multiserver.h"
#include "multiclient.h"
#include "peertopeer.h"


/*--------------------------------------------------------------------------------------------  
--  SOURCE:          StartWindow
--  
--  PROGRAM:         CommAudio
--  
--  FUNCTIONS:       explicit StartWindow(QWidget *parent = 0);
--                   ~StartWindow();
--                   void on_pushButton_clicked();
--                   void on_pushButton_released();
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
--  NOTES:           This class handles the UI for the welcome window, for which the user would
--                   use to choose what window and program type to use from a list of:
--                   	- Multicast client
--                   	- Multicast server
--                   	- Peer to Peer client
------------------------------------------------------------------------------------------*/
StartWindow::StartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow)
{
    ui->setupUi(this);
}

StartWindow::~StartWindow()
{
    delete ui;
}
/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        on_pushButton_clicked
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       Jaegar Sarauer
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     Jaegar Sarauer
--  
--  INTERFACE:       void StartWindow::on_pushButton_clicked()
--  
--  RETURNS:         void
--  
--  NOTES:           This function handles the radio buttons to determine whch window to create.
------------------------------------------------------------------------------------------*/
void StartWindow::on_pushButton_clicked()
{
   if (ui->radioMultiClient->isChecked()) {
       MultiClient *ms = new MultiClient(this);
       ms->setAttribute(Qt::WA_DeleteOnClose);
       ms->show();
   } else if (ui->radioMultiServer->isChecked()) {
       MultiServer *ms = new MultiServer(this);
       ms->setAttribute(Qt::WA_DeleteOnClose);
       ms->show();
   } else if (ui->radioPeer->isChecked()) {
       PeerToPeer *ms = new PeerToPeer(this);
       ms->setAttribute(Qt::WA_DeleteOnClose);
       ms->show();
   }
   destroy(true, false);
}

void StartWindow::on_pushButton_released()
{

}
