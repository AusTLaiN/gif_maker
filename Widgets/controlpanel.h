#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QLabel>
#include <QTextBlock>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

class controlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit controlPanel(QWidget *parent = 0);
    ~controlPanel() = default;

signals:

public slots:

protected:
    QTextBlock *text_block1;
    QLineEdit *line_edit_filename;
    QLineEdit *line_edit_folder;
    QPushButton *button_choose_folder;
    QLabel *label_folder, *label_filename, *label_format;
    QComboBox *combo_box_format;
};

#endif // CONTROLPANEL_H
