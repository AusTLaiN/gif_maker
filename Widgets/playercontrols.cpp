#include "playercontrols.h"
#include <QLayout>
#include <QStyle>

PlayerControls::PlayerControls(QWidget *parent) :
    QWidget(parent),
    button_play(new QPushButton),
    button_mute(new QPushButton),
    slider_volume(new VolumeSlider)
{
    connect(button_play, SIGNAL(clicked(bool)), this, SLOT(buttonPlay_clicked()));
    connect(button_mute, SIGNAL(clicked(bool)), this, SLOT(buttonMute_clicked()));
    connect(slider_volume, SIGNAL(volumeChanged(int)), SIGNAL(changeVolume(int)));

    QHBoxLayout *layout = new QHBoxLayout;

    layout->setMargin(0);
    layout->setSpacing(2);  
    layout->addWidget(button_play);   
    layout->addWidget(button_mute);
    layout->addWidget(slider_volume);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
    // Tooltips

    button_mute->setToolTip("Mute/Unmute");
    button_play->setToolTip("Play/Pause");

    button_mute->setCursor(Qt::PointingHandCursor);
    button_play->setCursor(Qt::PointingHandCursor);

    button_mute->setStyleSheet("QPushButton {"
                               "width: 22px;"
                               "height: 22px;"
                               "border-width: 1px;"
                               "border-style: solid;"
                               "border-color: grey;"
                               "border-radius: 4px;"
                               "}");
    button_play->setStyleSheet("QPushButton {"
                               "width: 22px;"
                               "height: 22px;"
                               "border-width: 1px;"
                               "border-style: solid;"
                               "border-color: grey;"
                               "border-radius: 4px;"
                               "}");

    // Default values

    playerMuted = false;
    button_mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    setMediaPlayerState(QMediaPlayer::PausedState);
}

QMediaPlayer::State PlayerControls::getMediaPlayerState() const
{
    return playerState;
}

int PlayerControls::getVolume() const
{
    return slider_volume->value();
}

bool PlayerControls::isMuted() const
{
    return playerMuted;
}

void PlayerControls::setMediaPlayerState(QMediaPlayer::State state)
{
    if (playerState != state)
    {
        playerState = state;
        switch(state)
        {
        case QMediaPlayer::PlayingState:
            button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
        case QMediaPlayer::StoppedState:
            button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        }
    }

}

void PlayerControls::setVolume(int volume)
{
    slider_volume->setVolume(volume);
}

void PlayerControls::setMuted(bool muted)
{
    if (muted != playerMuted)
    {
        playerMuted = muted;

        button_mute->setIcon(style()->standardIcon(muted
                ? QStyle::SP_MediaVolumeMuted
                : QStyle::SP_MediaVolume));
    }
}

void PlayerControls::buttonMute_clicked()
{
    emit changeMuting(!playerMuted);
}

void PlayerControls::buttonPlay_clicked()
{
    switch(playerState)
    {
    case QMediaPlayer::PlayingState:
        emit pause();
        break;
    case QMediaPlayer::PausedState:
    case QMediaPlayer::StoppedState:
        emit play();
        break;
    }
}
