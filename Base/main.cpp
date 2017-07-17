#include <QApplication>
#include <QFile>

#include "myplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("GifMaker");
    QCoreApplication::setOrganizationName("Home PC");

    MyPlayer player; 

    if (argc == 2)
    {
        QString filename = a.arguments().last();
        player.setFile(filename);
    }

    player.show();

    return a.exec();
}
