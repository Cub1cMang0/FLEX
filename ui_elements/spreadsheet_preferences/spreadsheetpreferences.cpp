#include "spreadsheetpreferences.h"
#include "ui_spreadsheetpreferences.h"

SpreadsheetPreferences::SpreadsheetPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SpreadsheetPreferences)
{
    ui->setupUi(this);
}

SpreadsheetPreferences::~SpreadsheetPreferences()
{
    delete ui;
}
