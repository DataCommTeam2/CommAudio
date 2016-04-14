#include "startwindow.h"
#include "multiserver.h"
#include <QApplication>


/*--------------------------------------------------------------------------------------------  
--  FUNCTION:        main
--  
--  DATE:            April 14th, 2016
--  
--  DESIGNERS:       
--  
--  REVISIONS:       NONE
--  
--  PROGRAMMERS:     
--  
--  INTERFACE:       int main(int argc, char *argv[])
--  
--  RETURNS:         int
--  
--  NOTES:           The main function to call for the program to begin running.
------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StartWindow w;
    w.show();

    return a.exec();
}
