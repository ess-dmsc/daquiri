#include "Server.h"
#include <core/util/custom_logger.h>

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
    QString text(server_socket->read(server_socket->bytesAvailable()));

    if (text == "STOP\n")
    {
      INFO("<CommandServer> received STOP");
      send_response("<DAQuiri::CommandServer> OK: stopping all ongoing acquisitions\n");
      emit stopDAQ();
    }
    else if (text == "START_NEW\n")
    {
      INFO("<CommandServer> received START_NEW");
      send_response("<DAQuiri::CommandServer> OK: opening new project and starting acquisition\n");
      emit startNewDAQ();
    }
    else if (text == "DIE\n")
    {
      INFO("<CommandServer> received DIE");
      send_response("<DAQuiri::CommandServer> OK: shutting down\n");
      emit die();
    }
    else {
      WARN("<CommandServer> received unknown command '{}'", text.toStdString());
      send_response("<DAQuiri::CommandServer> ERROR: received unknown command '" + text + "'\n");
    }
  }
}

void CommandServer::send_response(QString msg)
{
  if (!server_socket)
    return;
  QByteArray out;
  out.append(msg);
  server_socket->write(out);
}


void CommandServer::acceptNew()
{
  auto nc = nextPendingConnection();
  if (nc) {
    INFO("<CommandServer> established TCP connection");
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
      INFO("<CommandServer> TCP connection terminated");
      server_socket = nullptr;
    }
    else
      ERR("<CommandServer> TCP error: {}", server_socket->errorString().toStdString());
  }
}

bool CommandServer::start_listen(int port_no)
{
  if (!this->listen(QHostAddress::Any, port_no))
  {
    ERR("<CommandServer> Error! Cannot listen to port {}", port_no);
    return false;
  }
  return true;
}

void CommandServer::incomingConnection(int descriptor)
{
  if (!server_socket || !server_socket->setSocketDescriptor(descriptor))
  {
    ERR("<CommandServer> Incoming connection socket error.");
    return;
  }
}