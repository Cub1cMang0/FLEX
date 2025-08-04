#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QAction>
#include <QApplication>
#include "mainimageconverter.h"
#include "mainvideoconverter.h"
#include "maindocumentconverter.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->result_box->setReadOnly(true);
    ui->av_progress->setRange(0, 100);
    ui->av_progress->setValue(0);
    ui->image_progress->setRange(0, 100);
    ui->image_progress->setValue(0);
    ui->doc_progress->setValue(0);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit_program);
    connect(ui->actionChange_Save_Folder, &QAction::triggered, this, &MainWindow::change_save_folder);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::exit_program()
{
    QApplication::quit();
}

void MainWindow::change_save_folder()
{
    return;
}

void MainWindow::convert_user_image(bool alt_save_location)
{
    ui->image_progress->setValue(0);
    QString result_message = convert_image_file(ui->input_type_image->currentText(), ui->output_type_image->currentText(), alt_save_location);
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
    convert_user_image(false);
}

void MainWindow::on_convert_button_image_save_clicked()
{
    convert_user_image(true);
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

void MainWindow::convert_user_av(bool alt_save_location)
{
    ui->av_progress->setValue(0);
    QString result_message = convert_video_file(ui->input_type_av->currentText(), ui->output_type_av->currentText(), ui->av_progress, alt_save_location);
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
    convert_user_av(false);
}

void MainWindow::on_convert_button_av_save_clicked()
{
    convert_user_av(true);
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
        ui->convert_button_av->setEnabled(true);
    }
}

// Converts the user's provided document(s)
void MainWindow::convert_user_document(bool alt_save_location)
{
    ui->doc_progress->setMinimum(0);
    ui->doc_progress->setMaximum(0);
    QString result_message;
    MainDocumentConverter *converter = new MainDocumentConverter(this);
    QObject::connect(converter, &MainDocumentConverter::conversionFinished,
        [this](bool success, const QString &message) mutable {
            ui->result_box->setText(message);
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
    convert_document_file(this, ui->input_type_doc->currentText(), ui->output_type_doc->currentText(), converter, alt_save_location);
    ui->result_box->setReadOnly(false);
    ui->result_box->setText(result_message);
    ui->result_box->setReadOnly(true);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });

}

void MainWindow::on_convert_button_doc_clicked()
{
    convert_user_document(false);
}

void MainWindow::on_convert_button_doc_save_clicked()
{
    convert_user_document(true);
}


void MainWindow::on_input_type_doc_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_doc->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_doc->setEnabled(false);
        ui->convert_button_av_save->setEnabled(false);
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
