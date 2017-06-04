#include "myplayer.h"

#include <QtWidgets>
#include <QDebug>

// Internal

static QTime getTime(qint64 milliseconds)
{
    int secs = milliseconds / 1000;
    int mins = secs / 60;
    int hours = mins / 60;

    return QTime(hours, mins % 60, secs % 60, milliseconds % 1000);
}

// Internal end

MyPlayer::MyPlayer(QWidget *parent):
    QWidget(parent),
    player(new QMediaPlayer(this))
{  
    createWidgets();
    createLayouts();
    createConnections();

    player->setVideoOutput(video_widget);
    player->setNotifyInterval(50);

    setWindowTitle(QCoreApplication::applicationName());
    setPosition(0);

    loadSettings();
}

void MyPlayer::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MyPlayer::createWidgets()
{
    video_widget = new MyVideoWidget;

    slider_duration = new DurationSlider();

    label_duration = new QLabel;
    label_duration->setMinimumWidth(100);
    label_duration->setToolTip("Current time/Duration");

    button_open = new QPushButton("Open");
    button_open->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    button_open->setToolTip("Open file");

    seek_buttons = new SeekButtons;

    button_create = new QPushButton("Create movie");
    button_create->setFixedWidth(100);
    button_create->setToolTip("Select options and create a movie");

    controls = new PlayerControls;
    marker1 = new Marker("Start position :");
    marker2 = new Marker("End position :");
}

void MyPlayer::createLayouts()
{
    QVBoxLayout *main_layout = new QVBoxLayout();
    QVBoxLayout *second_layout = new QVBoxLayout();

    QHBoxLayout *layout_controls = new QHBoxLayout();
    layout_controls->setMargin(0);
    layout_controls->addWidget(button_open);
    layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_controls->addWidget(controls);
    layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_controls->addWidget(seek_buttons);
    layout_controls->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout_controls->addWidget(label_duration);

    QVBoxLayout *layout_markers = new QVBoxLayout();
    layout_markers->addWidget(marker1);
    layout_markers->addWidget(marker2);

    QHBoxLayout *layout_options = new QHBoxLayout();
    layout_options->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_options->addLayout(layout_markers);
    layout_options->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_options->addWidget(button_create);

    QHBoxLayout *layout_misc = new QHBoxLayout;
    layout_misc->setMargin(0);
    layout_misc->setSpacing(5);
    layout_misc->addWidget(button_create);
    layout_misc->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    second_layout->setMargin(5);
    second_layout->addWidget(slider_duration);
    second_layout->addLayout(layout_controls);
    second_layout->addLayout(layout_options);

    main_layout->setMargin(0);
    main_layout->addWidget(video_widget);
    main_layout->addLayout(second_layout);

    setLayout(main_layout);
}

void MyPlayer::createConnections()
{
    // Connections with specific objects

    // Duration slider

    // Prevents recursion updates
    // Disconnect player => slider update while slider is moving
    connect(slider_duration, &DurationSlider::sliderPressed, [this](){
        disconnect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    });
    connect(slider_duration, &DurationSlider::sliderReleased, [this](){
        connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    });
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));

    connect(slider_duration, SIGNAL(positionChanged(qint64)), this, SLOT(setPosition(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(updateDurationInfo(qint64)));

    // duration_slider end


    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

    connect(button_open, SIGNAL(clicked(bool)), this, SLOT(openFile()));
    connect(button_create, SIGNAL(clicked(bool)), this, SLOT(createMovie()));

    // Connections with player controls

    connect(video_widget, SIGNAL(mousePressed()), controls, SLOT(buttonPlay_clicked()));

    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), controls, SLOT(setMediaPlayerState(QMediaPlayer::State)));

    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));

    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));


    // Auto-repeat
    connect(player, &QMediaPlayer::stateChanged, [&, this](QMediaPlayer::State state){
        if (state == QMediaPlayer::StoppedState && player->position() >= player->duration())
        {
            qDebug() << "Repeat";
            setPosition(0);
            play();
        }
    });

    // Move N msecs back/forward from current position
    connect(seek_buttons, &SeekButtons::seek, [this](int ms){
        setPosition(this->player->position() + ms);
    });

    // Connections with markers

    QList<Marker*> markers = { marker1, marker2 };

    foreach (Marker* item, markers)
    {
        connect(item, SIGNAL(clicked_setMarker()), this, SLOT(setMarker()));
        connect(item, SIGNAL(clicked_move()), this, SLOT(moveToMarker()));
    }

}

void MyPlayer::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName() , QCoreApplication::applicationName());

    QPoint pos = settings.value("Window position", QPoint(100, 50)).toPoint();
    QSize size = settings.value("Window size", QSize(800, 600)).toSize();
    QString movies_directory_tmp = settings.value("Movies directory", QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath())).toString();

    settings.beginGroup("Player");
    int volume = settings.value("Volume", 15).toInt();
    bool muted = settings.value("Muted", false).toBool();
    settings.endGroup();

    // Applying

    this->resize(size);
    this->move(pos);
    movies_directory = movies_directory_tmp;

    player->setVolume(volume);
    player->setMuted(muted);
}

void MyPlayer::saveSettings() const
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName() , QCoreApplication::applicationName());

    settings.setValue("Window position", this->pos());
    settings.setValue("Window size", this->size());
    settings.setValue("Movies directory", this->movies_directory);

    settings.beginGroup("Player");
    settings.setValue("Volume", player->volume());
    settings.setValue("Muted", player->isMuted());
    settings.endGroup();
}

void MyPlayer::pause()
{
    player->pause();
}

void MyPlayer::play()
{
    player->play();
}

void MyPlayer::setFile(const QString &file)
{
    player->setMedia(QUrl::fromLocalFile(file));

    QStringList path = file.split("/");
    QString filename = path.last();

    setWindowTitle(QCoreApplication::applicationName() + " - " + filename);
    current_file = file;

    play();
}

void MyPlayer::setPosition(qint64 msecs)
{
    player->setPosition(msecs);
}

void MyPlayer::positionChanged(qint64 msecs)
{
    if (msecs > player->duration())
        msecs %= player->duration();

    slider_duration->setPosition(msecs);
}

void MyPlayer::durationChanged(qint64 duration)
{
    slider_duration->setRange(0, duration / 100);
    updateDurationInfo(0);

    marker1->setDefault(QTime(0, 0 , 0));
    marker1->clear();
    marker2->setDefault(getTime(duration));
    marker2->clear();
}

void MyPlayer::openFile()
{
    pause();
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle("Open file");
    fileDialog.setDirectory(movies_directory);

    if (fileDialog.exec() == QDialog::Accepted)
    {
        QString file = fileDialog.selectedFiles().first();

        setFile(file);
        movies_directory = QFileInfo(file).dir().path();
    }
}

void MyPlayer::setMarker() const
{
    Marker *obj = static_cast<Marker*>(sender());
    obj->setTime(getTime(player->position()));
}

void MyPlayer::moveToMarker()
{
    Marker *obj = static_cast<Marker*>(sender());
    QTime time = obj->getTime();
    qint64 hours = time.hour();
    qint64 minutes = hours * 60 + time.minute();
    qint64 seconds = minutes * 60 + time.second();
    qint64 milliseconds = seconds * 1000 + time.msec();

    setPosition(milliseconds);
}

void MyPlayer::updateDurationInfo(qint64 current_pos)
{
    QString info, format;

    QTime total = getTime(player->duration());
    QTime current = getTime(current_pos);

    format = total.hour() > 0 ? "HH:mm:ss.zzz" : "mm:ss.zzz";
    info = current.toString(format) + " / " + total.toString(format);

    label_duration->setText(info);
}

void MyPlayer::createMovie()
{
    if (current_file.isEmpty())
        return;

    if (marker1->getTime() >= marker2->getTime())
    {
        QMessageBox(QMessageBox::Critical, "Error", "Start time must be less than end").exec();
        return;
    }

    pause();
    MakeMovieDialog dlg(current_file, nullptr);
    dlg.setTime(marker1->getTime(), marker2->getTime(), getTime(player->duration()));
    dlg.exec();

    if (!dlg.getResult().isEmpty())
        setFile(dlg.getResult());
}
