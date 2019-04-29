#include <gui/daq/ConsumerMulti1D.h>
#include <QIcon>
#include <QVBoxLayout>
#include <QPlot/QHist.h>
#include <gui/widgets/qt_util.h>

#include <core/util/logger.h>

using namespace DAQuiri;

ConsumerMulti1D::ConsumerMulti1D(QWidget* parent)
    : QWidget(parent)
      , plot_(new QPlot::Multi1D())
{
  setWindowIcon(QIcon(":/icons/noun_583391_cc_b.png"));

  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);

//  plot_->setOpenGl(true);

  plot_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  plot_->setLineThickness(2);
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent * )), this, SLOT(mouseWheel(QWheelEvent * )));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  connect(plot_, SIGNAL(scaleChanged(QString)), this, SLOT(scaleChanged(QString)));

  connect(plot_, SIGNAL(clickedPlot(double, double, Qt::MouseButton)), this,
          SLOT(clickedPlot(double, double, Qt::MouseButton)));

  setLayout(fl);
}

void ConsumerMulti1D::update_data()
{
  if (!plot_ || !project_)
    return;

  if (group_ == invalid_group)
  {
    plot_->clearAll();
    return;
  }

  plot_->clearPrimary();
  bool empty{true};

  std::string new_label;

  for (const auto& consumer : project_->get_consumers())
  {
    ConsumerMetadata md = consumer->metadata();

    if (md.get_attribute("window_group").get_int() != group_)
      continue;

    if (!md.get_attribute("visible").get_bool())
      continue;

    if (consumer->dimensions() != 1)
      continue;

    double rescale = md.get_attribute("rescale").get_number();
    if (!std::isfinite(rescale) || (rescale == 0.0))
      rescale = 1.0;

    if (!initial_scale_)
    {
      auto st = md.get_attribute("preferred_scale");
      auto scale = st.metadata().enum_name(st.selection());
      plot_->setScaleType(QS(scale));
      initial_scale_ = true;
    }

    auto pen = QPen(QColor(QS(md.get_attribute("appearance").get_text())), 1);

    DataAxis axis;
    EntryList spectrum_data;

    DataspacePtr data = consumer->data();
    if (data)
    {
      axis = data->axis(0);
      auto bounds = axis.bounds();
      if (md.get_attribute("trim").get_bool() &&
          ((bounds.second - bounds.first) > 1))
      {
        bounds.second--;
      }

      spectrum_data = data->range({bounds});
    }

    QPlot::HistMap1D hist;
    if (spectrum_data)
    {
      for (auto it : *spectrum_data)
      {
        double xx = axis.domain[it.first[0]];
        double yy = to_double(it.second) * rescale;
        hist[xx] = yy;
      }
    }

    auto name = md.get_attribute("name").get_text();

    if (!hist.empty())
    {
      plot_->addGraph(hist, pen, QS(name));
      empty = false;
    }

    plot_->setAxisLabels(QS(axis.label()), "count");
    if (!new_label.empty())
      new_label += " + ";
    new_label += name;
  }

  if (!empty)
  {
    plot_->setMarkers({marker});
    plot_->replotExtras();
    plot_->legend->setVisible(true);
  }

  setWindowTitle(QS(new_label).trimmed());

  if (!user_zoomed_)
    plot_->zoomOut();
}

void ConsumerMulti1D::scaleChanged(QString sn)
{
  if (!plot_ || !project_ || (group_ == invalid_group))
    return;

  for (const auto& consumer : project_->get_consumers())
  {
    ConsumerMetadata md = consumer->metadata();

    if (md.get_attribute("window_group").get_int() != group_)
      continue;

    auto st = md.get_attribute("preferred_scale");
    for (auto e : st.metadata().enum_map())
    {
      if (e.second == sn.toStdString())
      {
        st.select(e.first);
        break;
      }
    }
    consumer->set_attribute(st);
  }
}

void ConsumerMulti1D::refresh()
{
  plot_->replot(QCustomPlot::rpQueuedRefresh);
}

void ConsumerMulti1D::mouseWheel(QWheelEvent* event)
{
  Q_UNUSED(event)
  user_zoomed_ = true;
}

void ConsumerMulti1D::zoomedOut()
{
  user_zoomed_ = false;
}

void ConsumerMulti1D::clickedPlot(double x, double y, Qt::MouseButton button)
{
  marker.appearance.default_pen = QPen(Qt::black, 2);
  marker.pos = x;
  marker.closest_val = y;
  marker.alignment = Qt::AlignAbsolute;
  marker.visible = (button == Qt::MouseButton::LeftButton);
  //&& (x >= 0) && (x < x_domain.size());

  update_data();
}
