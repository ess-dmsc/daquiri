#include "Profiles.h"
#include "ui_Profiles.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include "project.h"
#include <boost/algorithm/string.hpp>
#include "qt_util.h"

#include "json_file.h"

#include "custom_logger.h"

using namespace DAQuiri;

int TableProfiles::rowCount(const QModelIndex & /*parent*/) const
{
  return profiles_.size();
}

int TableProfiles::columnCount(const QModelIndex & /*parent*/) const
{
  return 1;
}

QVariant TableProfiles::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();
  std::stringstream dss;

  if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
  {
    switch (col) {
    case 0:
      return QString::fromStdString(profiles_[row].find({"Profile description"}, Match::id).get_text());
      //    case 1:
      //      return QString::fromStdString(profiles_[row].value_text);
    }
  }
  return QVariant();
}

QVariant TableProfiles::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return "Decription";
        //      case 1:
        //        return "Path";
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

void TableProfiles::update()
{
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex( rowCount() - 1, columnCount() - 1 );
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags TableProfiles::flags(const QModelIndex &index) const
{
  return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
}




WidgetProfiles::WidgetProfiles(QWidget *parent)
  : QDialog(parent)
  , table_model_(profiles_)
  , selection_model_(&table_model_)
  , ui(new Ui::WidgetProfiles)
{
  ui->setupUi(this);

  update_profiles();

  ui->tableProfiles->setModel(&table_model_);
  ui->tableProfiles->setSelectionModel(&selection_model_);
  ui->tableProfiles->verticalHeader()->hide();
  ui->tableProfiles->horizontalHeader()->setStretchLastSection(true);
  ui->tableProfiles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableProfiles->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableProfiles->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableProfiles->show();


  connect(&selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));
  connect(ui->tableProfiles, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(selection_double_clicked(QModelIndex)));

}

WidgetProfiles::~WidgetProfiles()
{
  delete ui;
}

void WidgetProfiles::update_profiles()
{
  selection_model_.clearSelection();
  profile_dirs_.clear();
  profiles_.clear();

  QSettings settings;
  settings.beginGroup("Program");
  QString settings_directory =
      settings.value("settings_directory",
                     QDir::homePath() + "/daquiri/settings").toString();

  QDir directory(settings_directory + "/profiles");
  directory.setFilter(QDir::Dirs);
  QStringList subdirs = directory.entryList();
  for (auto &q : subdirs)
  {
    QDir dir2(settings_directory + "/profiles/" + q);
//    DBG << "Checking profile path " << dir2.path().toStdString();

    QStringList nameFilter("profile.set");

    QStringList profile_files = dir2.entryList(nameFilter);
    if (profile_files.empty())
    {
      DBG << "<WidgetProfiles> no profile found in " << dir2.absolutePath().toStdString();
      continue;
    }

    std::string path = dir2.absolutePath().toStdString() + "/profile.set";

    json profile;
    try
    {
      profile = from_json_file(path);
    }
    catch (...)
    {}

    if (profile.empty())
    {
      DBG << "<WidgetProfiles> Bad json in " << path;
      continue;
    }

    //    DBG << "<WidgetProfiles> valid profile at " << path;

    Setting tree = profile;

    if (!tree)
    {
      DBG << "<WidgetProfiles> Profile invalid in " << path;
      continue;
    }

    profiles_.push_back(tree);
    profile_dirs_.push_back(dir2.absolutePath().toStdString());
  }

  Setting tree = Setting::stem("Empty");
  tree.branches.add(Setting::text("Profile description", "Offline analysis mode"));

  profiles_.push_back(tree);
  profile_dirs_.push_back("");

  table_model_.update();
}

void WidgetProfiles::selection_changed(QItemSelection, QItemSelection)
{
  toggle_push();
}

void WidgetProfiles::toggle_push()
{
  ui->pushApply->setEnabled(!selection_model_.selectedIndexes().empty());
  ui->pushApplyBoot->setEnabled(!selection_model_.selectedIndexes().empty());
}

void WidgetProfiles::selection_double_clicked(QModelIndex idx)
{
  int i = idx.row();

  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("profile_directory", QString::fromStdString(profile_dirs_[i]));
  settings.setValue("boot_on_startup", true);

  emit profileChosen();
  accept();
}


void WidgetProfiles::on_pushApply_clicked()
{
  QModelIndexList ixl = ui->tableProfiles->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();

  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("profile_directory", QString::fromStdString(profile_dirs_[i]));
  settings.setValue("boot_on_startup", false);

  emit profileChosen();
  accept();
}

void WidgetProfiles::on_pushApplyBoot_clicked()
{
  QModelIndexList ixl = ui->tableProfiles->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();

  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("profile_directory", QString::fromStdString(profile_dirs_[i]));
  settings.setValue("boot_on_startup", true);

  emit profileChosen();
  accept();
}

void WidgetProfiles::on_OutputDirFind_clicked()
{
  QSettings settings;
  settings.beginGroup("Program");
  QString settings_directory =
      settings.value("settings_directory",
                     QDir::homePath() + "/daquiri/settings").toString();

  QString dirName = QFileDialog::getExistingDirectory(this, "Open Directory", settings_directory,
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dirName.isEmpty()) {
    settings_directory = QDir(dirName).absolutePath();
    settings.setValue("settings_directory", settings_directory);
    update_profiles();
  }
}
