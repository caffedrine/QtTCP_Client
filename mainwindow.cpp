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
    if(client != Q_NULLPTR && client != 0)
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
    if(client != Q_NULLPTR || client)
    {
        if(client->is_alive())
        {
            client->doDisconnect();
        }
        return;
        delete client;
        client = Q_NULLPTR;
    }
}

void MainWindow::consoleLog(QString str)
{
    this->ui->textConsole->append("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "] " +  str);
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
    char recvBuffer[128];
    qint64 recvBytes = client->read(recvBuffer, 128);

    if(recvBytes == -1) /* Error */
        return;

    QString resultHex;
    for(qint64 i = 0; i < recvBytes; ++i)
        resultHex += QString("%1 ").arg(recvBuffer[i], 2, 16, QChar('0')).toUpper().remove("FFFFFFFFFFFFFF");
    resultHex.chop(1);

    consoleLog("RECV (" + QString::number(recvBytes) + " bytes): " + resultHex);
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
    QString strToSend = this->ui->textDataSend->text();
    if(strToSend.length() == 0)
        return;

    /* Store bytes to send as an array of bytes */
    QByteArray bytesToSend;

    /* Convert data from selected format to QByteArray */
    if(ui->radioButtonHex->isChecked())
    {
        bytesToSend = QByteArray::fromHex( strToSend.remove(" ").toLatin1() );

    }
    else if(ui->radioButtonString->isChecked())
    {
         bytesToSend = strToSend.toLocal8Bit();
    }
    else
    {
        consoleLog("Invalid data format selected!");
        return;
    }

    /* Send data to socket */
    qint64 sendBytes = client->write(bytesToSend);

    /* Check for socket errors */
    if(sendBytes == -1)
    {
        QMessageBox::warning(this, "Error!", "Failed to send data!\nREASON: " + client->getLastError());
        return;
    }

    /* Check how many bytes were send */
    if(sendBytes != bytesToSend.length())
    {
        QMessageBox::warning(this, "WARNING", "Only " + QString::number(sendBytes) + " / " + QString::number(bytesToSend.length()) + " were send!");
    }

    /* Build debug string to display relevant information about operation */
    QString dbgStr = "SEND (";
    dbgStr += QString::number(sendBytes) + " bytes): ";

    for(int i = 0; i < sendBytes; ++i)
        dbgStr += QString("%1 ").arg(bytesToSend[i], 2, 16, QChar('0')).toUpper().remove("FFFFFFFFFFFFFF");
    dbgStr.chop(1);

    consoleLog( dbgStr );
}
