#include "spreadsheetpreferences.h"
#include "ui_spreadsheetpreferences.h"
#include <fstream>
#include <QFileInfo>
#include "json.hpp"

SpreadsheetPreferences::SpreadsheetPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SpreadsheetPreferences)
{
    ui->setupUi(this);
    load_spreadsheet_preferences();
    ui->save_preferences->setEnabled(false);
    ui->cancel_preferences->setEnabled(false);
    ui->delimiter->setToolTip("Applies to: CSV");
    ui->empty_rc_cb->setToolTip("Applies to: ODS, FODS");
    ui->styling_cb->setToolTip("Applies to: XLSX");
    ui->pretty_print_cb->setToolTip("Applies to: XML");
}

SpreadsheetPreferences::~SpreadsheetPreferences()
{
    delete ui;
}

using json = nlohmann::json;
using namespace std;

QString initial_delimiter;
Qt::CheckState initial_empty_rc;
Qt::CheckState initial_styling;
Qt::CheckState initial_pretty;

void SpreadsheetPreferences::fetch_base_preferences()
{
    initial_delimiter = ui->delimiter->currentText();
    initial_empty_rc = ui->empty_rc_cb->checkState();
    initial_styling = ui->styling_cb->checkState();
    initial_pretty = ui->pretty_print_cb->checkState();
}

void SpreadsheetPreferences::load_spreadsheet_preferences()
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
        if (load_data.contains("spreadsheet"))
        {
            auto spreadsheet_preferences = load_data["spreadsheet"];
            QString delimiter = QString::fromStdString(spreadsheet_preferences["delimiter"][1]);
            bool remove_empty_rc = spreadsheet_preferences["rm_empty_rc"][0];
            if (remove_empty_rc) {ui->empty_rc_cb->setCheckState(Qt::Checked);}
            bool keep_styling = spreadsheet_preferences["styling"][0];
            if (keep_styling) {ui->styling_cb->setCheckState(Qt::Checked);}
            bool pretty_printing = spreadsheet_preferences["pretty_print"][0];
            if (pretty_printing) {ui->pretty_print_cb->setCheckState(Qt::Checked);}
        }
    }
    fetch_base_preferences();
}

void SpreadsheetPreferences::check_boxes_states()
{
    if (initial_delimiter == ui->delimiter->currentText() && initial_empty_rc == ui->empty_rc_cb->checkState() &&
        initial_styling == ui->styling_cb->checkState() && initial_pretty == ui->pretty_print_cb->checkState())
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

void SpreadsheetPreferences::on_save_preferences_clicked()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    json preference_data;
    json spreadsheet_data;
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
    spreadsheet_data["delimiter"] = {true, ui->delimiter->currentText().toStdString()};
    if (ui->empty_rc_cb->checkState() == Qt::Unchecked) {spreadsheet_data["rm_empty_rc"] = {false};}
    else {spreadsheet_data["rm_empty_rc"] = {true};}
    if (ui->styling_cb->checkState() == Qt::Unchecked) {spreadsheet_data["styling"] = {false};}
    else {spreadsheet_data["styling"] = {true};}
    if (ui->pretty_print_cb->checkState() == Qt::Unchecked) {spreadsheet_data["pretty_print"] = {false};}
    else {spreadsheet_data["pretty_print"] = {true};}
    preference_data["spreadsheet"] = spreadsheet_data;
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

void SpreadsheetPreferences::on_cancel_preferences_clicked()
{
    this->close();
}

void SpreadsheetPreferences::on_delimiter_currentTextChanged(const QString &arg1)
{
    check_boxes_states();
}

void SpreadsheetPreferences::on_empty_rc_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void SpreadsheetPreferences::on_styling_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}


void SpreadsheetPreferences::on_pretty_print_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_boxes_states();
}
