#include "changesavelocation.h"
#include "ui_changesavelocation.h"
#include <QFileDialog>
#include <QMainWindow>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

ChangeSaveLocation::ChangeSaveLocation(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangeSaveLocation)
{
    ui->setupUi(this);
    load_save_location();
}

ChangeSaveLocation::~ChangeSaveLocation()
{
    delete ui;
}

void ChangeSaveLocation::load_save_location()
{
    QString app_dir = QCoreApplication::applicationDirPath();
    QString json_path = app_dir + "/save_location.json";
    ifstream save_json(json_path.toStdString());
    json read_data;
    save_json >> read_data;
    QString save_folder = QString::fromStdString(read_data["location"].get<string>());
    ui->save_location->setText(save_folder);
    ui->save_location->setReadOnly(true);
    ui->save_button->setEnabled(false);
}

void ChangeSaveLocation::on_change_folder_clicked()
{
    QString new_save_location = QFileDialog::getExistingDirectory(NULL, "Select Save Folder", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!new_save_location.isEmpty())
    {
        ui->save_location->setReadOnly(false);
        ui->save_location->setText(new_save_location);
        ui->save_location->setReadOnly(true);
        ui->save_button->setEnabled(true);
    }
}

void ChangeSaveLocation::on_save_button_clicked()
{
    QString app_dir = QCoreApplication::applicationDirPath();
    QString json_path = app_dir + "/save_location.json";
    json save_data;
    save_data["location"] = (ui->save_location->toPlainText()).toStdString();
    ofstream file(json_path.toStdString());
    if (file.is_open())
    {
        file << save_data.dump(4);
        file.close();
        ui->save_button->setEnabled(false);
    }
}

void ChangeSaveLocation::on_cancel_button_clicked()
{
    this->close();
}

