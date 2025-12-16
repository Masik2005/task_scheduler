#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QListWidget>
#include <QStyledItemDelegate>
#include <QMouseEvent>

// Кастомный виджет списка задач с поддержкой снятия выделения
class TaskListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit TaskListWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

// Кастомный делегат для отрисовки элементов списка задач
class TaskListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskListDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // TASKLISTWIDGET_H



