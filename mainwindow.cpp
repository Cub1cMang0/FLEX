#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Converts the user's file type to another and outputs a message of the result
void MainWindow::on_convert_button_image_clicked()
{
    ui->image_progress->setValue(0);
    QString result_message = convert_image_file(ui->input_type_image->currentText(), ui->output_type_image->currentText());
    if (result_message != "Failed to load image." || result_message != "Failed to save image.")
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

void MainWindow::on_input_type_image_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_image->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_image->setEnabled(false);
    }
    else
    {
        ui->convert_button_image->setEnabled(true);
    }
}


void MainWindow::on_output_type_image_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_image->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_image->setEnabled(false);
    }
    else
    {
        ui->convert_button_image->setEnabled(true);
    }
}

void MainWindow::on_convert_button_av_clicked()
{
    ui->av_progress->setValue(0);
    QString result_message = convert_video_file(ui->input_type_av->currentText(), ui->output_type_av->currentText(), ui->av_progress);
    ui->result_box->setReadOnly(false);
    ui->result_box->setText(result_message);
    ui->result_box->setReadOnly(true);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
}

void MainWindow::on_input_type_av_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_av->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_av->setEnabled(false);
    }
    else
    {
        ui->convert_button_av->setEnabled(true);
    }
}

void MainWindow::on_output_type_av_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_av->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_av->setEnabled(false);
    }
    else
    {
        ui->convert_button_av->setEnabled(true);
    }
}

void MainWindow::on_convert_button_doc_clicked()
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
    convert_document_file(this, ui->input_type_doc->currentText(), ui->output_type_doc->currentText(), converter);
    ui->result_box->setReadOnly(false);
    ui->result_box->setText(result_message);
    ui->result_box->setReadOnly(true);
    QTimer::singleShot(5000, this, [this]() {
        ui->result_box->setReadOnly(false);
        ui->result_box->clear();
        ui->result_box->setReadOnly(true);
    });
}

void MainWindow::on_input_type_doc_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->output_type_doc->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_doc->setEnabled(false);
    }
    else
    {
        ui->convert_button_doc->setEnabled(true);
    }
}

void MainWindow::on_output_type_doc_currentTextChanged(const QString &arg1)
{
    QString opposite_selection = ui->input_type_doc->currentText();
    if (arg1 == opposite_selection)
    {
        ui->convert_button_doc->setEnabled(false);
    }
    else
    {
        ui->convert_button_doc->setEnabled(true);
    }
}
