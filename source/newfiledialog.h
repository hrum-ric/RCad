#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <QDialog>
#include <QDir>

namespace Ui {
class newFileDialog;
}

class newFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit newFileDialog(QDir& DefaultDir, QWidget *parent = 0);
    ~newFileDialog();

    QString getFullName();

private slots:
    void __chooseDir();
    void __textChanged();

private:
    Ui::newFileDialog *ui;
};

#endif // NEWFILEDIALOG_H
