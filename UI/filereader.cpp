#include "filereader.h"
#include <QDir>

void FileReader::startReading()
{
    QString absName = QDir::currentPath() + "/MusicFiles/" + filename;
    fp = fopen(absName.toStdString().c_str(), "rb");
    if (fp == NULL)
    {
        data[0] = 1;
        emit error(data, 1);
        return;
    }
    int t = 0;
    for(;;)
    {
        data[0] = 3;
        t = fread(&data[1], 1, DATA_BUFSIZE - 1, fp);
        if (t == 0)
        {
            fclose(fp);
            emit fileDone();
            return;
        }
        emit sendPacket(data, t+1);
    }
}
