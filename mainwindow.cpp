#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStyle>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Center on screen on launch */
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    if(client != Q_NULLPTR && client)
    {
        if(client->is_alive())
        {
            QMessageBox::warning(this, "Warning!", "Please disconnect first!");
            return;
        }
    }

    /* Create a new socket fd */
    client = new TcpClient();

    /* Connect it's signals*/
    connect(client, SIGNAL( onConnectionChanged(bool) ), this, SLOT(onConnectionChanged(bool)) );
    connect(client, SIGNAL( onBytesWritten(qint64 ) ), this, SLOT(onBytesWritten(qint64)) );
    connect(client, SIGNAL( onReadyRead(void) ), this, SLOT(onReadyRead(void)) );

    /* Set destination ip and port */
    client->setPort( ui->lineEditPort->text().toInt() );
    client->setHostname( ui->lineEditIP->text() );

    /* Connect to server */
    client->doConnect();
    if(!this->client->is_alive())
    {
        delete client;
        client = Q_NULLPTR;
    }
}

void MainWindow::on_buttonDisconnect_clicked()
{
    if(client != Q_NULLPTR || client != 0)
    {
        if(client->is_alive())
        {
            client->doDisconnect();
        }

        delete client;
        client = Q_NULLPTR;
    }
}

void MainWindow::consoleLog(QString str)
{
    this->ui->textConsole->append("[" + QTime::currentTime().toString() + "] " +  str);
}

/*
 *  EVENTS
 */
void MainWindow::onConnectionChanged(bool connected)
{
    ui->textLabelStatusTCP->setText(connected?"CONNECTED":"DISCONNECTED");

    if(connected)
        this->consoleLog("Connected to " + this->client->getHostName() + " " + QString::number(this->client->getPort()) );
    else
        this->consoleLog("Lost/broken connection to " + this->client->getHostName() + " " + QString::number(this->client->getPort()) );

    if(!client->is_alive())
    {
        client->doDisconnect();
    }
}

void MainWindow::onBytesWritten(qint64 bytes)
{
    if(bytes == -1)
    {
        ;
    }
}

void MainWindow::onReadyRead(void)
{
    char data[128];
    if(client->read(data, 128) == -1)
        return;

    consoleLog("RECV: " + QString::fromStdString(data));
}

void MainWindow::on_buttonSend_clicked()
{
    /* Check if there is a connection or not! */
    if(client == Q_NULLPTR || client == 0 || !client->is_alive())
    {
        QMessageBox::warning(this, "Warning!", "Please connect first!");
        return;
    }

    /* Fetch data from UI */
    QString data = this->ui->textDataSend->text();

    /* Send data to socket */
    qint64 sendBytes = client->write(data);

    if(sendBytes == -1)
    {
        QMessageBox::warning(this, "Error!", "Failed to send data!\nREASON: " + client->getLastError());
        return;
    }

    if(sendBytes != data.length())
    {
        QMessageBox::warning(this, "WARNING", "Only " + QString::number(sendBytes) + " / " + QString::number(data.length()) + " were send!");
    }

    consoleLog("SEND: " + data.left(sendBytes) );
}
