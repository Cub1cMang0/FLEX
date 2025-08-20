#ifndef DRAGANDDROP_H
#define DRAGANDDROP_H

#include <QTextEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPushButton>
#include <QMimeData>
#include <QDebug>

class DragAndDrop : public QTextEdit
{
    Q_OBJECT
    public:
        explicit DragAndDrop(QWidget *parent = nullptr) : QTextEdit(parent)
        {
            setAcceptDrops(true);
            setReadOnly(true);
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
                QTextEdit::dragEnterEvent(event);
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
                QTextEdit::dragMoveEvent(event);
            }
        }
        void dropEvent(QDropEvent *event) override
        {
            QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url : urls)
            {
                QString file_path = url.toLocalFile();
                if (!file_path.isEmpty() && !files.contains(file_path))
                {
                    append(file_path);
                    files.insert(file_path);
                    QPushButton *button = new QPushButton(this);
                }
            }
            event->acceptProposedAction();
        }
    private:
        QSet<QString> files;
};

#endif // DRAGANDDROP_H
