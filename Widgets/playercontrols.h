#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include "volumeslider.h"

#include <QMediaPlayer>
#include <QWidget>
#include <QPushButton>
#include <QSlider>

class PlayerControls : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerControls(QWidget *parent = 0);
    ~PlayerControls() = default;

    QMediaPlayer::State getMediaPlayerState() const;
    int getVolume() const;
    bool isMuted() const;

public slots:
    void setMediaPlayerState(QMediaPlayer::State state);
    void setVolume(int volume);
    void setMuted(bool muted);

signals:
    void play();
    void pause();
    void changeVolume(int volume);
    void changeMuting(bool muting);

protected slots:
    void buttonMute_clicked();
    void buttonPlay_clicked();

protected:
    bool playerMuted;
    QMediaPlayer::State playerState;

    QPushButton *button_play, *button_mute;
    VolumeSlider *slider_volume;
};

#endif // PLAYERCONTROLS_H
