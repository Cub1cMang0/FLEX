#ifndef DRAGANDDROP_H
#define DRAGANDDROP_H

#include <QListWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPushButton>
#include <QMimeData>
#include <QDebug>

class DragAndDrop : public QListWidget
{
    Q_OBJECT
    public:
        explicit DragAndDrop(QWidget *parent = nullptr) : QListWidget(parent)
        {
            setAcceptDrops(true);
            setEditTriggers(QAbstractItemView::NoEditTriggers);
            setSelectionMode(QAbstractItemView::ExtendedSelection);
        }
        void clearFiles()
        {
            this->clear();
            files.clear();
        }
        QVector<QString> get_files()
        {
            return files;
        }
        void addFile(const QString &file_path)
        {
            if (!file_path.isEmpty() && !files.contains(file_path))
            {
                this->addItem(file_path);
                files.append(file_path);
            }
        }
        void update_file_path(int index, QString file_path)
        {
            if (index < 0 || index >= files.size())
                return;
            files[index] = file_path;
            QListWidgetItem *item = this->item(index);
            if (item)
            {
                item->setText(file_path);
            }
        }
    protected:
        void dragEnterEvent(QDragEnterEvent *event) override
        {
            if (event->mimeData()->hasUrls())
            {
                event->acceptProposedAction();
            }
            else
            {
                QListWidget::dragEnterEvent(event);
            }
        }
        void dragMoveEvent(QDragMoveEvent *event) override
        {
            if (event->mimeData()->hasUrls())
            {
                event->acceptProposedAction();
            }
            else
            {
                QListWidget::dragMoveEvent(event);
            }
        }
        void dropEvent(QDropEvent *event) override
        {
            QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url : std::as_const(urls))
            {
                QString file_path = url.toLocalFile();
                if (!file_path.isEmpty() && !files.contains(file_path))
                {
                    this->addItem(file_path);
                    files.append(file_path);
                }
            }
            event->acceptProposedAction();
        }
    private:
        QVector<QString> files;
};

#endif // DRAGANDDROP_H
