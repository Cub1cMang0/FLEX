#ifndef ARCHIVEPREFERENCES_H
#define ARCHIVEPREFERENCES_H

#include <QDialog>

namespace Ui {
class ArchivePreferences;
}

class ArchivePreferences : public QDialog
{
    Q_OBJECT

public:
    explicit ArchivePreferences(QWidget *parent = nullptr);
    ~ArchivePreferences();

private slots:
    void fetch_base_preferences();

    void set_zip_level_slider(QString level);

    void set_7zip_level_slider(QString level);

    void load_archive_preferences();

    void on_save_preferences_clicked();

    void check_boxes_states();

    void on_zip_comp_method_currentTextChanged(const QString &arg1);

    void on_seven_zip_comp_method_currentTextChanged(const QString &arg1);

    void on_tar_comp_method_currentTextChanged(const QString &arg1);

    void on_xar_comp_method_currentTextChanged(const QString &arg1);

    void on_cpio_format_currentTextChanged(const QString &arg1);

    void on_ar_format_currentTextChanged(const QString &arg1);

    void on_tar_metadata_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_xar_metadata_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_cpio_metadata_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_ar_determ_mode_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_cancel_preferences_clicked();

    void on_zip_comp_slider_sliderMoved(int position);

    void on_seven_zip_comp_slider_sliderMoved(int position);

    void on_iso_uppercase_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_iso_enforce_83_cb_checkStateChanged(const Qt::CheckState &arg1);

    void on_iso_rm_invalid_cb_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::ArchivePreferences *ui;
};

#endif // ARCHIVEPREFERENCES_H
