// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file sniffer.cpp
///
/// \brief ev42 schema 's niffer'
///
/// Identifies types of modes
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>
#include <QApplication>
#include <QCommandLineParser>
#include <stdio.h>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCommandLineParser CLI;
  CLI.setApplicationDescription("Daquiri light - when you're driving home");
  CLI.addHelpOption();

  QCommandLineOption kafkaBrokerOption("b", "Kafka broker", "unusedDefault");
  CLI.addOption(kafkaBrokerOption);

  CLI.process(app);


  ESSConsumer consumer(CLI.value(kafkaBrokerOption).toStdString(), "amor_detector");
   while (true) {
     auto Msg = consumer.consume();
     consumer.handleMessage(Msg);
   }
}
