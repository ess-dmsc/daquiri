#include <gui/ProfilesForm.h>
#include "ui_ProfilesForm.h"

#include <gui/Profiles.h>
#include <gui/widgets/qt_util.h>
#include <QMessageBox>
#include <QBoxLayout>
#include <QInputDialog>

#include <core/util/logger.h>

using namespace DAQuiri;

ProfileDialog::ProfileDialog(QString description, QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle("Profile actions");

  QPushButton *buttonBoot = new QPushButton("Boot", this);
  buttonBoot->setIcon(QIcon(":/icons/boot16.png"));
  connect(buttonBoot, SIGNAL(clicked()), this, SLOT(clickedBoot()));

  QPushButton *buttonLoad = new QPushButton("Load", this);
  buttonLoad->setIcon(QIcon(":/icons/oxy/16/games_endturn.png"));
  connect(buttonLoad, SIGNAL(clicked()), this, SLOT(clickedLoad()));

  QPushButton *buttonRemove = new QPushButton("Remove", this);
  buttonRemove->setIcon(QIcon(":/icons/oxy/16/editdelete.png"));
  connect(buttonRemove, SIGNAL(clicked()), this, SLOT(clickedRemove()));

  QPushButton *buttonCancel = new QPushButton("Cancel", this);
//  buttonCancel->setIcon(QIcon(":/icons/oxy/16/editdelete.png"));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(clickedCancel()));

  QVBoxLayout *layout = new QVBoxLayout();

  description = "Select action for\n" + description;

  layout->addWidget(new QLabel(description));

  layout->addWidget(buttonBoot);
  layout->addWidget(buttonLoad);
  layout->addWidget(buttonRemove);
  layout->addWidget(buttonCancel);

  setLayout(layout);
}

void ProfileDialog::clickedLoad()
{
  emit load();
  accept();
}

void ProfileDialog::clickedBoot()
{
  emit boot();
  accept();
}

void ProfileDialog::clickedRemove()
{
  emit remove();
  accept();
}

void ProfileDialog::clickedCancel()
{
  accept();
}


ProfilesForm::ProfilesForm(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ProfilesForm)
{
  ui->setupUi(this);

  ui->tableProfiles2->verticalHeader()->hide();
  ui->tableProfiles2->horizontalHeader()->hide();
  ui->tableProfiles2->horizontalHeader()->setStretchLastSection(true);
  ui->tableProfiles2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableProfiles2->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableProfiles2->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(ui->tableProfiles2, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(selection_double_clicked(QModelIndex)));

  update_profiles();
}

ProfilesForm::~ProfilesForm()
{
  delete ui;
}

void ProfilesForm::update_profiles()
{
  profiles_.clear();
  profiles_.push_back({"", "(Offline mode)"});

  auto dir = Profiles::profiles_dir();

  QDir directory(dir);
  directory.setFilter(QDir::Dirs);
  QStringList subdirs = directory.entryList();

  for (auto &q : subdirs)
  {
    Setting tree = Profiles::get_profile(q);

    if (!tree)
      continue;

    auto description = tree.find({"ProfileDescr"}, Match::id);
    profiles_.push_back({q, QS(description.get_text())});
  }

  ui->tableProfiles2->clear();
  ui->tableProfiles2->setRowCount(profiles_.size() + 1);
  ui->tableProfiles2->setColumnCount(2);

  auto thisprofile = Profiles::singleton().current_profile_name();
  for (auto i=0; i < profiles_.size(); ++i)
  {
    QBrush background = (thisprofile == profiles_[i].id) ?
          QBrush(Qt::green) : QBrush(Qt::white);

    add_to_table(ui->tableProfiles2, i, 0,
                 profiles_[i].id,
                 QVariant(), background);

    add_to_table(ui->tableProfiles2, i, 1,
                 profiles_[i].description,
                 QVariant(), background);

  }

  QTableWidgetItem *icon_item = new QTableWidgetItem;
  icon_item->setIcon(QIcon(":/icons/oxy/16/edit_add.png"));
  icon_item->setFlags(icon_item->flags() ^ Qt::ItemIsEditable);
  ui->tableProfiles2->setItem(profiles_.size(), 0, icon_item);

  add_to_table(ui->tableProfiles2, profiles_.size(), 1, "CREATE NEW");
  QFont font;
  font.setBold(true);
  ui->tableProfiles2->item(profiles_.size(), 1)->setFont(font);
}

void ProfilesForm::apply_selection(size_t i, bool boot)
{
  emit profileChosen(profiles_[i].id, boot);
  accept();
}

void ProfilesForm::selection_double_clicked(QModelIndex idx)
{
  if (idx.row() == profiles_.size())
    create_profile();
  else
  {
    auto mm = new ProfileDialog(profiles_[idx.row()].description, this);
    connect(mm, SIGNAL(load()), this, SLOT(select_no_boot()));
    connect(mm, SIGNAL(boot()), this, SLOT(select_and_boot()));
    connect(mm, SIGNAL(remove()), this, SLOT(remove_profile()));
    mm->exec();
  }
}

void ProfilesForm::select_and_boot()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  apply_selection(ixl.front().row(), true);
}

void ProfilesForm::select_no_boot()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  apply_selection(ixl.front().row(), false);
}

void ProfilesForm::on_pushSelectRoot_clicked()
{
  QString dirName =
      QFileDialog::getExistingDirectory(this, "Open Directory", Profiles::settings_dir(),
                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dirName.isEmpty())
    return;

  if (!Profiles::is_valid_settings_dir(dirName))
  {
    QMessageBox msgBox;
    msgBox.setText("Not a valid settings root. Must contain a valid 'profiles' subdirectory.");
    msgBox.exec();
    return;
  }

  Profiles::select_settings_dir(dirName);
  update_profiles();
}

void ProfilesForm::create_profile()
{
  bool ok;
  QString name = QInputDialog::getText(this, tr("Profile directory"),
                                       tr("Subdirectory for new profile:"),
                                       QLineEdit::Normal, "", &ok);
  if (!ok && name.isEmpty())
    return;

  if (Profiles::profile_exists(name))
  {
    DBG("Profile '{}' already exists", name.toStdString());
    //ask user overwrite?
    return;
  }

  Profiles::create_profile(name);
  update_profiles();
}

void ProfilesForm::remove_profile()
{
  QModelIndexList ixl = ui->tableProfiles2->selectionModel()->selectedRows();
  if (ixl.empty() || !ixl.front().row())
    return;

  QMessageBox msgBox;
  msgBox.setText("Remove profile?");
  msgBox.setInformativeText("Do you want remove this profile?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No
                            /*| QMessageBox::Cancel*/);
  msgBox.setDefaultButton(QMessageBox::No);
  if (msgBox.exec() == QMessageBox::Yes)
  {
    Profiles::remove_profile(profiles_[ixl.front().row()].id);
    update_profiles();
  }
}
