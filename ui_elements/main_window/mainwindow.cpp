#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QString>
#include <QAction>
#include <QApplication>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QTimer>
#include <QUrl>
#include <QDesktopServices>
#include <filesystem>
#include <fstream>
#include "changesavelocation.h"
#include "imagepreferences.h"
#include "videoaudiopreferences.h"
#include "documentpreferences.h"
#include "spreadsheetpreferences.h"
#include "archivepreferences.h"
#include <bulkconvertmanager.h>
#include "json.hpp"
#include <archive.h>
#include <archive_entry.h>

using json = nlohmann::json;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->result_box->setReadOnly(true);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit_program);
    connect(ui->actionOpen_Save_Folder, &QAction::triggered, this, &MainWindow::open_save_location);
    connect(ui->actionChange_Save_Folder, &QAction::triggered, this, &MainWindow::change_save_folder);
    connect(ui->actionImage_Preferences, &QAction::triggered, this, &MainWindow::change_image_preferences);
    connect(ui->actionVideo_Audio_Preferences, &QAction::triggered, this, &MainWindow::change_videoaudio_preferences);
    connect(ui->actionDocument_Preferences, &QAction::triggered, this, &MainWindow::change_document_preferences);
    connect(ui->actionSpreadsheet_Preferenecs, &QAction::triggered, this, &MainWindow::change_spreadsheet_preferences);
    connect(ui->actionArchives_Preferences, &QAction::triggered, this, &MainWindow::change_archive_preferences);
    ui->pause_conversion->setEnabled(false);
    ui->continue_conversion->setEnabled(false);
    check_save_location();
    setup_progress_bars();
    auto *model = ui->drag_n_drop_area->model();
    connect(model, &QAbstractItemModel::rowsInserted, this, &MainWindow::dnd_label_visibility);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &MainWindow::dnd_label_visibility);
}

MainWindow::~MainWindow()
{
    if (bcm) {
        bcm->disconnect();
        delete bcm;
        bcm = nullptr;
    }
    delete ui;
}

void MainWindow::change_save_folder()
{
    ChangeSaveLocation change_save_location;
    change_save_location.setModal(true);
    change_save_location.exec();
}

void MainWindow::change_image_preferences()
{
    ImagePreferences image_preferences;
    image_preferences.setModal(true);
    image_preferences.exec();
}

void MainWindow::change_videoaudio_preferences()
{
    VideoAudioPreferences videoaudio_preferences;
    videoaudio_preferences.setModal(true);
    videoaudio_preferences.exec();
}

void MainWindow::change_document_preferences()
{
    DocumentPreferences document_preferences;
    document_preferences.setModal(true);
    document_preferences.exec();
}

void MainWindow::change_spreadsheet_preferences()
{
    SpreadsheetPreferences spreadsheet_preferences;
    spreadsheet_preferences.setModal(true);
    spreadsheet_preferences.exec();
}

void MainWindow::change_archive_preferences()
{
    ArchivePreferences archive_preferences;
    archive_preferences.setModal(true);
    archive_preferences.exec();
}

void MainWindow::setup_progress_bars()
{
    ui->image_progress->setRange(0, 100);
    ui->image_progress->setValue(0);
    ui->av_progress->setRange(0, 100);
    ui->av_progress->setValue(0);
    ui->doc_progress->setValue(0);
    ui->archives_progress->setValue(0);
    ui->spread_progress->setValue(0);
}

void MainWindow::dnd_label_visibility()
{
    const bool has_files = ui->drag_n_drop_area->count() > 0;
    ui->drag_n_drop_text->setVisible(!has_files);
}

void MainWindow::check_save_location()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/save_location.json";
    if (filesystem::exists(json_path.toStdString()) && filesystem::is_regular_file(json_path.toStdString())) {}
    else
    {
        json save_data;
        save_data["location"] = cpp_directory.toStdString();
        ofstream file(json_path.toStdString());
        if (file.is_open())
        {
            file << save_data.dump(4);
            file.close();
        }
    }
}

QString load_save_location()
{
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString cpp_directory = file_info.absolutePath();
    QString json_path = cpp_directory + "/save_location.json";
    ifstream save_json(json_path.toStdString());
    json read_data;
    save_json >> read_data;
    QString save_folder = QString::fromStdString(read_data["location"].get<string>());
    return save_folder;
}

void MainWindow::exit_program()
{
    QApplication::quit();
}

void MainWindow::open_save_location()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(load_save_location()));
}

void MainWindow::convert_user_image(QString save_folder)
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    ui->image_progress->setValue(0);
    ui->result_box->setReadOnly(false);
    ui->result_box->clear();
    ui->result_box->setReadOnly(true);
    ui->select_file_images->setEnabled(false);
    ui->convert_button_image->setEnabled(false);
    ui->convert_button_image_save->setEnabled(false);
    for (int i = 0; i < og_file_paths.size(); ++i)
    {
        QString clean_path = og_file_paths[i];
        int last_space_index = clean_path.lastIndexOf(' ');
        if (last_space_index != -1)
        {
            QString suffix = clean_path.mid(last_space_index + 1);
            if (suffix == QString::fromUtf8("⌛") ||
                suffix == QString::fromUtf8("🔄️") ||
                suffix == QString::fromUtf8("⏭️️") ||
                suffix == QString::fromUtf8("❌") ||
                suffix == QString::fromUtf8("✅"))
            {
                clean_path = clean_path.left(last_space_index);
            }
        }
        ui->drag_n_drop_area->update_file_path(i, clean_path);
    }
    bcm = new BulkConvertManager(this);
    bcm->set_file_type(FileType::Image);
    if (save_folder == "Alternate")
    {
        QString output_path = QFileDialog::getExistingDirectory(NULL, "Select Folder");
        if (output_path.isEmpty())
        {
            ui->image_progress->setValue(0);
            ui->result_box->setReadOnly(false);
            ui->result_box->setText("No location selected.");
            ui->result_box->setReadOnly(true);
            QTimer::singleShot(5000, this, [this]() {
                ui->result_box->setReadOnly(false);
                ui->result_box->clear();
                ui->result_box->setReadOnly(true);
            });
            ui->select_file_images->setEnabled(true);
            ui->convert_button_image->setEnabled(true);
            ui->convert_button_image_save->setEnabled(true);
            if (bcm)
            {
                disconnect(bcm, nullptr, this, nullptr);
                bcm->deleteLater();
                bcm = nullptr;
            }
            return;
        }
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_image->currentText(), output_path);
    }
    else
    {
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_image->currentText(), save_folder);
    }
    og_file_paths = ui->drag_n_drop_area->get_files();
    connect(bcm, &BulkConvertManager::progress_updated, this, [this](int finished, int total)
    {
        if (total > 0) {
            int percent = (finished * 100) / total;
            ui->image_progress->setValue(percent);
        }
    });
    connect(bcm, &BulkConvertManager::job_status_updated, this, [this](int job_index, ConversionStatus status)
    {
        QString file_path = og_file_paths[job_index];
        switch (status)
        {
            case ConversionStatus::Waiting:
                file_path = file_path + QString::fromUtf8(" ⌛");
                break;
            case ConversionStatus::Converting:
                file_path = file_path + QString::fromUtf8(" 🔄️");
                break;
            case ConversionStatus::Skipped:
                file_path = file_path + QString::fromUtf8(" ⏭️️");
                break;
            case ConversionStatus::Failed:
                file_path = file_path + QString::fromUtf8(" ❌");
                break;
            case ConversionStatus::Complete:
                file_path = file_path + QString::fromUtf8(" ✅");
                break;
        }
        ui->drag_n_drop_area->update_file_path(job_index, file_path);
    });
    connect(bcm, &BulkConvertManager::finished, this, [this]()
    {
        ui->image_progress->setValue(100);
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        if (bcm->total_success())
        {
            ui->result_box->setText("Image conversion finished.");
        }
        else
        {
            if (bcm->log_made())
            {
                ui->result_box->setText(QString("%1 of %2 image conversions succeeded (error log available).").arg(bcm->succeeded).arg(bcm->job_count));
            }
            else
            {
                ui->result_box->setText(QString("%1 of %2 image conversions succeeded.").arg(bcm->succeeded).arg(bcm->job_count));
            }
        }
        ui->result_box->setReadOnly(true);
        QTimer::singleShot(5000, this, [this]() {
            ui->result_box->setReadOnly(false);
            ui->result_box->clear();
            ui->result_box->setReadOnly(true);
        });
        ui->select_file_images->setEnabled(true);
        ui->convert_button_image->setEnabled(true);
        ui->convert_button_image_save->setEnabled(true);
        ui->pause_conversion->setEnabled(false);
        ui->continue_conversion->setEnabled(false);
        if (bcm)
        {
            disconnect(bcm, nullptr, this, nullptr);
            bcm->deleteLater();
            bcm = nullptr;
        }
    });
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->start();
}

// Converts the user's provided image file(s)
void MainWindow::on_convert_button_image_clicked()
{
    convert_user_image(load_save_location());
}

void MainWindow::on_convert_button_image_save_clicked()
{
    convert_user_image("Alternate");
}

void MainWindow::on_input_type_image_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_image->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_image->setEnabled(false);
        ui->convert_button_image_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_image->setEnabled(true);
        ui->convert_button_image_save->setEnabled(true);
    }
}

void MainWindow::on_output_type_image_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_image->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_image->setEnabled(false);
        ui->convert_button_image_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_image->setEnabled(true);
        ui->convert_button_image_save->setEnabled(true);
    }
}

// Converts the user's provided video/audio file(s)
void MainWindow::convert_user_av(QString save_folder)
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    ui->av_progress->setValue(0);
    ui->result_box->setReadOnly(false);
    ui->result_box->clear();
    ui->result_box->setReadOnly(true);
    ui->select_file_av->setEnabled(false);
    ui->convert_button_av->setEnabled(false);
    ui->convert_button_av_save->setEnabled(false);
    for (int i = 0; i < og_file_paths.size(); ++i)
    {
        QString clean_path = og_file_paths[i];
        int last_space_index = clean_path.lastIndexOf(' ');
        if (last_space_index != -1)
        {
            QString suffix = clean_path.mid(last_space_index + 1);
            if (suffix == QString::fromUtf8("⌛") ||
                suffix == QString::fromUtf8("🔄️") ||
                suffix == QString::fromUtf8("⏭️️") ||
                suffix == QString::fromUtf8("❌") ||
                suffix == QString::fromUtf8("✅"))
            {
                clean_path = clean_path.left(last_space_index);
            }
        }
        ui->drag_n_drop_area->update_file_path(i, clean_path);
    }
    bcm = new BulkConvertManager(this);
    bcm->set_file_type(FileType::AV);
    if (save_folder == "Alternate")
    {
        QString output_path = QFileDialog::getExistingDirectory(NULL, "Select Folder");
        if (output_path.isEmpty())
        {
            ui->av_progress->setValue(0);
            ui->result_box->setReadOnly(false);
            ui->result_box->setText("No location selected.");
            ui->result_box->setReadOnly(true);
            QTimer::singleShot(5000, this, [this]() {
                ui->result_box->setReadOnly(false);
                ui->result_box->clear();
                ui->result_box->setReadOnly(true);
            });
            if (bcm)
            {
                disconnect(bcm, nullptr, this, nullptr);
                bcm->deleteLater();
                bcm = nullptr;
            }
            return;
        }
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_av->currentText(), output_path);
    }
    else
    {
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_av->currentText(), save_folder);
    }
    og_file_paths = ui->drag_n_drop_area->get_files();
    connect(bcm, &BulkConvertManager::progress_updated, this, [this](int finished, int total)
    {
        if (total > 0) {
            int percent = (finished * 100) / total;
            ui->av_progress->setValue(percent);
        }
    });
    connect(bcm, &BulkConvertManager::job_status_updated, this, [this](int job_index, ConversionStatus status)
    {
        QString file_path = og_file_paths[job_index];
        switch (status)
        {
        case ConversionStatus::Waiting:
            file_path = file_path + QString::fromUtf8(" ⌛");
            break;
        case ConversionStatus::Converting:
            file_path = file_path + QString::fromUtf8(" 🔄️");
            break;
        case ConversionStatus::Skipped:
            file_path = file_path + QString::fromUtf8(" ⏭️️");
            break;
        case ConversionStatus::Failed:
            file_path = file_path + QString::fromUtf8(" ❌");
            break;
        case ConversionStatus::Complete:
            file_path = file_path + QString::fromUtf8(" ✅");
            break;
        }
        ui->drag_n_drop_area->update_file_path(job_index, file_path);
    });
    connect(bcm, &BulkConvertManager::finished, this, [this]()
    {
        ui->av_progress->setValue(100);
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        if (bcm->total_success())
        {
            ui->result_box->setText("Video/Audio conversion finished.");
        }
        else
        {
            if (bcm->log_made())
            {
                ui->result_box->setText(QString("%1 of %2 video/audio conversions succeeded (error log available).").arg(bcm->succeeded).arg(bcm->job_count));
            }
            else
            {
                ui->result_box->setText(QString("%1 of %2 video/audio conversions succeeded.").arg(bcm->succeeded).arg(bcm->job_count));
            }
        }
        ui->result_box->setReadOnly(true);
        QTimer::singleShot(5000, this, [this]() {
            ui->result_box->setReadOnly(false);
            ui->result_box->clear();
            ui->result_box->setReadOnly(true);
        });
        ui->select_file_av->setEnabled(true);
        ui->convert_button_av->setEnabled(true);
        ui->convert_button_av_save->setEnabled(true);
        ui->pause_conversion->setEnabled(false);
        ui->continue_conversion->setEnabled(false);
        if (bcm)
        {
            disconnect(bcm, nullptr, this, nullptr);
            bcm->deleteLater();
            bcm = nullptr;
        }
    });
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->start();
}

void MainWindow::on_convert_button_av_clicked()
{
    convert_user_av(load_save_location());
}

void MainWindow::on_convert_button_av_save_clicked()
{
    convert_user_av("Alternate");
}

void MainWindow::on_input_type_av_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_av->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_av->setEnabled(false);
        ui->convert_button_av_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_av->setEnabled(true);
        ui->convert_button_av_save->setEnabled(true);
    }
}

void MainWindow::on_output_type_av_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_av->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_av->setEnabled(false);
        ui->convert_button_av_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_av->setEnabled(true);
        ui->convert_button_av_save->setEnabled(true);
    }
}

// Converts the user's provided document file(s)
void MainWindow::convert_user_document(QString save_folder)
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    ui->doc_progress->setValue(0);
    ui->result_box->setReadOnly(false);
    ui->result_box->clear();
    ui->result_box->setReadOnly(true);
    ui->select_file_doc->setEnabled(false);
    ui->convert_button_doc->setEnabled(false);
    ui->convert_button_doc_save->setEnabled(false);
    for (int i = 0; i < og_file_paths.size(); ++i)
    {
        QString clean_path = og_file_paths[i];
        int last_space_index = clean_path.lastIndexOf(' ');
        if (last_space_index != -1)
        {
            QString suffix = clean_path.mid(last_space_index + 1);
            if (suffix == QString::fromUtf8("⌛") ||
                suffix == QString::fromUtf8("🔄️") ||
                suffix == QString::fromUtf8("⏭️️") ||
                suffix == QString::fromUtf8("❌") ||
                suffix == QString::fromUtf8("✅"))
            {
                clean_path = clean_path.left(last_space_index);
            }
        }
        ui->drag_n_drop_area->update_file_path(i, clean_path);
    }
    bcm = new BulkConvertManager(this);
    bcm->set_file_type(FileType::Doc);
    if (save_folder == "Alternate")
    {
        QString output_path = QFileDialog::getExistingDirectory(NULL, "Select Folder");
        if (output_path.isEmpty())
        {
            ui->doc_progress->setValue(0);
            ui->result_box->setReadOnly(false);
            ui->result_box->setText("No location selected.");
            ui->result_box->setReadOnly(true);
            QTimer::singleShot(5000, this, [this]() {
                ui->result_box->setReadOnly(false);
                ui->result_box->clear();
                ui->result_box->setReadOnly(true);
            });
            if (bcm)
            {
                disconnect(bcm, nullptr, this, nullptr);
                bcm->deleteLater();
                bcm = nullptr;
            }
            return;
        }
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_doc->currentText(), output_path);
    }
    else
    {
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_doc->currentText(), save_folder);
    }
    og_file_paths = ui->drag_n_drop_area->get_files();
    connect(bcm, &BulkConvertManager::progress_updated, this, [this](int finished, int total)
    {
        if (total > 0) {
            int percent = (finished * 100) / total;
            ui->doc_progress->setValue(percent);
        }
    });
    connect(bcm, &BulkConvertManager::job_status_updated, this, [this](int job_index, ConversionStatus status)
    {
        QString file_path = og_file_paths[job_index];
        switch (status)
        {
        case ConversionStatus::Waiting:
            file_path = file_path + QString::fromUtf8(" ⌛");
            break;
        case ConversionStatus::Converting:
            file_path = file_path + QString::fromUtf8(" 🔄️");
            break;
        case ConversionStatus::Skipped:
            file_path = file_path + QString::fromUtf8(" ⏭️️");
            break;
        case ConversionStatus::Failed:
            file_path = file_path + QString::fromUtf8(" ❌");
            break;
        case ConversionStatus::Complete:
            file_path = file_path + QString::fromUtf8(" ✅");
            break;
        }
        ui->drag_n_drop_area->update_file_path(job_index, file_path);
    });
    connect(bcm, &BulkConvertManager::finished, this, [this]()
    {
        ui->doc_progress->setValue(100);
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        if (bcm->total_success())
        {
            ui->result_box->setText("Document conversion finished.");
        }
        else
        {
            if (bcm->log_made())
            {
                ui->result_box->setText(QString("%1 of %2 document conversions succeeded (error log available).").arg(bcm->succeeded).arg(bcm->job_count));
            }
            else
            {
                ui->result_box->setText(QString("%1 of %2 document conversions succeeded.").arg(bcm->succeeded).arg(bcm->job_count));
            }
        }
        ui->result_box->setReadOnly(true);
        QTimer::singleShot(5000, this, [this]() {
            ui->result_box->setReadOnly(false);
            ui->result_box->clear();
            ui->result_box->setReadOnly(true);
        });
        ui->select_file_doc->setEnabled(true);
        ui->convert_button_doc->setEnabled(true);
        ui->convert_button_doc_save->setEnabled(true);
        ui->pause_conversion->setEnabled(false);
        ui->continue_conversion->setEnabled(false);
        if (bcm)
        {
            disconnect(bcm, nullptr, this, nullptr);
            bcm->deleteLater();
            bcm = nullptr;
        }
    });
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->start();
}

void MainWindow::on_convert_button_doc_clicked()
{
    convert_user_document(load_save_location());
}

void MainWindow::on_convert_button_doc_save_clicked()
{
    convert_user_document("Alternate");
}


void MainWindow::on_input_type_doc_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_doc->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_doc->setEnabled(false);
        ui->convert_button_doc_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_doc->setEnabled(true);
        ui->convert_button_doc_save->setEnabled(true);
    }
}

void MainWindow::on_output_type_doc_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_doc->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_doc->setEnabled(false);
        ui->convert_button_doc_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_doc->setEnabled(true);
        ui->convert_button_doc_save->setEnabled(true);
    }
}

// Converts the user's provided spreadsheet file(s)
void MainWindow::convert_user_spreadsheet(QString save_folder)
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    bcm = new BulkConvertManager(this);
    ui->spread_progress->setValue(0);
    ui->result_box->setReadOnly(false);
    ui->result_box->clear();
    ui->result_box->setReadOnly(true);
    ui->select_file_ss->setEnabled(false);
    ui->convert_button_spread->setEnabled(false);
    ui->convert_button_spread_save->setEnabled(false);
    for (int i = 0; i < og_file_paths.size(); ++i)
    {
        QString clean_path = og_file_paths[i];
        int last_space_index = clean_path.lastIndexOf(' ');
        if (last_space_index != -1)
        {
            QString suffix = clean_path.mid(last_space_index + 1);
            if (suffix == QString::fromUtf8("⌛") ||
                suffix == QString::fromUtf8("🔄️") ||
                suffix == QString::fromUtf8("⏭️️") ||
                suffix == QString::fromUtf8("❌") ||
                suffix == QString::fromUtf8("✅"))
            {
                clean_path = clean_path.left(last_space_index);
            }
        }
        ui->drag_n_drop_area->update_file_path(i, clean_path);
    }
    bcm = new BulkConvertManager(this);
    bcm->set_file_type(FileType::SS);
    if (save_folder == "Alternate")
    {
        QString output_path = QFileDialog::getExistingDirectory(NULL, "Select Folder");
        if (output_path.isEmpty())
        {
            ui->spread_progress->setValue(0);
            ui->result_box->setReadOnly(false);
            ui->result_box->setText("No location selected.");
            ui->result_box->setReadOnly(true);
            QTimer::singleShot(5000, this, [this]() {
                ui->result_box->setReadOnly(false);
                ui->result_box->clear();
                ui->result_box->setReadOnly(true);
            });
            if (bcm)
            {
                disconnect(bcm, nullptr, this, nullptr);
                bcm->deleteLater();
                bcm = nullptr;
            }
            return;
        }
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_spread->currentText(), output_path);
    }
    else
    {
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_spread->currentText(), save_folder);
    }
    og_file_paths = ui->drag_n_drop_area->get_files();
    connect(bcm, &BulkConvertManager::progress_updated, this, [this](int finished, int total)
    {
        if (total > 0) {
            int percent = (finished * 100) / total;
            ui->spread_progress->setValue(percent);
        }
    });
    connect(bcm, &BulkConvertManager::job_status_updated, this, [this](int job_index, ConversionStatus status)
    {
        QString file_path = og_file_paths[job_index];
        switch (status)
        {
        case ConversionStatus::Waiting:
            file_path = file_path + QString::fromUtf8(" ⌛");
            break;
        case ConversionStatus::Converting:
            file_path = file_path + QString::fromUtf8(" 🔄️");
            break;
        case ConversionStatus::Skipped:
            file_path = file_path + QString::fromUtf8(" ⏭️️");
            break;
        case ConversionStatus::Failed:
            file_path = file_path + QString::fromUtf8(" ❌");
            break;
        case ConversionStatus::Complete:
            file_path = file_path + QString::fromUtf8(" ✅");
            break;
        }
        ui->drag_n_drop_area->update_file_path(job_index, file_path);
    });
    connect(bcm, &BulkConvertManager::finished, this, [this]()
    {
        ui->spread_progress->setValue(100);
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        if (bcm->total_success())
        {
            ui->result_box->setText("Spreadsheet conversion finished.");
        }
        else
        {
            if (bcm->log_made())
            {
                ui->result_box->setText(QString("%1 of %2 spreadsheet conversions succeeded (error log available).").arg(bcm->succeeded).arg(bcm->job_count));
            }
            else
            {
                ui->result_box->setText(QString("%1 of %2 spreadsheet conversions succeeded.").arg(bcm->succeeded).arg(bcm->job_count));
            }
        }
        ui->result_box->setReadOnly(true);
        QTimer::singleShot(5000, this, [this]() {
            ui->result_box->setReadOnly(false);
            ui->result_box->clear();
            ui->result_box->setReadOnly(true);
        });
        ui->select_file_ss->setEnabled(true);
        ui->convert_button_spread->setEnabled(true);
        ui->convert_button_spread_save->setEnabled(true);
        ui->pause_conversion->setEnabled(false);
        ui->continue_conversion->setEnabled(false);
        if (bcm)
        {
            disconnect(bcm, nullptr, this, nullptr);
            bcm->deleteLater();
            bcm = nullptr;
        }
    });
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->start();
}

void MainWindow::on_convert_button_spread_clicked()
{
    convert_user_spreadsheet(load_save_location());
}

void MainWindow::on_convert_button_spread_save_clicked()
{
    convert_user_spreadsheet("Alternate");
}

void MainWindow::on_input_type_spread_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_spread->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_spread->setEnabled(false);
        ui->convert_button_spread_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_spread->setEnabled(true);
        ui->convert_button_spread_save->setEnabled(true);
    }
}

void MainWindow::on_output_type_spread_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_spread->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_spread->setEnabled(false);
        ui->convert_button_spread_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_spread->setEnabled(true);
        ui->convert_button_spread_save->setEnabled(true);
    }
}

// Converts the user's provided archive file(s)
void MainWindow::convert_user_archive(QString save_folder)
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    ui->archives_progress->setValue(0);
    ui->result_box->setReadOnly(false);
    ui->result_box->clear();
    ui->result_box->setReadOnly(true);
    ui->select_file_ar->setEnabled(false);
    ui->convert_button_archive->setEnabled(false);
    ui->convert_button_archive_save->setEnabled(false);
    for (int i = 0; i < og_file_paths.size(); ++i)
    {
        QString clean_path = og_file_paths[i];
        int last_space_index = clean_path.lastIndexOf(' ');
        if (last_space_index != -1)
        {
            QString suffix = clean_path.mid(last_space_index + 1);
            if (suffix == QString::fromUtf8("⌛") ||
                suffix == QString::fromUtf8("🔄️") ||
                suffix == QString::fromUtf8("⏭️️") ||
                suffix == QString::fromUtf8("❌") ||
                suffix == QString::fromUtf8("✅"))
            {
                clean_path = clean_path.left(last_space_index);
            }
        }
        ui->drag_n_drop_area->update_file_path(i, clean_path);
    }
    bcm = new BulkConvertManager(this);
    bcm->set_file_type(FileType::Archive);
    if (save_folder == "Alternate")
    {
        QString output_path = QFileDialog::getExistingDirectory(NULL, "Select Folder");
        if (output_path.isEmpty())
        {
            ui->archives_progress->setValue(0);
            ui->result_box->setReadOnly(false);
            ui->result_box->setText("No location selected.");
            ui->result_box->setReadOnly(true);
            QTimer::singleShot(5000, this, [this]() {
                ui->result_box->setReadOnly(false);
                ui->result_box->clear();
                ui->result_box->setReadOnly(true);
            });
            if (bcm)
            {
                disconnect(bcm, nullptr, this, nullptr);
                bcm->deleteLater();
                bcm = nullptr;
            }
            return;
        }
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_archive->currentText(), output_path);
    }
    else
    {
        bcm->set_jobs(ui->drag_n_drop_area->get_files(), ui->output_type_archive->currentText(), save_folder);
    }
    og_file_paths = ui->drag_n_drop_area->get_files();
    connect(bcm, &BulkConvertManager::progress_updated, this, [this](int finished, int total)
    {
        if (total > 0) {
            int percent = (finished * 100) / total;
            ui->archives_progress->setValue(percent);
        }
    });
    connect(bcm, &BulkConvertManager::job_status_updated, this, [this](int job_index, ConversionStatus status)
    {
        QString file_path = og_file_paths[job_index];
        switch (status)
        {
        case ConversionStatus::Waiting:
            file_path = file_path + QString::fromUtf8(" ⌛");
            break;
        case ConversionStatus::Converting:
            file_path = file_path + QString::fromUtf8(" 🔄️");
            break;
        case ConversionStatus::Skipped:
            file_path = file_path + QString::fromUtf8(" ⏭️️");
            break;
        case ConversionStatus::Failed:
            file_path = file_path + QString::fromUtf8(" ❌");
            break;
        case ConversionStatus::Complete:
            file_path = file_path + QString::fromUtf8(" ✅");
            break;
        }
        ui->drag_n_drop_area->update_file_path(job_index, file_path);
    });
    connect(bcm, &BulkConvertManager::finished, this, [this]()
    {
        ui->archives_progress->setValue(100);
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        if (bcm->total_success())
        {
            ui->result_box->setText("Archive conversion finished.");
        }
        else
        {
            if (bcm->log_made())
            {
                ui->result_box->setText(QString("%1 of %2 archive conversions succeeded (error log available).").arg(bcm->succeeded).arg(bcm->job_count));
            }
            else
            {
                ui->result_box->setText(QString("%1 of %2 archive conversions succeeded.").arg(bcm->succeeded).arg(bcm->job_count));
            }
        }
        ui->result_box->setReadOnly(true);
        QTimer::singleShot(5000, this, [this]() {
            ui->result_box->setReadOnly(false);
            ui->result_box->clear();
            ui->result_box->setReadOnly(true);
        });
        ui->select_file_ar->setEnabled(true);
        ui->convert_button_archive->setEnabled(true);
        ui->convert_button_archive_save->setEnabled(true);
        ui->pause_conversion->setEnabled(false);
        ui->continue_conversion->setEnabled(false);
        if (bcm)
        {
            disconnect(bcm, nullptr, this, nullptr);
            bcm->deleteLater();
            bcm = nullptr;
        }
    });
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->start();
}

void MainWindow::on_convert_button_archive_clicked()
{
    convert_user_archive(load_save_location());
}


void MainWindow::on_convert_button_archive_save_clicked()
{
    convert_user_archive("Alternate");
}


void MainWindow::on_input_type_archive_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_archive->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_archive->setEnabled(false);
        ui->convert_button_archive_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_archive->setEnabled(true);
        ui->convert_button_archive_save->setEnabled(true);
    }
}

void MainWindow::on_output_type_archive_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_archive->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_archive->setEnabled(false);
        ui->convert_button_archive_save->setEnabled(false);
    }
    else
    {
        ui->convert_button_archive->setEnabled(true);
        ui->convert_button_archive_save->setEnabled(true);
    }
}

void MainWindow::on_select_file_images_clicked()
{
    QString input_ext = ui->input_type_image->currentText();
    QString output_ext = ui->output_type_image->currentText();
    QString input_info = input_ext + " Files " + "(*." + input_ext.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_area->addItem(file_path);
    if (!file_path.isEmpty())
    {
        ui->drag_n_drop_area->clearFiles();
        ui->drag_n_drop_area->addFile(file_path);
    }
}

void MainWindow::on_select_file_av_clicked()
{
    QString input_ext = ui->input_type_av->currentText();
    QString output_ext = ui->output_type_av->currentText();
    QString input_info = input_ext + " Files " + "(*." + input_ext.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_area->addItem(file_path);
    if (!file_path.isEmpty())
    {
        ui->drag_n_drop_area->clearFiles();
        ui->drag_n_drop_area->addFile(file_path);
    }
}


void MainWindow::on_select_file_doc_clicked()
{
    QString input_ext = ui->input_type_doc->currentText();
    QString output_ext = ui->output_type_doc->currentText();
    QString input_info = input_ext + " Files " + "(*." + input_ext.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_area->addItem(file_path);
    if (!file_path.isEmpty())
    {
        ui->drag_n_drop_area->clearFiles();
        ui->drag_n_drop_area->addFile(file_path);
    }
}


void MainWindow::on_select_file_ss_clicked()
{
    QString input_ext = ui->input_type_spread->currentText();
    QString output_ext = ui->output_type_spread->currentText();
    QString input_info = input_ext + " Files " + "(*." + input_ext.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_area->addItem(file_path);
    if (!file_path.isEmpty())
    {
        ui->drag_n_drop_area->clearFiles();
        ui->drag_n_drop_area->addFile(file_path);
    }
}


void MainWindow::on_select_file_ar_clicked()
{
    QString input_ext = ui->input_type_archive->currentText();
    QString output_ext = ui->output_type_archive->currentText();
    QString input_info = input_ext + " Files " + "(*." + input_ext.toLower() + ")";
    QString file_path = QFileDialog::getOpenFileName(NULL, "Open File", "", input_info);
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_area->addItem(file_path);
    if (!file_path.isEmpty())
    {
        ui->drag_n_drop_area->clearFiles();
        ui->drag_n_drop_area->addFile(file_path);
    }
}


void MainWindow::on_pause_conversion_clicked()
{
    if (!bcm)
    {
        return;
    }
    ui->pause_conversion->setEnabled(false);
    ui->continue_conversion->setEnabled(true);
    bcm->pause();
}


void MainWindow::on_continue_conversion_clicked()
{
    if (!bcm)
    {
        return;
    }
    ui->pause_conversion->setEnabled(true);
    ui->continue_conversion->setEnabled(false);
    bcm->resume();
}

void MainWindow::on_clear_conversion_clicked()
{
    if (bcm)
    {
        disconnect(bcm, nullptr, this, nullptr);
        bcm->deleteLater();
        bcm = nullptr;
    }
    ui->drag_n_drop_area->clearFiles();
    ui->drag_n_drop_text->show();
    int file_type_index = ui->file_sections->currentIndex();
    switch (file_type_index)
    {
        case 0:
            ui->convert_button_image->setEnabled(true);
            ui->convert_button_image_save->setEnabled(true);
            ui->select_file_images->setEnabled(true);
            break;
        case 1:
            ui->convert_button_av->setEnabled(true);
            ui->convert_button_av_save->setEnabled(true);
            ui->select_file_av->setEnabled(true);
            break;
        case 2:
            ui->convert_button_doc->setEnabled(true);
            ui->convert_button_doc_save->setEnabled(true);
            ui->select_file_doc->setEnabled(true);
            break;
        case 3:
            ui->convert_button_spread->setEnabled(true);
            ui->convert_button_spread_save->setEnabled(true);
            ui->select_file_ss->setEnabled(true);
            break;
        case 4:
            ui->convert_button_archive->setEnabled(true);
            ui->convert_button_archive_save->setEnabled(true);
            ui->select_file_ar->setEnabled(true);
            break;
        default:
            break;
    }
    ui->pause_conversion->setEnabled(false);
    ui->continue_conversion->setEnabled(false);
}
