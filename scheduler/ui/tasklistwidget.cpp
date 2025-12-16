#include "tasklistwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionViewItem>
#include <QTextOption>
#include <QPainterPath>
#include "../models/task.h"

TaskListWidget::TaskListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

void TaskListWidget::mousePressEvent(QMouseEvent *event)
{
    QListWidgetItem *item = itemAt(event->pos());
    
    if (item == nullptr) {
        clearSelection();
        setCurrentItem(nullptr);
    }
    
    QListWidget::mousePressEvent(event);
}

TaskListDelegate::TaskListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void TaskListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    QVariant bgVar = index.data(Qt::BackgroundRole);
    QVariant fgVar = index.data(Qt::ForegroundRole);
    
    QColor bgColor;
    QColor fgColor;
    
    if (bgVar.isValid() && bgVar.canConvert<QBrush>()) {
        bgColor = bgVar.value<QBrush>().color();
    } else {
        bgColor = QColor("#ffffff");
    }
    
    if (fgVar.isValid() && fgVar.canConvert<QBrush>()) {
        fgColor = fgVar.value<QBrush>().color();
    } else {
        fgColor = QColor("#2c3e50");
    }
    
    if (opt.state & QStyle::State_Selected) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRect rect = opt.rect.adjusted(3, 3, -3, -3);
    QPainterPath path;
    path.addRoundedRect(rect, 5, 5);
    
    painter->fillPath(path, bgColor);
    
    QPen borderPen(QColor("#ecf0f1"), 1);
    painter->setPen(borderPen);
    painter->drawPath(path);
    
    painter->setPen(fgColor);
    QRect textRect = rect.adjusted(10, 5, -10, -5);
    painter->setFont(opt.font);
    
    QString text = opt.text;
    QTextOption textOption(Qt::AlignLeft | Qt::AlignTop);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textOption.setFlags(QTextOption::IncludeTrailingSpaces);
    painter->drawText(textRect, text, textOption);
    
    painter->restore();
}

QSize TaskListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(60);
    if (size.width() < 550) {
        size.setWidth(550);
    }
    return size;
}



