#include "seekbuttons.h"

#include <QLayout>
#include <QPushButton>
#include <QStyle>

SeekButtons::SeekButtons(QWidget *parent)
    : QWidget(parent)
{
    const int count = 4;
    QHBoxLayout *layout = new QHBoxLayout;
    QHBoxLayout *l1 = new QHBoxLayout;
    QHBoxLayout *l2 = new QHBoxLayout;
    QPushButton* buttons[count][2];

    for (int i = 0; i < count; ++i)
    {
        buttons[i][0] = new QPushButton;
        buttons[i][0]->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
        buttons[i][0]->setFixedWidth(50);

        buttons[i][1] = new QPushButton;
        buttons[i][1]->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
        buttons[i][1]->setFixedWidth(50);

        l1->addWidget(buttons[i][0]);
        l2->addWidget(buttons[i][1]);
    }

    auto setText = [this, buttons](int btn_num, int value)
    {
        QPushButton *btn1 = buttons[btn_num][0];
        QPushButton *btn2 = buttons[count - btn_num - 1][1];

        connect(btn1, SIGNAL(clicked(bool)), &signalMapper, SLOT(map()));
        connect(btn2, SIGNAL(clicked(bool)), &signalMapper, SLOT(map()));
        signalMapper.setMapping(btn1, -value);
        signalMapper.setMapping(btn2, value);

        // Setting tooltips and text
        QString hint = "Move ";
        QString text;

        if (value < 1000)
        {
            hint += QString::number(value) + " milliseconds";
            text = "0." + QString::number(value / 100) + "s";
        }
        else if (value == 1000)
        {
            hint += "1 second";
            text = "1s";
        }
        else if (value < 1000 * 60)
        {
            hint += QString::number(value / 1000) + " seconds";
            text = QString::number(value / 1000) + "s";
        }

        btn1->setToolTip(hint + " back");
        btn2->setToolTip(hint + " forward");
        btn1->setText(text);
        btn2->setText(text);
    };

    setText(0, 1000 * 0.1);
    setText(1, 1000 * 1);
    setText(2, 1000 * 5);
    setText(3, 1000 * 30);

    connect(&signalMapper, SIGNAL(mapped(int)), this, SIGNAL(seek(int)));

    l1->setMargin(0); l1->setSpacing(0);
    l2->setMargin(0); l2->setSpacing(0);
    layout->setMargin(0); layout->setSpacing(0);

    layout->addLayout(l1);
    layout->addLayout(l2);

    setLayout(layout);
}
