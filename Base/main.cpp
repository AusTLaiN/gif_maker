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
        player.setFile(QFile(argv[1]).fileName());
    }

    player.show();

    return a.exec();
}
