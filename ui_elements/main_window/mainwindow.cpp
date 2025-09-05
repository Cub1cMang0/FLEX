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
#include "mainimageconverter.h"
#include "mainvideoconverter.h"
#include "maindocumentconverter.h"
#include "mainarchiveconverter.h"
#include "mainspreadconverter.h"
#include "changesavelocation.h"
#include "imagepreferences.h"
#include "videoaudiopreferences.h"
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
    check_save_location();
    setup_progress_bars();
}

MainWindow::~MainWindow()
{
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
    ui->image_progress->setValue(0);
    QString result_message = convert_image_file(ui->input_type_image->currentText(), ui->output_type_image->currentText(), save_folder);
    if (result_message != "Failed to load image." && result_message != "Failed to save image." && result_message != "No location selected")
    {
        ui->image_progress->setValue(100);
    }
    ui->result_box->setReadOnly(false);
    ui->result_box->setText(result_message);
    ui->result_box->setReadOnly(true);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
}

// Converts the user's file type to another and outputs a message of the result
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

void MainWindow::convert_user_av(QString save_folder)
{
    ui->av_progress->setValue(0);
    QString result_message = convert_video_file(ui->input_type_av->currentText(), ui->output_type_av->currentText(), ui->av_progress, save_folder);
    ui->result_box->setReadOnly(false);
    ui->result_box->setText(result_message);
    ui->result_box->setReadOnly(true);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
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

// Converts the user's provided document(s)
void MainWindow::convert_user_document(QString save_folder)
{
    ui->doc_progress->setMinimum(0);
    ui->doc_progress->setMaximum(0);
    MainDocumentConverter *converter = new MainDocumentConverter(this);
    QObject::connect(converter, &MainDocumentConverter::conversionFinished, [this](bool success, const QString &message) mutable
    {
        ui->result_box->setReadOnly(false);
        ui->result_box->setText(message);
        ui->result_box->setReadOnly(true);
        ui->doc_progress->setRange(0, 1);
        if (success)
        {
            ui->doc_progress->setValue(1);
        }
        else
        {
            ui->doc_progress->setValue(0);
        }
    });
    convert_document_file(this, ui->input_type_doc->currentText(), ui->output_type_doc->currentText(), converter, save_folder);
    QTimer::singleShot(5000, this, [this]()
    {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
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

void MainWindow::convert_user_archive(QString save_folder)
{
    ui->archives_progress->setMinimum(0);
    ui->archives_progress->setMaximum(0);
    MainArchiveConverter *converter = new MainArchiveConverter(this);
    QObject::connect(converter, &MainArchiveConverter::conversion_finished,
        [this](bool success, const QString &message) mutable {
            ui->result_box->setReadOnly(false);
            ui->result_box->setText(message);
            ui->result_box->setReadOnly(true);
            ui->archives_progress->setRange(0, 1);
            if (success)
            {
                ui->archives_progress->setValue(1);
            }
            else
            {
                ui->archives_progress->setValue(0);
            }
        });
    convert_archive_file(this, ui->input_type_archive->currentText(), ui->output_type_archive->currentText(), converter, save_folder);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
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

void MainWindow::convert_user_spreadsheet(QString save_folder)
{
    ui->spread_progress->setMinimum(0);
    ui->spread_progress->setMaximum(0);
    MainSpreadConverter *converter = new MainSpreadConverter(this);
    QObject::connect(converter, &MainSpreadConverter::conversion_finished, [this](bool success, const QString &message) mutable
    {
        ui->result_box->setReadOnly(false);
        ui->result_box->setText(message);
        ui->result_box->setReadOnly(true);
        ui->spread_progress->setRange(0, 1);
        if (success)
        {
            ui->spread_progress->setValue(1);
        }
        else
        {
            ui->spread_progress->setValue(0);
        }
    });
    convert_spread_file(this, ui->input_type_spread->currentText(), ui->output_type_spread->currentText(), converter, save_folder);
    QTimer::singleShot(5000, this, [this]()
    {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
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

void MainWindow::on_drag_n_drop_area_textChanged()
{
    ui->drag_n_drop_text->setText("");
}

