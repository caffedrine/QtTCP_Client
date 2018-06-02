#include "TcpClient.h"

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
}

TcpClient::~TcpClient()
{
    this->doDisconnect();

    if(socket != Q_NULLPTR)
        delete socket;
}

void TcpClient::setHostname(QString host)
{
	this->hostname = host;
}

void TcpClient::setLastError(QString err)
{
    this->lastError = err;
}

void TcpClient::setPort(int prt)
{
	this->port = prt;
}

QString TcpClient::getHostName()
{
    return this->hostname;
}

quint16 TcpClient::getPort()
{
    return this->port;
}

QString TcpClient::getLastError()
{
    return this->lastError;
}

void TcpClient::doConnect()
{
    if(this->socket != Q_NULLPTR || socket)
    {
        delete this->socket;
        this->socket = Q_NULLPTR;
    }

    /* Clean from last comunication */
    this->socket = new QTcpSocket(this);
	this->socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

	connect(socket, SIGNAL(connected()),this, SLOT(connected()));
	connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
	connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

	// this is not blocking call
	socket->connectToHost(this->hostname, this->port);

	// we need to wait...
	if(!socket->waitForConnected(10000))
	{
        this->lastError = socket->errorString();
        this->socket->disconnectFromHost();
        this->socket->close();

        if(this->socket != Q_NULLPTR || socket)
        {
            delete this->socket;
            this->socket = Q_NULLPTR;
        }
    }
}

void TcpClient::connected()
{
    emit onConnectionChanged(true);
}

void TcpClient::disconnected()
{
    emit onConnectionChanged(false);
}

void TcpClient::bytesWritten(qint64 bytes)
{
    emit onBytesWritten(bytes);
}

void TcpClient::readyRead()
{
    emit onReadyRead();
}

qint64 TcpClient::read(char *data, qint64 max_len)
{
    if(this->socket == Q_NULLPTR || this->socket == 0)
        return -1;

    if(!this->is_alive())
        return -1;

    return socket->read(data, max_len);
}

qint64 TcpClient::write(QString msg)
{
    if(this->socket == Q_NULLPTR || this->socket == 0)
        return -1;

    if(!this->is_alive())
        return -1;

	const QByteArray bytesToSend = QByteArray::fromStdString( msg.toStdString() );
    return this->socket->write(bytesToSend);
}

bool TcpClient::is_alive()
{
	if(this->socket == Q_NULLPTR || this->socket == 0)
		return false;

    if (!this->socket->isValid())
        return false;

	if(this->socket->isOpen() && this->socket->isWritable())
		return true;
	else
		return false;
}

void TcpClient::doDisconnect()
{
    if(this->socket == Q_NULLPTR || this->socket == 0)
        return;

    if(this->is_alive())
    {
        this->socket->disconnectFromHost();
        this->socket->close();
    }

    delete this->socket;
    this->socket = Q_NULLPTR;
}
