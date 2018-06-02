#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTime>

#include "TcpClient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void consoleLog(QString str);

private slots:
    void onConnectionChanged(bool connected);
    void onBytesWritten(qint64 bytes);
    void onReadyRead(void);

    void on_buttonConnect_clicked();
    void on_buttonDisconnect_clicked();

    void on_buttonSend_clicked();

private:
    Ui::MainWindow *ui;
    class TcpClient *client = Q_NULLPTR;
};

#endif // MAINWINDOW_H
