#include "seekbuttons.h"

#include <QLayout>
#include <QPushButton>
#include <QStyle>

static void setText(QPushButton *button, int msecs, const QString &direction)
{
    QString hint = "Move ";
    QString text;

    if (msecs < 1000)
    {
        hint += QString::number(msecs) + " milliseconds";
        text = QString::number(msecs / 1000.0, 'f', 1) + "s";
    }
    else
    {
        hint += QString::number(msecs / 1000) + " second";
        text = QString::number(msecs / 1000) + "s";

        if (msecs / 1000 > 1)
            hint += 's';
    }

    button->setText(text);
    button->setToolTip(hint + " " + direction);
}

SeekButtons::SeekButtons(QWidget *parent)
    : QWidget(parent)
{
    const int count = 4;
    QHBoxLayout *layout = new QHBoxLayout;
    QHBoxLayout *l1 = new QHBoxLayout; // Layout for "back" buttons
    QHBoxLayout *l2 = new QHBoxLayout; // Layout for "forward" buttons
    double values[] = { 0.1, 1, 5, 30 }; // button seek value in seconds
    QPushButton* buttons[count][2];  

    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < 2; j++)
        {
            buttons[i][j] = new QPushButton;
            buttons[i][j]->setFixedWidth(48);
            buttons[i][j]->setCursor(Qt::PointingHandCursor);

            buttons[i][j]->setIcon(style()->standardIcon(j ? QStyle::SP_MediaSeekForward
                                                           : QStyle::SP_MediaSeekBackward));

            buttons[i][j]->setStyleSheet("QPushButton {"
                                         "height: 20px;"
                                         "border-width: 1px;"
                                         "border-style: solid;"
                                         "border-color: grey;"
                                         "border-radius: 0px;"
                                         "}");

            int index = j ? (count - i - 1) : i;
            double value = values[index] * 1000;

            connect(buttons[i][j], SIGNAL(clicked(bool)), &signalMapper, SLOT(map()));
            signalMapper.setMapping(buttons[i][j], j ? value : -value);
            setText(buttons[i][j], value, j ? "forward" : "back");
        }
    }

    for (int i = 0; i < count; ++i)
    {
        l1->addWidget(buttons[i][0]);
        l2->addWidget(buttons[i][1]);
    }

    connect(&signalMapper, SIGNAL(mapped(int)), this, SIGNAL(seek(int)));

    l1->setMargin(0);
    l1->setSpacing(0);

    l2->setMargin(0);
    l2->setSpacing(0);

    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addLayout(l1);
    layout->addLayout(l2);

    setLayout(layout);
}
