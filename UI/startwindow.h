#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>
#include <QtMultimedia>
#include <QtMultimediaWidgets>

namespace Ui {
class StartWindow;
}


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
class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = 0);
    ~StartWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_released();

private:
    Ui::StartWindow *ui;
};

#endif // STARTWINDOW_H
