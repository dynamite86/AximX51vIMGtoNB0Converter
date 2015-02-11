#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_aximImgUtil = new AximROMImageUtil(this);
    connect(m_aximImgUtil, SIGNAL(signalOsNb0FileDecryptIsComplete(bool)), this, SLOT(m_osNb0FileDecryptIsComplete(bool)));
    connect(m_aximImgUtil, SIGNAL(logOutputSignal(bool,QString)),          this, SLOT(m_logOutputMessage(bool,QString)));

    ui->pushButton_decrypt->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete m_aximImgUtil;
    delete ui;
}

void MainWindow::on_toolButton_chooseFile_clicked()
{
    QString file = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, "Choose Dell Axim ROM image file", "", "Dell Axim ROM image files (*.img)"));
    if((!file.isEmpty())&&QFileInfo(file).exists())
    {
        m_aximImgUtil->setImageFile(file);
        ui->lineEdit_filePath->setText(file);
        ui->pushButton_decrypt->setEnabled(true);
        ui->checkBox_verboseLogging->setEnabled(true);
    }
    else
    {
        ui->lineEdit_filePath->clear();
        ui->pushButton_decrypt->setDisabled(true);
        ui->checkBox_verboseLogging->setDisabled(true);
    }
}

void MainWindow::on_pushButton_decrypt_clicked()
{
    m_aximImgUtil->decryptOsNb0File(ui->checkBox_verboseLogging->isChecked());
    ui->pushButton_decrypt->setDisabled(true);
    ui->checkBox_verboseLogging->setDisabled(true);
}

void MainWindow::m_osNb0FileDecryptIsComplete(bool success)
{
    if(success)
    {
        QMessageBox::information(this, this->windowTitle(), "Done!");
    }
    else
    {
        QMessageBox::critical(this, this->windowTitle(), "FFFUUU!!!!!!");
    }
    ui->checkBox_verboseLogging->setEnabled(true);
    ui->pushButton_decrypt->setEnabled(true);
}

void MainWindow::m_logOutputMessage(bool errorMsg, QString msg)
{
    if(errorMsg){
        ui->textEdit_log->append("<font color=red>"   + msg + "</font>");
    }else{
        ui->textEdit_log->append("<font color=black>" + msg + "</font>");
    }
}

