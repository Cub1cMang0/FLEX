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
    void on_input_type_image_currentTextChanged(const QString &arg1);

    void on_output_type_image_currentTextChanged(const QString &arg1);

    void on_convert_button_image_clicked();

    void on_input_type_av_currentTextChanged(const QString &arg1);

    void on_output_type_av_currentTextChanged(const QString &arg1);

    void on_convert_button_av_clicked();

    void on_input_type_doc_currentTextChanged(const QString &arg1);

    void on_output_type_doc_currentTextChanged(const QString &arg1);

    void on_convert_button_doc_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
