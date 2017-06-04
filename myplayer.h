#ifndef MYPLAYER_H
#define MYPLAYER_H

#include "Dialogs/dialogs.h"
#include "Widgets/widgets.h"

#include <QVideoWidget>
#include <QLabel>

class MyPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MyPlayer(QWidget *parent = 0);
    ~MyPlayer() = default;

public slots:
    void pause();
    void play();
    void setFile(const QString &file);

protected slots:
    void setPosition(qint64 msecs);
    void positionChanged(qint64 msecs);
    void durationChanged(qint64 duration);
    void updateDurationInfo(qint64 current_pos);

    void openFile();

    void setMarker() const;
    void moveToMarker();

    void loadSettings();
    void saveSettings() const;

    void createWidgets();
    void createLayouts();
    void createConnections();

    void createMovie();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

protected:
    MyVideoWidget *video_widget;
    DurationSlider *slider_duration;
    QLabel *label_duration;
    QPushButton *button_open, *button_create;

    QMediaPlayer *player;

    PlayerControls *controls;
    Marker *marker1, *marker2;
    SeekButtons *seek_buttons;

    QString movies_directory, current_file;
};

#endif // MYPLAYER_H
