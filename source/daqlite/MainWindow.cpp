
#include <MainWindow.h>
#include <QWidget>

MainWindow::MainWindow(Configuration &Config) : mConfig(Config) {
  Plot2D = new Custom2DPlot(mConfig);
  setupLayout();
  show();
  startKafkaConsumer();
}

void MainWindow::setupLayout() {
  setWindowTitle("Daquiri lite - test");
  resize(600, 500);

  // main vertical layout
  QVBoxLayout *pMainLayout = new QVBoxLayout(this);
  setLayout(pMainLayout);

  // ------------------------------------------------------
  // first sub layouts
  QHBoxLayout *pTopHBox = new QHBoxLayout;
  pMainLayout->addLayout(pTopHBox);

  QHBoxLayout *pBtnHBox = new QHBoxLayout;
  pMainLayout->addLayout(pBtnHBox);

  // ------------------------------------------------------
  // create the group box
  QGroupBox *plotBox = new QGroupBox(this);
  plotBox->setTitle(mConfig.mTopic.c_str());

  // create the layout for the plotBox
  QGridLayout *plotLayout = new QGridLayout;

  plotLayout->addWidget(Plot2D, 0, 0);
  // auto plot2 = new Custom2DPlot;
  // plotLayout->addWidget(plot2, 0, 1);
  plotBox->setLayout(plotLayout);
  pTopHBox->addWidget(plotBox);

  // ------------------------------------------------------
  // create the button area
  QPushButton *pBtn = new QPushButton(QObject::tr("Quit"), this);
  pBtnHBox->addWidget(pBtn);
  pBtnHBox->addItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

  connect(pBtn, SIGNAL(clicked()), this, SLOT(handleExitButton()));
}

void MainWindow::startKafkaConsumer() {
  KafkaConsumer = new WorkerThread(this, mConfig);
  qRegisterMetaType<int>("int&");
  connect(KafkaConsumer, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
  KafkaConsumer->start();
}

// SLOT
void MainWindow::handleKafkaData(int i) { Plot2D->addData(i); }

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }
