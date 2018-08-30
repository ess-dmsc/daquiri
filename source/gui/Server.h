#pragma once

#include <QtNetwork>

class CommandServer : public QTcpServer
{
  Q_OBJECT
 public:
  explicit CommandServer(QObject* parent = 0);
  ~CommandServer();

 signals:
  void stopDAQ();
  void startNewDAQ();
  void die();

 public slots:
  void tcpReady();
  void tcpError(QAbstractSocket::SocketError error);
  bool start_listen(int port_no);
  void acceptNew();
 protected:
  void incomingConnection(int descriptor);

  QTcpSocket* server_socket {nullptr};

  void send_response(QString);
};
