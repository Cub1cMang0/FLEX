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

private slots:
    void fetch_base_preferences();

    void on_cancel_preferences_clicked();

    void check_boxes_states();

    void load_spreadsheet_preferences();

    void on_save_preferences_clicked();

    void on_empty_rc_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_styling_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_pretty_print_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_delimiter_currentTextChanged(const QString &arg1);

private:
    Ui::SpreadsheetPreferences *ui;

};

#endif // SPREADSHEETPREFERENCES_H
