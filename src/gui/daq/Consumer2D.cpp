#include "Consumer2D.h"
#include "custom_timer.h"

#include "custom_logger.h"

using namespace DAQuiri;

Consumer2D::Consumer2D(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , plot_ (new QPlot::Plot2D())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);
  plot_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  setLayout(fl);


  QWidget *popup = new QWidget(this);

  crop_slider_ = new QSlider(Qt::Vertical, popup);
  crop_slider_->setRange(0, 100);

  crop_label_ = new QLabel(popup);
  crop_label_->setAlignment(Qt::AlignCenter);
  crop_label_->setNum(100);
  crop_label_->setMinimumWidth(crop_label_->sizeHint().width());

  typedef void(QLabel::*IntSlot)(int);
  connect(crop_slider_, &QAbstractSlider::valueChanged, crop_label_, static_cast<IntSlot>(&QLabel::setNum));

  QBoxLayout *popupLayout = new QHBoxLayout(popup);
  popupLayout->setMargin(2);
  popupLayout->addWidget(crop_slider_);
  popupLayout->addWidget(crop_label_);

  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(popup);

  crop_menu_ = new QMenu(this);
  crop_menu_->addAction(action);

  plot_->setShowGradientLegend(true);


  //  ui->toolCrop->setMenu(crop_menu_);
  //  ui->toolCrop->setPopupMode(QToolButton::InstantPopup);
  //  connect(crop_menu_, SIGNAL(aboutToHide()), this, SLOT(crop_changed()));

//  crop_slider_->setValue(50);
//  crop_changed();
}


void Consumer2D::reset_content()
{
  //DBG << "reset content";
  plot_->clearAll();
  plot_->replot();
  resolution_x_ = 0;
  resolution_y_ = 0;
}

void Consumer2D::update()
{
  if (!this->isVisible()
      || !plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  //  CustomTimer guiside(true);

  ConsumerMetadata md = consumer_->metadata();

//  bool buffered = md.get_attribute("buffered").triggered();
//  PreciseFloat total = md.get_attribute("total_count").get_number();
  uint16_t bits = md.get_attribute("resolution").selection();
  uint32_t res_x = pow(2,bits) * zoom_;
  uint32_t res_y = res_x;

  bool zoomout = (res_x != resolution_x_) || (res_y != resolution_y_);

  resolution_x_ = res_x;
  resolution_y_ = res_y;

  if (!resolution_x_ || !resolution_y_)
  {
    plot_->clearAll();
    plot_->replot();
    return;
  }


  double rescale  = md.get_attribute("rescale").get_number();
  std::shared_ptr<EntryList> spectrum_data =
      std::move(consumer_->data_range({{0, resolution_x_}, {0, resolution_y_}}));

  QPlot::HistList2D hist;
  if (spectrum_data)
    for (auto p : *spectrum_data)
      hist.push_back(QPlot::p2d(p.first.at(0),
                                p.first.at(1),
                                rescale * to_double(p.second)));

  if (!hist.empty())
  {
    plot_->clearExtras();
    plot_->updatePlot(resolution_x_ + 1, resolution_y_ + 1, hist);
    plot_->setAxes(
          "", 0, resolution_x_+1,
          "", 0, resolution_y_+1,
          "Event count");
    plot_->replotExtras();
    plot_->replot();
  }

  std::string new_label = md.get_attribute("name").get_text();
//  plot_->setTitle(QString::fromStdString(new_label).trimmed());
  setWindowTitle(QString::fromStdString(new_label).trimmed());

  if (zoomout)
    plot_->zoomOut();

  //  DBG << "<Plot2D> plotting took " << guiside.ms() << " ms";
}

void Consumer2D::set_zoom(double zm)
{
  if (zm > 1.0)
    zm = 1.0;
  zoom_ = zm;
  //  ui->toolCrop->setText(QString::number(zm * 100) + "% ");
  crop_slider_->setValue(zm * 100);
  update();
}

void Consumer2D::crop_changed()
{
//  DBG << "changing zoom_";
  zoom_ = crop_slider_->value() / 100.0;
//  ui->toolCrop->setText(QString::number(crop_slider_->value()) + "% ");
  update();
}


double Consumer2D::zoom()
{
  return zoom_;
}

