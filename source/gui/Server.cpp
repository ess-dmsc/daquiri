#include "Server.h"
#include <core/util/logger.h>

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
    auto text = QString(server_socket->read(server_socket->bytesAvailable())).trimmed();

    if (text == "STOP")
    {
      INFO("<CommandServer> received STOP");
      send_response("<DAQuiri::CommandServer> OK: stopping all ongoing acquisitions\n");
      emit stopDAQ();
    }
    else if (text.startsWith("START_NEW"))
    {
      auto tokens = text.split(" ");
      QString name;
      if (tokens.size() > 1)
        name = tokens[1];
      INFO("<CommandServer> received START_NEW '{}'", name.toStdString());
      send_response("<DAQuiri::CommandServer> OK: opening new project and starting acquisition\n");
      emit startNewDAQ(name);
    }
    else if (text.startsWith("CLOSE_OLDER"))
    {
      auto tokens = text.split(" ");
      QString num;
      if (tokens.size() > 1)
        num = tokens[1];
      uint32_t mins = num.toInt();
      INFO("<CommandServer> received CLOSE_OLDER than {} minutes", mins);
      send_response("<DAQuiri::CommandServer> OK: closing older projects\n");
      emit close_older(mins);
    }
    else if (text == "SAVE")
    {
      INFO("<CommandServer> received SAVE");
      send_response("<DAQuiri::CommandServer> OK: saving\n");
      emit save();
    }
    else if (text == "DIE")
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
    DBG("<CommandServer> established TCP connection");
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
  Q_UNUSED(error);

  if (server_socket)
  {
    if (server_socket->error() == QAbstractSocket::RemoteHostClosedError)
    {
      DBG("<CommandServer> TCP connection terminated");
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