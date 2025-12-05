#ifndef SPREADSHEETPREFERENCES_H
#define SPREADSHEETPREFERENCES_H

#include <QDialog>

namespace Ui {
class SpreadsheetPreferences;
}

class SpreadsheetPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit SpreadsheetPreferences(QWidget *parent = nullptr);
    ~SpreadsheetPreferences();

private:
    Ui::SpreadsheetPreferences *ui;
};

#endif // SPREADSHEETPREFERENCES_H
