#include "filereader.h"
#include <QDir>

void FileReader::startReading()
{
    QString absName = QDir::currentPath() + "/MusicFiles/" + filename;
    fp = fopen(absName.toStdString().c_str(), "rb");
    sendSize();
}

void FileReader::sendSize()
{
    char temp[256] = {0};
    data[0] = 1;
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    sprintf(temp, "%d", size);
    memcpy(&data[1], temp, strlen(temp));
    emit sendPacket(data, strlen(data));
}

void FileReader::sendData()
{
    int t = 0;
    for(;;)
    {
        t = fread(data, 1, DATA_BUFSIZE, fp);
        if (t == 0)
        {
            fclose(fp);
            emit fileDone();
            return;
        }
        emit sendPacket(data, t);
    }
}
