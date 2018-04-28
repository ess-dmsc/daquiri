#pragma once

#include "AbstractConsumerWidget.h"
#include <QProgressBar>
#include <QLabel>

class ConsumerScalar : public AbstractConsumerWidget
{
    Q_OBJECT

  public:
    ConsumerScalar(QWidget *parent = 0);

    void update() override;
    void refresh() override;

  private slots:
    void mouseWheel (QWheelEvent *event);
    void zoomedOut();
    void scaleChanged(QString);

  private:
    QProgressBar* plot_ {nullptr};
    QLabel* label_ {nullptr};
    bool initial_scale_ {false};
    bool user_zoomed_ {false};
};
