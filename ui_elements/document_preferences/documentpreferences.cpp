#include "documentpreferences.h"
#include "ui_documentpreferences.h"
#include "json.hpp"
#include "fstream"
#include <QFileInfo>

DocumentPreferences::DocumentPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DocumentPreferences)
{
    ui->setupUi(this);
    load_document_preferences();
    set_tooltips();
    ui->save_preferences->setEnabled(false);
    ui->cancel_preferences->setEnabled(false);
}

DocumentPreferences::~DocumentPreferences()
{
    delete ui;
}

using json = nlohmann::json;
using namespace std;

bool initial_rm_metadata;
bool initial_pres_formatting;
bool initial_pres_media;
bool initial_line_break;
bool initial_standalone;

void DocumentPreferences::fetch_base_preferences()
{
    initial_rm_metadata = ui->rm_metadata_cb->isChecked();
    initial_pres_formatting = ui->preserve_formatting_cb->isChecked();
    initial_pres_media = ui->preserve_media_cb->isChecked();
    initial_line_break = ui->line_breaks_cb->isChecked();
    initial_standalone = ui->standalone_cb->isChecked();
}

void DocumentPreferences::set_tooltips()
{
    ui->rm_metadata_cb->setToolTip("Applies to: DOCX, EPUB, HTML, JSON, LATEX, ODT, RTF, RTS, ORG");
    ui->preserve_formatting_cb->setToolTip("Applies to: DOCX, EPUB, HTML, LATEX, ODT, RTF");
    ui->preserve_media_cb->setToolTip("Applies to: DOCX, EPUB, HTML, LATEX, ODT, RTF, RTS, ORG");
    ui->line_breaks_cb->setToolTip("Applies to: HTML, MD, LATEX, ODT, RTF, RTS, ORG");
    ui->standalone_cb->setToolTip("Applies to: EPUB, HTML, MD, LATEX, RTS, ORG");
}

void DocumentPreferences::load_document_preferences()
{
    QString app_dir = QCoreApplication::applicationDirPath();
    QString json_path = app_dir + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (save_json.is_open())
    {
        json load_data;
        save_json >> load_data;
        if (load_data.contains("document"))
        {
            auto document_preferences = load_data["document"];
            int rm_metadata = document_preferences["rm_metadata"];
            if (rm_metadata == 2) {ui->rm_metadata_cb->setCheckState(Qt::Checked);}
            int pres_formatting = document_preferences["pres_formatting"];
            if (pres_formatting == 2) {ui->preserve_formatting_cb->setCheckState(Qt::Checked);}
            int pres_media = document_preferences["pres_media"];
            if (pres_media == 2) {ui->preserve_media_cb->setCheckState(Qt::Checked);}
            int line_break = document_preferences["line_break"];
            if (line_break == 2) {ui->line_breaks_cb->setCheckState(Qt::Checked);}
            int standalone = document_preferences["standalone"];
            if (standalone == 2) {ui->standalone_cb->setCheckState(Qt::Checked);}
        }
    }
    fetch_base_preferences();
}

void DocumentPreferences::check_boxes_states()
{
    if (initial_rm_metadata == ui->rm_metadata_cb->isChecked() && initial_pres_formatting == ui->preserve_formatting_cb->isChecked() &&
        initial_pres_media == ui->preserve_media_cb->isChecked() && initial_line_break == ui->line_breaks_cb->isChecked() &&
        initial_standalone == ui->standalone_cb->isChecked())
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

void DocumentPreferences::on_save_preferences_clicked()
{
    QString app_dir = QCoreApplication::applicationDirPath();
    QString json_path = app_dir + "/conversion_preferences.json";
    json preference_data;
    json document_data;
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
    document_data["rm_metadata"] = (ui->rm_metadata_cb->checkState());
    document_data["pres_formatting"] = (ui->preserve_formatting_cb->checkState());
    document_data["pres_media"] = (ui->preserve_media_cb->checkState());
    document_data["line_break"] = (ui->line_breaks_cb->checkState());
    document_data["standalone"] = (ui->standalone_cb->checkState());
    preference_data["document"] = document_data;
    ofstream output_file(json_path.toStdString());
    if (output_file.is_open())
    {
        output_file << preference_data.dump(4);
        output_file.close();
        ui->save_preferences->setEnabled(false);
        ui->cancel_preferences->setEnabled(false);
        fetch_base_preferences();
    }
    check_boxes_states();
}

void DocumentPreferences::on_cancel_preferences_clicked()
{
    this->close();
}

void DocumentPreferences::on_rm_metadata_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void DocumentPreferences::on_preserve_formatting_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}

void DocumentPreferences::on_preserve_media_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void DocumentPreferences::on_line_breaks_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void DocumentPreferences::on_standalone_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}
