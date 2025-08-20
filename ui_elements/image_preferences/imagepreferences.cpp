#include "imagepreferences.h"
#include "ui_imagepreferences.h"
#include <QIntValidator>
#include <QFileInfo>
#include "json.hpp"
#include <fstream>

ImagePreferences::ImagePreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImagePreferences)
{
    ui->setupUi(this);
    QIntValidator *validator = new QIntValidator(0, 100, this);
    ui->quality_range->setValidator(validator);
    load_image_preferences();
}

ImagePreferences::~ImagePreferences()
{
    delete ui;
}

using json = nlohmann::json;
using namespace std;

QString initial_aspect_ratio;
QPair<Qt::CheckState, QString> initial_quality;
Qt::CheckState initial_grayscale;
Qt::CheckState initial_alpha;
QPair<Qt::CheckState, QString> initial_bit_depth;

void ImagePreferences::fetch_initial_cb_states(QString &aspect_ratio, QPair<Qt::CheckState, QString> &quality, Qt::CheckState &grayscale, Qt::CheckState &alpha, QPair<Qt::CheckState, QString> &bit_depth)
{
    aspect_ratio = ui->aspect_ratio->currentText();
    quality.first = ui->quality_cb->checkState();
    quality.second = ui->quality_range->text();
    grayscale = ui->gray_scale_cb->checkState();
    alpha = ui->remove_alpha_cb->checkState();
    bit_depth.first = ui->bit_depth_cb->checkState();
    bit_depth.second = ui->bit_depth->currentText();
}

void ImagePreferences::load_image_preferences()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    ifstream save_json(json_path.toStdString());
    if (!save_json.is_open())
    {
        fetch_initial_cb_states(initial_aspect_ratio, initial_quality, initial_grayscale, initial_alpha, initial_bit_depth);
    }
    else
    {
        json load_data;
        save_json >> load_data;
        save_json.close();
        if (load_data.contains("image"))
        {
            auto image_preference = load_data["image"];
            QString aspect_ratio = QString::fromStdString(image_preference["aspect_ratio"][1]);
            int ar_selection = ui->aspect_ratio->findText(aspect_ratio, Qt::MatchFixedString);
            if (ar_selection >= 0) {ui->aspect_ratio->setCurrentIndex(ar_selection);}
            if (image_preference["quality"][0])
            {
                ui->quality_cb->setCheckState(Qt::Checked);
                QString quality = QString::fromStdString(image_preference["quality"][1]);
                ui->quality_range->setText(quality);
            }
            else
            {
                ui->quality_cb->setCheckState(Qt::Unchecked);
            }
            bool grayscale = image_preference["grayscale"][0];
            if (grayscale) {ui->gray_scale_cb->setCheckState(Qt::Checked);}
            bool remove_alpha = image_preference["alpha"][0];
            if (remove_alpha) {ui->remove_alpha_cb->setCheckState(Qt::Checked);}
            if (image_preference["bitdepth"][0])
            {
                ui->bit_depth_cb->setCheckState(Qt::Checked);
                QString bitdepth = QString::fromStdString(image_preference["bitdepth"][1]);
                int bd_selection = ui->bit_depth->findText(bitdepth, Qt::MatchFixedString);
                if (bd_selection >= 0) {ui->bit_depth->setCurrentIndex(bd_selection);}
            }
        }
    }
    ui->save_image_preferences->setEnabled(false);
    ui->cancel_image_preferences->setEnabled(false);
    if (ui->quality_cb->checkState() == Qt::Unchecked) {ui->quality_range->setReadOnly(true);}
    if (ui->bit_depth_cb->checkState() == Qt::Unchecked) {ui->bit_depth->setEnabled(false);}
}

void ImagePreferences::check_checkbox_states()
{
    if (initial_aspect_ratio == ui->aspect_ratio->currentText() && initial_quality.first == ui->quality_cb->checkState() && initial_quality.second == ui->quality_range->text() &&
        initial_grayscale == ui->gray_scale_cb->checkState() && initial_alpha == ui->remove_alpha_cb->checkState() && initial_bit_depth.first == ui->bit_depth_cb->checkState() &&
        initial_bit_depth.second == ui->bit_depth->currentText())
    {
        ui->save_image_preferences->setEnabled(false);
        ui->cancel_image_preferences->setEnabled(false);
    }
    else
    {
        ui->save_image_preferences->setEnabled(true);
        ui->cancel_image_preferences->setEnabled(true);
    }
}

void ImagePreferences::on_aspect_ratio_currentTextChanged(const QString &arg1)
{
    check_checkbox_states();
}

void ImagePreferences::on_quality_range_textChanged(const QString &numbers)
{
    check_checkbox_states();
}

void ImagePreferences::on_quality_cb_stateChanged(int state)
{
    check_checkbox_states();
}

void ImagePreferences::on_save_image_preferences_clicked()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/conversion_preferences.json";
    json image_data;
    if (ui->aspect_ratio->currentText() == "Keep Aspect Ratio") {image_data["aspect_ratio"] = {false, "None"};}
    else {image_data["aspect_ratio"] = {true, ui->aspect_ratio->currentText().toStdString()};}
    if (ui->quality_cb->checkState() == Qt::Unchecked) {image_data["quality"] = {false, "None"};}
    else {image_data["quality"] = {true, ui->quality_range->text().toStdString()};}
    if (ui->gray_scale_cb->checkState() == Qt::Unchecked) {image_data["grayscale"] = {false};}
    else {image_data["grayscale"] = {true};}
    if (ui->remove_alpha_cb->checkState() == Qt::Unchecked) {image_data["alpha"] = {false};}
    else {image_data["alpha"] = {true};}
    if (ui->bit_depth_cb->checkState() == Qt::Unchecked) {image_data["bitdepth"] = {false, "None"};}
    else {image_data["bitdepth"] = {true, ui->bit_depth->currentText().toStdString()};}
    json preference_data;
    preference_data["image"] = image_data;
    ofstream file(json_path.toStdString());
    if (file.is_open())
    {
        file << preference_data.dump(4);
        file.close();
        ui->save_image_preferences->setEnabled(false);
        ui->cancel_image_preferences->setEnabled(false);
    }
}

void ImagePreferences::on_cancel_image_preferences_clicked()
{
    this->close();
}

void ImagePreferences::on_quality_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_checkbox_states();
    if (arg1 == Qt::Unchecked) {ui->quality_range->setReadOnly(true);}
    else {ui->quality_range->setReadOnly(false);}
}

void ImagePreferences::on_quality_range_textEdited(const QString &arg1)
{
    ui->quality_range->blockSignals(true);
    QString clean = arg1;
    clean.remove('%');
    clean = clean.trimmed();
    if (clean.isEmpty())
    {
        ui->quality_range->setText("");
        ui->quality_range->blockSignals(false);
        return;
    }
    ui->quality_range->setText(clean + "%");
    int cursorPos = ui->quality_range->text().size() - 1;
    ui->quality_range->setCursorPosition(cursorPos);
    ui->quality_range->blockSignals(false);
}

void ImagePreferences::on_gray_scale_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_checkbox_states();
}

void ImagePreferences::on_remove_alpha_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_checkbox_states();
}

void ImagePreferences::on_bit_depth_cb_checkStateChanged(const Qt::CheckState &arg1)
{
    check_checkbox_states();
    if (arg1 == Qt::Unchecked) {ui->bit_depth->setEnabled(false);}
    else {ui->bit_depth->setEnabled(true);}
}

void ImagePreferences::on_bit_depth_currentTextChanged(const QString &arg1)
{
    check_checkbox_states();
}

