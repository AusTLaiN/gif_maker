#include <QApplication>

#include "myplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // forGif , GifMaker, GifMe , Gifker, EasyGif, FastGif, Gif Factory

    QCoreApplication::setApplicationName("GifMaker");
    QCoreApplication::setOrganizationName("Home PC");

    MyPlayer player; 
    player.show();

    return a.exec();
}
