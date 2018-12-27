#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QBitArray>
#include <QDebug>

#include "mainwindow.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);
    ~TcpClient();



    /* Set internals */
    void setHostname(QString hostname);
    void setPort(int port);
    void setLastError(QString err);

    /* Get internals */
    QString getHostName(void);
    quint16 getPort();
    QString getLastError(void);

    /* Socket operations */
    void doConnect();
    void doDisconnect();
    qint64 read(char *data, qint64 max_len);
    qint64 write(QByteArray sendBytes);
    bool is_alive();

signals:
    /* To be implemented on mother class */
    void onConnectionChanged(bool connected);
    void onBytesWritten(qint64 bytes);
    void onReadyRead(void);

private slots:
    /* Slots triggered by internal signals */
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    /* Internal variables */
    QString lastError, hostname;
    int port;
    bool isConnected = false;

    /* Socket fd */
    QTcpSocket *socket = Q_NULLPTR;
};

#endif // TCPCLIENT_H
