#include "Profiles.h"
#include "ui_Profiles.h"
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include "qt_util.h"

#include "json_file.h"

#include "custom_logger.h"

#include <QInputDialog>
#include "engine.h"

using namespace DAQuiri;

WidgetProfiles::WidgetProfiles(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::WidgetProfiles)
{
  ui->setupUi(this);

  ui->tableProfiles2->verticalHeader()->hide();
  ui->tableProfiles2->horizontalHeader()->hide();
  ui->tableProfiles2->horizontalHeader()->setStretchLastSection(true);
  ui->tableProfiles2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableProfiles2->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableProfiles2->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(ui->tableProfiles2->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));
  connect(ui->tableProfiles2, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(selection_double_clicked(QModelIndex)));

  update_profiles();
}

WidgetProfiles::~WidgetProfiles()
{
  delete ui;
}


QString WidgetProfiles::settings_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("settings_directory",
                        QDir::homePath() + "/daquiri/settings").toString();
}

QString WidgetProfiles::profiles_dir()
{
  return settings_dir() + "/profiles";
}

QString WidgetProfiles::current_profile_dir()
{
  QSettings settings;
  settings.beginGroup("Program");
  return settings.value("profile_directory","").toString();
}

void WidgetProfiles::update_profiles()
{
  profile_dirs_.clear();
  std::vector<std::string> descriptions;

  descriptions.push_back("(Offline mode)");
  profile_dirs_.push_back("");

  auto dir = profiles_dir();
  auto thisprofile = QDir(current_profile_dir());

  QDir directory(dir);
  directory.setFilter(QDir::Dirs);
  QStringList subdirs = directory.entryList();

  for (auto &q : subdirs)
  {
    QDir dir2(dir + "/" + q);

    Setting tree;
    try
    {
      json profile =
          from_json_file(dir2.absolutePath().toStdString() + "/profile.set");
      tree = profile;
    }
    catch (...)
    {}

    if (!tree)
    {
      DBG << "<WidgetProfiles> No valid profile in "
          << dir2.absolutePath().toStdString();
      continue;
    }

    descriptions.push_back(tree.find({"Profile description"}, Match::id).get_text());
    profile_dirs_.push_back(dir2.absolutePath().toStdString());
  }

  ui->tableProfiles2->clear();
  ui->tableProfiles2->setRowCount(descriptions.size());
  ui->tableProfiles2->setColumnCount(1);
  for (size_t i=0; i < descriptions.size(); ++i)
    add_to_table(ui->tableProfiles2, i, 0, descriptions[i], QVariant(),
                 (thisprofile.absolutePath().toStdString() == profile_dirs_[i]) ?
                   QBrush(Qt::green) : QBrush(Qt::white));
}

void WidgetProfiles::selection_changed(QItemSelection, QItemSelection)
{
  bool selected = !ui->tableProfiles2->selectionModel()->
      selectedIndexes().empty();
  ui->pushApply->setEnabled(selected);
  ui->pushApplyBoot->setEnabled(selected);
  ui->pushDelete->setEnabled(selected);
}

void WidgetProfiles::apply_selection(size_t i, bool boot)
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("profile_directory", QString::fromStdString(profile_dirs_[i]));
  settings.setValue("boot_on_startup", boot);

  emit profileChosen();
  accept();
}

void WidgetProfiles::selection_double_clicked(QModelIndex idx)
{
  apply_selection(idx.row(), true);
}

void WidgetProfiles::on_pushApplyBoot_clicked()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  apply_selection(ixl.front().row(), true);
}

void WidgetProfiles::on_pushApply_clicked()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  apply_selection(ixl.front().row(), false);
}

void WidgetProfiles::on_OutputDirFind_clicked()
{
  QString dirName =
      QFileDialog::getExistingDirectory(this, "Open Directory", settings_dir(),
                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dirName.isEmpty())
    return;
  QSettings settings;
  settings.setValue("settings_directory", QDir(dirName).absolutePath());
  update_profiles();
}

void WidgetProfiles::on_pushCreate_clicked()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Profile directory"),
                                       tr("Subdirectory for new profile:"),
                                       QLineEdit::Normal, "", &ok);
  if (!ok && text.isEmpty())
    return;

  auto sd = profiles_dir() + "/" + text;

  DBG << "new dir " << sd.toStdString();

  if (QDir(sd).exists())
  {
    DBG << "Already exists";
    return;
  }

  text = QInputDialog::getText(this, tr("Profile description"),
                               tr("Description for profile:"),
                               QLineEdit::Normal, "", &ok);
  if (!ok)
    return;

  QDir().mkdir(sd);

  auto profile = DAQuiri::Engine::singleton().default_settings();
  if (!text.isEmpty())
    profile.set(Setting::text("Profile description", text.toStdString()));

  DBG << profile.debug();

  auto path = sd.toStdString() + "/profile.set";
  DBG << "Will save to " << path;
  profile.condense();
  profile.strip_metadata();
  to_json_file(profile, path);

  update_profiles();
}

void WidgetProfiles::on_pushDelete_clicked()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  QDir path(QString::fromStdString(profile_dirs_[ixl.front().row()]));
  path.removeRecursively();
  update_profiles();
}
