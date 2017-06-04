#ifndef SEEKBUTTONS_H
#define SEEKBUTTONS_H

#include <QWidget>
#include <QSignalMapper>

class SeekButtons : public QWidget
{
    Q_OBJECT

public:
    explicit SeekButtons(QWidget *parent = 0);

signals:
    void seek(int ms);

protected:
    QSignalMapper signalMapper;
};

#endif // SEEKBUTTONS_H
