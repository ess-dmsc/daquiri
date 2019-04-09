#include "Consumer2D.h"
#include <widgets/qt_util.h>

#include <core/util/timer.h>

#include <core/util/logger.h>

using namespace DAQuiri;

Consumer2D::Consumer2D(QWidget* parent)
    : AbstractConsumerWidget(parent)
      , plot_(new QPlot::Plot2D())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);

//  plot_->setOpenGl(true);

  plot_->setSizePolicy(QSizePolicy::Preferred,
                       QSizePolicy::MinimumExpanding);
  plot_->setGradient("Spectrum2");
  plot_->setShowGradientLegend(true);
  plot_->clearAll();
  plot_->replotAll();
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent * )), this, SLOT(mouseWheel(QWheelEvent * )));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  connect(plot_, SIGNAL(scaleChanged(QString)), this, SLOT(scaleChanged(QString)));
  connect(plot_, SIGNAL(gradientChanged(QString)), this, SLOT(gradientChanged(QString)));
  connect(plot_, SIGNAL(flipYChanged(bool)), this, SLOT(flipYChanged(bool)));

  connect(plot_, SIGNAL(clickedPlot(double, double, Qt::MouseButton)), this,
          SLOT(clickedPlot(double, double, Qt::MouseButton)));

  setLayout(fl);
}

void Consumer2D::update()
{
  if (!this->isVisible()
      || !plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  //  Timer guiside(true);

  DataspacePtr data = consumer_->data();
  ConsumerMetadata md = consumer_->metadata();

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QS(new_label).trimmed());

  if (!initial_scale_)
  {
    auto st = md.get_attribute("preferred_scale");
    auto scale = st.metadata().enum_name(st.selection());
    plot_->setScaleType(QS(scale));

    auto app = md.get_attribute("appearance").get_text();
    plot_->setGradient(QS(app));
    initial_scale_ = true;

    auto flipy = md.get_attribute("flip-y").get_bool();
    plot_->setFlipY(flipy);
  }

  double rescale = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  QPlot::HistList2D hist;
  if (data)
  {
    auto spectrum_data = data->all_data();
    if (spectrum_data)
      for (const auto& p : *spectrum_data)
        hist.push_back(QPlot::p2d(p.first[0],
                                  p.first[1],
                                  rescale * to_double(p.second)));
  }

  if (!hist.empty())
  {
    DataAxis axis_x = data->axis(0);
    DataAxis axis_y = data->axis(1);

    x_domain = axis_x.domain;
    y_domain = axis_y.domain;

    uint32_t res_x = axis_x.bounds().second;
    uint32_t res_y = axis_y.bounds().second;

    plot_->clearExtras();
    plot_->clearData();
    plot_->setAxes(
        QS(axis_x.label()), x_domain[0], x_domain[res_x],
        QS(axis_y.label()), y_domain[0], y_domain[res_y],
        "count");

    if (box_visible)
    {
      QPlot::MarkerBox2D box;

      box.border = Qt::black;
      box.fill = QColor(0, 0, 0, 32);

      box.x1 = box.x2 = x_domain[box_x];
      if (box_x > 0)
      {
        box.x1 += x_domain[box_x - 1];
        box.x1 /= 2;
      }
      if ((box_x + 1u) < x_domain.size())
      {
        box.x2 += x_domain[box_x + 1];
        box.x2 /= 2;
      }

      box.y1 = box.y2 = y_domain[box_y];
      if (box_y > 0)
      {
        box.y1 += y_domain[box_y - 1];
        box.y1 /= 2;
      }
      if ((box_y + 1u) < y_domain.size())
      {
        box.y2 += y_domain[box_y + 1];
        box.y2 /= 2;
      }

      QPlot::Label2D label;
      label.text = QString::number(double(data->get({box_x, box_y})));
      label.x = box.x2;
      label.y = plot_->flipY() ? box.y2 : box.y1;

      plot_->addBoxes({box});
      plot_->addLabels({label});
    }

    plot_->updatePlot(res_x + 1, res_y + 1, hist);
    plot_->replotExtras();
  }

  if (!user_zoomed_)
    plot_->zoomOut();

  //  DBG( "<Plot2D> plotting took " << guiside.ms() << " ms";
}

void Consumer2D::scaleChanged(QString sn)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("preferred_scale");
  for (auto e : st.metadata().enum_map())
  {
    if (e.second == sn.toStdString())
    {
      DBG("Matched {}={}", e.second, e.first);
      st.select(e.first);
      break;
    }
  }
  consumer_->set_attribute(st);
}

void Consumer2D::gradientChanged(QString g)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("appearance");
  st.set_text(g.toStdString());
  consumer_->set_attribute(st);
}

void Consumer2D::flipYChanged(bool flip_y)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("flip-y");
  st.set_bool(flip_y);
  consumer_->set_attribute(st);
}

void Consumer2D::refresh()
{
  plot_->replot(QCustomPlot::rpQueuedRefresh);
}

void Consumer2D::mouseWheel(QWheelEvent* event)
{
  Q_UNUSED(event)
  user_zoomed_ = true;
}

void Consumer2D::zoomedOut()
{
  user_zoomed_ = false;
}

void Consumer2D::clickedPlot(double x, double y, Qt::MouseButton button)
{
  box_visible = (button == Qt::MouseButton::LeftButton) &&
      (x >= 0) && (x < x_domain.size()) &&  (y >= 0) && (y < y_domain.size());

  box_x = static_cast<uint16_t>(x);
  box_y = static_cast<uint16_t>(y);

  update();
}



