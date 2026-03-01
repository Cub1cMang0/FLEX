#ifndef CHANGESAVELOCATION_H
#define CHANGESAVELOCATION_H

#include <QDialog>

namespace Ui
{
    class ChangeSaveLocation;
}

class ChangeSaveLocation : public QDialog
{
    Q_OBJECT
    public:
        explicit ChangeSaveLocation(QWidget *parent = nullptr);
        ~ChangeSaveLocation();
    private slots:
        void on_change_folder_clicked();

        void on_save_button_clicked();

        void load_save_location();

        void on_cancel_button_clicked();
    private:
        Ui::ChangeSaveLocation *ui;
};

#endif // CHANGESAVELOCATION_H
