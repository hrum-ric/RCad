#include "newfiledialog.h"
#include "ui_newfiledialog.h"
#include <QtWidgets/QCompleter>
#include <QtWidgets/QDirModel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

newFileDialog::newFileDialog(QDir& DefaultDir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newFileDialog)
{
    ui->setupUi(this);

    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));
    ui->directory->setCompleter(completer);
	ui->directory->setText( DefaultDir.absolutePath() );

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

newFileDialog::~newFileDialog()
{
    delete ui;
}

QString newFileDialog::getFullName()
{
	return QFileInfo(ui->directory->text(), ui->name->text()).filePath();
}

void newFileDialog::__chooseDir()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("directory where the file will be created."), ui->directory->text());
    if( !dir.isEmpty() )  ui->directory->setText(dir);
}

void newFileDialog::__textChanged()
{
    if( !ui->name->text().isEmpty() && QDir(ui->directory->text()).exists() )
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}
