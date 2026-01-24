#include "archivepreferences.h"
#include "ui_archivepreferences.h"
#include <QFileInfo>
#include <fstream>
#include "json.hpp"

ArchivePreferences::ArchivePreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArchivePreferences)
{
    ui->setupUi(this);
    load_archive_preferences();
    ui->save_preferences->setEnabled(false);
    ui->cancel_preferences->setEnabled(false);
}

ArchivePreferences::~ArchivePreferences()
{
    delete ui;
}

using json = nlohmann::json;
using namespace std;

struct iso_naming
{
    Qt::CheckState enforce_83;
    Qt::CheckState uppercase;
    Qt::CheckState remove_invalid;
};

QPair<QString, QString> initial_zip_config;
QPair<QString, QString> initial_7zip_config;
QPair<QString, Qt::CheckState> initial_tar_config;
QPair<QString, Qt::CheckState> initial_xar_config;
QPair<QString, Qt::CheckState> initial_cpio_config;
iso_naming initial_iso_config;
QPair<QString, Qt::CheckState> initial_ar_config;

void ArchivePreferences::fetch_base_preferences()
{
    initial_zip_config.first = ui->zip_comp_method->currentText();
    initial_zip_config.second = ui->zip_level_label->text();
    initial_7zip_config.first = ui->seven_zip_comp_method->currentText();
    initial_7zip_config.second = ui->seven_zip_level_label->text();
    initial_tar_config.first = ui->tar_comp_method->currentText();
    initial_tar_config.second = ui->tar_metadata_cb->checkState();
    initial_xar_config.first = ui->xar_comp_method->currentText();
    initial_xar_config.second = ui->xar_metadata_cb->checkState();
    initial_cpio_config.first = ui->cpio_format->currentText();
    initial_cpio_config.second = ui->cpio_metadata_cb->checkState();
    initial_iso_config.enforce_83 = ui->iso_enforce_83_cb->checkState();
    initial_iso_config.uppercase = ui->iso_uppercase_cb->checkState();
    initial_iso_config.remove_invalid = ui->iso_rm_invalid_cb->checkState();
    initial_ar_config.first = ui->ar_format->currentText();
    initial_ar_config.second = ui->ar_determ_mode_cb->checkState();
}

void ArchivePreferences::set_zip_level_slider(QString level)
{
    map<string, int> level_map =
    {
        {"N/A", 0},
        {"Store", 1},
        {"Fast", 2},
        {"Normal", 3},
        {"Maximum", 4}
    };
    string level_str = level.toStdString();
    if (level_map.find(level_str) != level_map.end())
    {
        ui->zip_comp_slider->setValue(level_map[level_str]);
    }
    ui->zip_level_label->setText(level);
}

void ArchivePreferences::set_7zip_level_slider(QString level)
{
    map<string, int> level_map =
        {
            {"N/A", 0},
            {"Store", 1},
            {"Fast", 2},
            {"Normal", 3},
            {"Maximum", 4}
        };
    string level_str = level.toStdString();
    if (level_map.find(level_str) != level_map.end())
    {
        ui->seven_zip_comp_slider->setValue(level_map[level_str]);
    }
    ui->seven_zip_level_label->setText(level);
}

void ArchivePreferences::load_archive_preferences()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (save_json.is_open())
    {
        json load_data;
        save_json >> load_data;
        save_json.close();
        if (load_data.contains("archive"))
        {
            auto archive_preferences = load_data["archive"];
            QString zip_comp_method = QString::fromStdString(archive_preferences["zip"]["comp_method"]);
            int zip_method_found = ui->zip_comp_method->findText(zip_comp_method, Qt::MatchFixedString);
            if (zip_method_found >= 0) {ui->zip_comp_method->setCurrentIndex(zip_method_found);}
            QString zip_comp_level = QString::fromStdString(archive_preferences["zip"]["comp_level"]);
            set_zip_level_slider(zip_comp_level);
            QString seven_zip_comp_method = QString::fromStdString(archive_preferences["seven_zip"]["comp_method"]);
            int seven_zip_method_found = ui->seven_zip_comp_method->findText(seven_zip_comp_method, Qt::MatchFixedString);
            if (seven_zip_method_found >= 0) {ui->seven_zip_comp_method->setCurrentIndex(seven_zip_method_found);}
            QString seven_zip_comp_level = QString::fromStdString(archive_preferences["seven_zip"]["comp_level"]);
            set_7zip_level_slider(seven_zip_comp_level);
            QString tar_comp_method = QString::fromStdString(archive_preferences["tar"]["comp_method"]);
            int tar_method_found = ui->tar_comp_method->findText(tar_comp_method, Qt::MatchFixedString);
            if (tar_method_found >= 0) {ui->tar_comp_method->setCurrentIndex(tar_method_found);}
            bool tar_keep_metadata = archive_preferences["tar"]["pres_metadata"][0];
            if (tar_keep_metadata) {ui->tar_metadata_cb->setCheckState(Qt::Checked);}
            QString xar_comp_method = QString::fromStdString(archive_preferences["xar"]["comp_method"]);
            int xar_method_found = ui->xar_comp_method->findText(xar_comp_method, Qt::MatchFixedString);
            if (xar_method_found >= 0) {ui->xar_comp_method->setCurrentIndex(xar_method_found);}
            bool xar_keep_metadata = archive_preferences["xar"]["pres_metadata"][0];
            if (xar_keep_metadata) {ui->xar_metadata_cb->setCheckState(Qt::Checked);}
            QString cpio_format = QString::fromStdString(archive_preferences["cpio"]["format"]);
            int cpio_format_found = ui->cpio_format->findText(cpio_format, Qt::MatchFixedString);
            if (cpio_format_found >= 0) {ui->cpio_format->setCurrentIndex(cpio_format_found);}
            bool cpio_keep_metadata = archive_preferences["cpio"]["pres_metadata"][0];
            if (cpio_keep_metadata) {ui->cpio_metadata_cb->setCheckState(Qt::Checked);}
            bool iso_enforce_83 = archive_preferences["iso"]["enforce_83"][0];
            if (iso_enforce_83) {ui->iso_enforce_83_cb->setCheckState(Qt::Checked);}
            bool iso_uppercase = archive_preferences["iso"]["uppercase"][0];
            if (iso_uppercase) {ui->iso_uppercase_cb->setCheckState(Qt::Checked);}
            bool iso_rm_invalid = archive_preferences["iso"]["rm_invalid"][0];
            if (iso_rm_invalid) {ui->iso_rm_invalid_cb->setCheckState(Qt::Checked);}
            QString ar_format = QString::fromStdString(archive_preferences["ar"]["format"]);
            int ar_format_found = ui->ar_format->findText(ar_format, Qt::MatchFixedString);
            if (ar_format_found >= 0) {ui->ar_format->setCurrentIndex(ar_format_found);}
            bool ar_determ_mode = archive_preferences["ar"]["determ_mode"][0];
            if (ar_determ_mode) {ui->ar_determ_mode_cb->setCheckState(Qt::Checked);}
        }
    }
    fetch_base_preferences();
}

void ArchivePreferences::check_boxes_states()
{
    if (initial_zip_config.first == ui->zip_comp_method->currentText() &&
        initial_zip_config.second == ui->zip_level_label->text() &&
        initial_7zip_config.first == ui->seven_zip_comp_method->currentText() &&
        initial_7zip_config.second == ui->seven_zip_level_label->text() &&
        initial_tar_config.first == ui->tar_comp_method->currentText() &&
        initial_tar_config.second == ui->tar_metadata_cb->checkState() &&
        initial_xar_config.first == ui->xar_comp_method->currentText() &&
        initial_xar_config.second == ui->xar_metadata_cb->checkState() &&
        initial_cpio_config.first == ui->cpio_format->currentText() &&
        initial_cpio_config.second == ui->cpio_metadata_cb->checkState() &&
        initial_iso_config.enforce_83 == ui->iso_enforce_83_cb->checkState() &&
        initial_iso_config.uppercase == ui->iso_uppercase_cb->checkState() &&
        initial_iso_config.remove_invalid == ui->iso_rm_invalid_cb->checkState() &&
        initial_ar_config.first == ui->ar_format->currentText() &&
        initial_ar_config.second == ui->ar_determ_mode_cb->checkState())
    {
        ui->save_preferences->setEnabled(false);
        ui->cancel_preferences->setEnabled(false);
    }
    else
    {
        ui->save_preferences->setEnabled(true);
        ui->cancel_preferences->setEnabled(true);
    }
}

void ArchivePreferences::on_save_preferences_clicked()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    json preference_data;
    json archive_data;
    ifstream input_file(json_path.toStdString());
    if (input_file.is_open())
    {
        try
        {
            input_file >> preference_data;
        }
        catch (...)
        {
            json preference_data;
        }
    }
    archive_data["zip"]["comp_method"] = (ui->zip_comp_method->currentText()).toStdString();
    archive_data["zip"]["comp_level"] = (ui->zip_level_label->text()).toStdString();
    archive_data["seven_zip"]["comp_method"] = (ui->seven_zip_comp_method->currentText()).toStdString();
    archive_data["seven_zip"]["comp_level"] = (ui->seven_zip_level_label->text()).toStdString();
    archive_data["tar"]["comp_method"] = (ui->tar_comp_method->currentText()).toStdString();
    if (ui->tar_metadata_cb->checkState() == Qt::Unchecked) {archive_data["tar"]["pres_metadata"] = {false};}
    else {archive_data["tar"]["pres_metadata"] = {true};}
    archive_data["xar"]["comp_method"] = (ui->xar_comp_method->currentText()).toStdString();
    if (ui->xar_metadata_cb->checkState() == Qt::Unchecked) {archive_data["xar"]["pres_metadata"] = {false};}
    else {archive_data["tar"]["pres_metadata"] = {true};}
    archive_data["cpio"]["format"] = (ui->cpio_format->currentText()).toStdString();
    if (ui->cpio_metadata_cb->checkState() == Qt::Unchecked) {archive_data["cpio"]["pres_metadata"] = {false};}
    else {archive_data["cpio"]["pres_metadata"] = {true};}
    if (ui->iso_enforce_83_cb->checkState() == Qt::Unchecked) {archive_data["iso"]["enforce_83"] = {false};}
    else {archive_data["iso"]["enforce_83"] = {true};}
    if (ui->iso_uppercase_cb->checkState() == Qt::Unchecked) {archive_data["iso"]["uppercase"] = {false};}
    else {archive_data["iso"]["uppercase"] = {true};}
    if (ui->iso_rm_invalid_cb->checkState() == Qt::Unchecked) {archive_data["iso"]["rm_invalid"] = {false};}
    else {archive_data["iso"]["rm_invalid"] = {true};}
    archive_data["ar"]["format"] = (ui->ar_format->currentText()).toStdString();
    if (ui->ar_determ_mode_cb->checkState() == Qt::Unchecked) {archive_data["ar"]["determ_mode"] = {false};}
    else {archive_data["ar"]["determ_mode"] = {true};}
    preference_data["archive"] = archive_data;
    ofstream output_file(json_path.toStdString());
    if (output_file.is_open())
    {
        output_file << preference_data.dump(4);
        output_file.close();
        ui->save_preferences->setEnabled(false);
        ui->cancel_preferences->setEnabled(false);
        fetch_base_preferences();
    }
}


void ArchivePreferences::on_zip_comp_method_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_seven_zip_comp_method_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_tar_comp_method_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_xar_comp_method_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_cpio_format_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_ar_format_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_tar_metadata_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_xar_metadata_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_cpio_metadata_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}

void ArchivePreferences::on_ar_determ_mode_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_iso_uppercase_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_iso_enforce_83_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void ArchivePreferences::on_iso_rm_invalid_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}

void ArchivePreferences::on_cancel_preferences_clicked()
{
    this->close();
}


void ArchivePreferences::on_zip_comp_slider_sliderMoved(int position)
{
    switch (position)
    {
    case 0:
        ui->zip_level_label->setText("N/A");
        break;
    case 1:
        ui->zip_level_label->setText("Store");
        break;
    case 2:
        ui->zip_level_label->setText("Fast");
        break;
    case 3:
        ui->zip_level_label->setText("Normal");
        break;
    case 4:
        ui->zip_level_label->setText("Maximum");
    }
    check_boxes_states();
}


void ArchivePreferences::on_seven_zip_comp_slider_sliderMoved(int position)
{
    switch (position)
    {
    case 0:
        ui->seven_zip_level_label->setText("N/A");
        break;
    case 1:
        ui->seven_zip_level_label->setText("Store");
        break;
    case 2:
        ui->seven_zip_level_label->setText("Fast");
        break;
    case 3:
        ui->seven_zip_level_label->setText("Normal");
        break;
    case 4:
        ui->seven_zip_level_label->setText("Maximum");
    }
    check_boxes_states();
}
