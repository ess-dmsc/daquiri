#include "Server.h"
#include <QMessageBox>
#include <QDebug>

CommandServer::CommandServer(QObject* parent) : QTcpServer(parent)
{
  connect(this, SIGNAL(newConnection()), this, SLOT(acceptNew()));
}

CommandServer::~CommandServer()
{
  if (server_socket)
  {
    server_socket->disconnectFromHost();
    server_socket->waitForDisconnected();
  }
}

void CommandServer::tcpReady()
{
  if (server_socket)
  {
    QByteArray array = server_socket->read(server_socket->bytesAvailable());
    qDebug() << "received data " << array;

    QString text(array);
    if (text == "STOP\n")
    {
      qDebug() << "emitting stop";
      emit stopDAQ();
    }
    else if (text == "START_NEW\n")
    {
      emit startNewDAQ();
    }

    QByteArray out;
    out.append(QString("<OK>\n"));
    server_socket->write(out);
  }
}

void CommandServer::acceptNew()
{
  qDebug() << "incoming connection";
  auto nc = nextPendingConnection();
  if (nc) {
    qDebug() << "good incoming connection";
    server_socket = nc;
    connect(server_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(tcpError(QAbstractSocket::SocketError)));
    connect(server_socket, SIGNAL(readyRead()),
            this, SLOT(tcpReady()));
    server_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
  }
}

void CommandServer::tcpError(QAbstractSocket::SocketError error)
{
  if (server_socket)
  {
    if (server_socket->error() == QAbstractSocket::RemoteHostClosedError)
    {
      qDebug() << "connection terminated";
      server_socket = nullptr;
    }
    else
      QMessageBox::warning(static_cast<QWidget*>(this->parent()),
          "Error", QString("TCP error: %1").arg(server_socket->errorString()));
  }
}

bool CommandServer::start_listen(int port_no)
{
  if (!this->listen(QHostAddress::Any, port_no))
  {
    QMessageBox::warning((QWidget * )
    this->parent(), "Error!", QString("Cannot listen to port %1").arg(port_no));
  }
  else
    return true;
}

void CommandServer::incomingConnection(int descriptor)
{
  if (!server_socket || !server_socket->setSocketDescriptor(descriptor))
  {
    QMessageBox::warning(static_cast<QWidget*>(this->parent()),
        "Error!", "Socket error!");
    return;
  }
}