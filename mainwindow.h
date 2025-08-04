#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void convert_user_image(bool alt_save_location);

    void on_convert_button_image_clicked();

    void on_convert_button_image_save_clicked();

    void on_input_type_image_currentTextChanged(const QString &arg1);

    void on_output_type_image_currentTextChanged(const QString &arg1);

    void convert_user_av(bool alt_save_location);

    void on_convert_button_av_clicked();

    void on_convert_button_av_save_clicked();

    void on_input_type_av_currentTextChanged(const QString &arg1);

    void on_output_type_av_currentTextChanged(const QString &arg1);

    void convert_user_document(bool alt_save_location);

    void on_convert_button_doc_clicked();

    void on_convert_button_doc_save_clicked();

    void on_input_type_doc_currentTextChanged(const QString &arg1);

    void on_output_type_doc_currentTextChanged(const QString &arg1);

    void exit_program();

    void change_save_folder();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
