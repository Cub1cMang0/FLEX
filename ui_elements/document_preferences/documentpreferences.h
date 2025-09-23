#ifndef DOCUMENTPREFERENCES_H
#define DOCUMENTPREFERENCES_H

#include <QDialog>

namespace Ui {
class DocumentPreferences;
}

class DocumentPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentPreferences(QWidget *parent = nullptr);
    ~DocumentPreferences();

private slots:

    void set_tooltips();

    void fetch_base_preferences();

    void load_document_preferences();

    void on_save_preferences_clicked();

    void on_cancel_preferences_clicked();

    void check_boxes_states();

    void on_preserve_metadata_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_preserve_formatting_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_preserve_media_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_line_breaks_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_standalone_cb_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::DocumentPreferences *ui;
};

struct DocumentFormatCapabilities
{
    bool remove_meta_support = false;
    bool preserve_format_support = false;
    bool preserve_media_support = false;
    bool line_break_support = false;
    bool standalone_support = false;
};

inline const QMap<QString, DocumentFormatCapabilities> document_capabilities =
    {
        {"docx", {true, true, true, false, false}},
        {"epub", {true, false, true, false, true}},
        {"html", {true, true, true, true, true}},
        {"json", {false, false, false, false, false}},
        {"pptx",{true, true, true, false, false}},
        {"md", {true, false, false, true, true}},
        {"pdf", {true, true, true, true, true}},
        {"tex", {true, true, false, true, true}},
        {"odt", {true, true, true, false, false}},
        {"rtf", {true, false, true, false, false}}
};

#endif // DOCUMENTPREFERENCES_H
