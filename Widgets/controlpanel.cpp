#include "controlpanel.h"
#include <QLayout>
#include <QStyle>

controlPanel::controlPanel(QWidget *parent) : QWidget(parent)
{
    // Folder part

    label_folder = new QLabel("Working directory :");
    line_edit_folder = new QLineEdit("Path to folder..");
    button_choose_folder = new QPushButton(style()->standardIcon(QStyle::SP_DialogOpenButton), "Choose");
    button_choose_folder->setMaximumWidth(80);

    QVBoxLayout *layout1 = new QVBoxLayout();
    QVBoxLayout *layout2 = new QVBoxLayout;

    layout1->setMargin(0);
    layout1->addWidget(label_folder);
    layout1->addWidget(line_edit_folder);

    layout2->setMargin(0);
    layout2->addWidget(new QLabel());
    layout2->addWidget(button_choose_folder);

    // File part

    label_filename = new QLabel("Output filename :");
    line_edit_filename = new QLineEdit("Filename");

    QVBoxLayout *layout3 = new QVBoxLayout();

    layout3->setMargin(0);
    layout3->addWidget(label_filename);
    layout3->addWidget(line_edit_filename);

    // Format part

    QVBoxLayout *layout4 = new QVBoxLayout;

    label_format = new QLabel("Output format :");

    combo_box_format = new QComboBox();
    combo_box_format->setMaximumWidth(70);
    combo_box_format->addItem(".avi");
    combo_box_format->addItem(".gif");

    layout4->setMargin(0);
    layout4->addWidget(label_format);
    layout4->addWidget(combo_box_format);

    QHBoxLayout *layout_main = new QHBoxLayout();

    layout_main->setMargin(0);
    layout_main->addLayout(layout1);
    layout_main->addLayout(layout2);
    layout_main->addLayout(layout3);
    layout_main->addLayout(layout4);

    setLayout(layout_main);
}
