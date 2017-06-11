#include "myplayer.h"

#include <QtWidgets>
#include <QDebug>

MyPlayer::MyPlayer(QWidget *parent):
    QMainWindow(parent),
    player(new QMediaPlayer(this))
{  
    createWidgets();
    createLayouts();
    createConnections();
    createActions();
    createMenus();

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
    label_duration->setMinimumWidth(110);
    label_duration->setToolTip("Current time/Duration");

    /*button_open = new QPushButton("Open");
    button_open->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    button_open->setToolTip("Open file");*/

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
    //layout_controls->addWidget(button_open);
    //layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_controls->addWidget(controls);
    //layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_controls->addWidget(seek_buttons);
    layout_controls->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_controls->addWidget(label_duration);

    QVBoxLayout *layout_markers = new QVBoxLayout();
    layout_markers->setMargin(0);
    layout_markers->setSpacing(2);
    layout_markers->addWidget(marker1);
    layout_markers->addWidget(marker2);

    QHBoxLayout *layout_options = new QHBoxLayout();
    layout_options->addSpacerItem(new QSpacerItem(142, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    //layout_options->addSpacerItem(new QSpacerItem(controls->width(), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_options->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_options->addLayout(layout_markers);
    layout_options->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout_options->addWidget(button_create);

    QHBoxLayout *layout_misc = new QHBoxLayout;
    layout_misc->setMargin(0);
    layout_misc->setSpacing(5);
    layout_misc->addWidget(button_create);
    layout_misc->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QHBoxLayout *layout_duration = new QHBoxLayout;
    layout_duration->addSpacerItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    layout_duration->addWidget(slider_duration);
    layout_duration->addSpacerItem(new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));

    second_layout->setMargin(5);
    //second_layout->addWidget(slider_duration);
    second_layout->addLayout(layout_controls);
    second_layout->addLayout(layout_options);

    main_layout->setMargin(0);
    main_layout->addWidget(video_widget);
    //main_layout->addWidget(slider_duration);
    main_layout->addLayout(layout_duration);
    main_layout->addLayout(second_layout);

    QWidget *central_widget = new QWidget();
    central_widget->setLayout(main_layout);

    //setLayout(main_layout);
    setCentralWidget(central_widget);
}

void MyPlayer::createConnections()
{
    // Connection with duration slider

    // Prevents recursion updates ( like: slider -> player -> slider -> ... )
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

    // Misc connections

    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handlePlayerError(QMediaPlayer::Error)));

    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

    //connect(button_open, SIGNAL(clicked(bool)), this, SLOT(openFile()));
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
        if (state == QMediaPlayer::StoppedState && player->position() >= player->duration()
            && player->error() == QMediaPlayer::NoError)
        {
            qDebug() << "Repeat";
            setPosition(0);
            play();
        }
    });

    // Connections with seek buttons
    // Move N msecs back/forward from current position
    connect(seek_buttons, &SeekButtons::seek, [this](int ms){
        qint64 pos = player->position() + ms;
        if (pos < 0)
            pos = 0;
        else if (pos > player->duration())
            pos = player->duration();

        setPosition(pos);
    });

    // Connections with markers

    QList<Marker*> markers = { marker1, marker2 };

    foreach (Marker* item, markers)
    {
        connect(item, SIGNAL(clicked_setMarker()), this, SLOT(setMarker()));
        connect(item, SIGNAL(clicked_move()), this, SLOT(moveToMarker()));
    }

}

void MyPlayer::createActions()
{
    action_open_file = new QAction("Open", this);
    action_open_file->setShortcuts(QKeySequence::Open);
    action_open_file->setStatusTip(tr("Open local file"));
    connect(action_open_file, &QAction::triggered, this, &MyPlayer::openFile);

    action_recent_clear = new QAction("Clear", this);
    action_open_file->setStatusTip(tr("Clear recent files list"));
}

void MyPlayer::createMenus()
{
    /*menu_file = menuBar()->addMenu("File");
    menu_file->addAction(action_open_file);

    menu_recent = menu_file->addMenu("Recent");
    menu_recent->addAction(action_recent_clear);*/
    menuBar()->addAction(action_open_file);
    menu_recent = menuBar()->addMenu("Recent files");
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
    if (player->error() != QMediaPlayer::NoError)
    {
        qDebug() << "MyPlayer::setPosition: player contains error";
        return;
    }

    player->setPosition(msecs);
}

void MyPlayer::positionChanged(qint64 msecs)
{
    if (msecs > player->duration())
    {
        msecs %= player->duration();
        setPosition(msecs);
        return;
    }

    slider_duration->setPosition(msecs);
}

void MyPlayer::durationChanged(qint64 duration)
{
    slider_duration->setRange(0, duration / 100);
    updateDurationInfo(0);

    auto start = QTime(0, 0 ,0);
    auto end = QTime(0, 0, 0).addMSecs(duration);

    marker1->setDefault(start);
    marker1->clear();
    marker2->setDefault(end);
    marker2->clear();
}

void MyPlayer::openFile()
{
    if (player->error() == QMediaPlayer::NoError)
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
    QTime time = QTime(0, 0, 0).addMSecs(player->position());
    obj->setTime(time);
}

void MyPlayer::moveToMarker()
{
    Marker *obj = static_cast<Marker*>(sender());
    qint64 msecs = QTime(0, 0, 0).msecsTo(obj->getTime());

    setPosition(msecs);
}

void MyPlayer::updateDurationInfo(qint64 current_pos)
{
    QString info, format;

    QTime total = QTime(0, 0 ,0).addMSecs(player->duration());
    QTime current = QTime(0, 0, 0).addMSecs(current_pos);

    format = total.hour() > 0 ? "HH:mm:ss.zzz" : "mm:ss.zzz";
    info = current.toString(format) + " / " + total.toString(format);

    label_duration->setText(info);
}

void MyPlayer::createMovie()
{
    if (current_file.isEmpty())
        return;
    if (player->error() != QMediaPlayer::NoError)
        return;

    if (marker1->getTime() >= marker2->getTime())
    {
        QMessageBox(QMessageBox::Critical, "Error", "Start time must be less than end").exec();
        return;
    }

    pause();
    MakeMovieDialog dlg(current_file, nullptr);
    QTime start = marker1->getTime();
    QTime end = marker2->getTime();
    QTime total = QTime(0, 0, 0).addMSecs(player->duration());

    dlg.setTime(start, end, total);
    dlg.exec();

    if (!dlg.getResult().isEmpty())
        setFile(dlg.getResult());
}

void MyPlayer::handlePlayerError(QMediaPlayer::Error error)
{
    player->stop();
    QStringList err_msg;
    err_msg << "MediaPlayer error"
            << "Error code : " + QString::number(error)
            << "Error msg  : " + player->errorString();
    QMessageBox(QMessageBox::Critical, "Error", err_msg.join("\n")).exec();
}
