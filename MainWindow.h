#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtWidgets>
#include <QtGui>
#include "AximROMImageUtil.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_toolButton_chooseFile_clicked();
    void on_pushButton_decrypt_clicked();

    void m_osNb0FileDecryptIsComplete(bool success);
    void m_logOutputMessage(bool errorMsg, QString msg);

private:
    Ui::MainWindow *ui;
    AximROMImageUtil *m_aximImgUtil;
};

#endif // MAINWINDOW_H
