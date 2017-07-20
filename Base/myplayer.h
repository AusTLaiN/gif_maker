#ifndef MYPLAYER_H
#define MYPLAYER_H

#include "Dialogs/makemoviedialog.h"
#include "Widgets/widgets.h"

#include <QVideoWidget>
#include <QMainWindow>
#include <QLabel>

class MyPlayer : public QMainWindow
{
    Q_OBJECT

public:
    static const int recent_limit = 20;

public:
    explicit MyPlayer(QWidget *parent = 0);
    ~MyPlayer() = default;

signals:
    void durationInfoChanged(const QString &info);
    void allowRepeatChanged(bool allowed);

public slots:
    void pause();
    void play();
    void setFile(const QString &file);
    void allowRepeat(bool allowed);

protected slots:
    void setPosition(qint64 msecs);
    void positionChanged(qint64 msecs);
    void durationChanged(qint64 duration);
    void updateDurationInfo(qint64 current_pos);

    void openFile();

    void setMarker() const;
    void moveToMarker();

    void createMovie();

    void actionOpenRecent();
    void actionRecentClear();

    void handlePlayerError(QMediaPlayer::Error error);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

    void loadSettings();
    void saveSettings() const;

    void updateRecent(const QString &file);

    void createWidgets();
    void createLayouts();
    void createConnections();
    void createActions();
    void createMenus();
    void createMenuRecent();
    void createFullscreenWidget();

protected:
    MyVideoWidget *video_widget;
    DurationSlider *slider_duration;
    QLabel *label_duration;
    QPushButton *button_create;
    QWidget *fullScreen_widget;

    QMediaPlayer *player;

    PlayerControls *controls;
    Marker *marker1;
    Marker *marker2;
    SeekButtons *seek_buttons;

    QString movies_directory;
    QString current_file;

    QStringList recent_files;

    QMenu *menu_file;
    QMenu *menu_recent;

    QAction *action_open_file;
    QAction *action_recent_clear;

    bool repeat_allowed;
};

#endif // MYPLAYER_H
