#include "tasklistwidget.h"
#include "../models/task.h"
#include "../models/project.h"
#include "../models/user.h"
#include "../data/taskservice.h"
#include "../managers/command.h"
#include "../managers/remindermanager.h"
#include "taskeditor.h"
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionViewItem>
#include <QTextOption>
#include <QPainterPath>
#include <QColor>
#include <QBrush>
#include <QSize>
#include <QComboBox>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QCheckBox>
#include <QTime>

TaskListWidget::TaskListWidget(QWidget *parent)
    : QListWidget(parent),
      m_taskService(nullptr),
      m_commandManager(nullptr),
      m_reminderManager(nullptr)
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

// Форматирует задачу для отображения в списке
// Устанавливает текст, цвета в зависимости от приоритета и статуса
// Сохраняет указатель на задачу в Qt::UserRole для быстрого доступа
void TaskListWidget::formatTaskItem(Task *task, QListWidgetItem *item, int itemWidth)
{
    if (!task || !item) return;
    
    QString priorityText = Task::priorityToString(task->getPriority());
    
    QString title = task->getTitle();
    if (task->isCompleted()) {
        title = QString("✓ %1").arg(title);
    }
    
    QString text = QString("[%1] %2")
                   .arg(priorityText)
                   .arg(title);
    
    QString deadlineStr = task->getDeadline().toString("dd.MM.yyyy HH:mm");
    text += QString(" | Дедлайн: %1").arg(deadlineStr);
    
    if (task->getProject()) {
        text += QString(" | 📁 %1").arg(task->getProject()->getName());
    }
    if (task->getOwner()) {
        text += QString(" | 👤 %1").arg(task->getOwner()->getName());
    }
    
    item->setText(text);
    // Сохраняем указатель на задачу для быстрого доступа
    item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(task)));
    
    // Цветовая схема: зеленый (низкий), желтый (средний), красный (высокий), серый (завершенные)
    QColor bgColor, textColor;
    
    if (task->isCompleted()) {
        bgColor = QColor("#ffffff");
        textColor = QColor("#7f8c8d");
    } else {
        switch (task->getPriority()) {
        case Priority::Low:
            bgColor = QColor("#d5f4e6");
            textColor = QColor("#2c3e50");
            break;
        case Priority::Medium:
            bgColor = QColor("#fff9c4");
            textColor = QColor("#2c3e50");
            break;
        case Priority::High:
            bgColor = QColor("#ffcdd2");
            textColor = QColor("#2c3e50");
            break;
        default:
            bgColor = QColor("#ffffff");
            textColor = QColor("#2c3e50");
        }
    }
    
    QBrush bgBrush(bgColor);
    QBrush textBrush(textColor);
    
    item->setData(Qt::BackgroundRole, bgBrush);
    item->setData(Qt::ForegroundRole, textBrush);
    item->setBackground(bgBrush);
    item->setForeground(textBrush);
    
    item->setSizeHint(QSize(itemWidth, 60));
}

void TaskListWidget::updateTasks(const QList<Task*> &tasks)
{
    clear();
    
    for (Task *task : tasks) {
        QListWidgetItem *item = new QListWidgetItem();
        addItem(item);
        int itemWidth = qMax(width() - 20, 550);
        formatTaskItem(task, item, itemWidth);
    }
}

void TaskListWidget::setDependencies(TaskService *service, CommandManager *commandManager, ReminderManager *reminderManager)
{
    m_taskService = service;
    m_commandManager = commandManager;
    m_reminderManager = reminderManager;
}

void TaskListWidget::updateFilters(TaskService *service, QComboBox *projectFilter, QComboBox *userFilter)
{
    if (!service || !projectFilter || !userFilter) return;
    
    // Обновляем список проектов в фильтре
    projectFilter->clear();
    projectFilter->addItem("Все", -1);
    for (Project *project : service->getAllProjects()) {
        projectFilter->addItem(project->getName(), project->getId());
    }
    
    // Обновляем список пользователей в фильтре
    userFilter->clear();
    userFilter->addItem("Все", -1);
    for (User *user : service->getAllUsers()) {
        userFilter->addItem(user->getName(), user->getId());
    }
}

Task* TaskListWidget::getSelectedTask() const
{
    QListWidgetItem *item = currentItem();
    if (item) {
        return static_cast<Task*>(item->data(Qt::UserRole).value<void*>());
    }
    return nullptr;
}

void TaskListWidget::addTask()
{
    if (!m_taskService || !m_commandManager || !m_reminderManager) return;
    
    TaskEditorDialog *editor = new TaskEditorDialog(m_taskService, parentWidget());
    
    if (editor->exec() == QDialog::Accepted) {
        Task *task = editor->getCreatedTask();
        if (task) {
            // Используем Command Pattern для поддержки undo/redo
            AddTaskCommand *cmd = new AddTaskCommand(m_taskService, task);
            m_commandManager->executeCommand(cmd);
            // Автоматически создаем напоминание для новой задачи
            if (m_reminderManager && task) {
                m_reminderManager->addReminder(task, task->getReminderMinutes());
            }
            emit taskListChanged();
        }
    }
    delete editor;
}

void TaskListWidget::editTask()
{
    if (!m_taskService || !m_commandManager || !m_reminderManager) return;
    
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(parentWidget(), "Информация", "Выберите задачу для редактирования");
        return;
    }
    
    TaskEditorDialog *editor = new TaskEditorDialog(m_taskService, parentWidget());
    editor->setTask(task);
    
    // Сохраняем старое состояние для команды undo
    QString oldTitle = task->getTitle();
    QDateTime oldDeadline = task->getDeadline();
    Priority oldPriority = task->getPriority();
    
    if (editor->exec() == QDialog::Accepted) {
        Task *editedTask = editor->getCreatedTask();
        if (editedTask) {
            // Создаем команду только для полей, поддерживающих undo
            EditTaskCommand *cmd = new EditTaskCommand(
                task,
                oldTitle, editedTask->getTitle(),
                oldDeadline, editedTask->getDeadline(),
                oldPriority, editedTask->getPriority()
            );
            m_commandManager->executeCommand(cmd);
            // Остальные поля обновляем напрямую
            task->setDescription(editedTask->getDescription());
            task->setProject(editedTask->getProject());
            task->setOwner(editedTask->getOwner());
            task->setReminderMinutes(editedTask->getReminderMinutes());
            // Обновляем напоминание при изменении задачи
            if (m_reminderManager && task) {
                m_reminderManager->addReminder(task, task->getReminderMinutes());
            }
            emit taskListChanged();
        }
    }
    delete editor;
}

void TaskListWidget::deleteTask()
{
    if (!m_taskService || !m_commandManager) return;
    
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(parentWidget(), "Информация", "Выберите задачу для удаления");
        return;
    }
    
    if (QMessageBox::question(parentWidget(), "Подтверждение", 
                              QString("Удалить задачу '%1'?").arg(task->getTitle())) == QMessageBox::Yes) {
        RemoveTaskCommand *cmd = new RemoveTaskCommand(m_taskService, task);
        m_commandManager->executeCommand(cmd);
        emit taskListChanged();
    }
}

void TaskListWidget::completeTask()
{
    if (!m_taskService || !m_commandManager || !m_reminderManager) return;
    
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(parentWidget(), "Информация", "Выберите задачу");
        return;
    }
    
    bool completed = !task->isCompleted();
    CompleteTaskCommand *cmd = new CompleteTaskCommand(task, completed);
    m_commandManager->executeCommand(cmd);
    
    if (m_reminderManager && task) {
        if (completed) {
            m_reminderManager->removeReminder(task);
        } else {
            m_reminderManager->addReminder(task, task->getReminderMinutes());
        }
    }
    
    emit taskListChanged();
}

void TaskListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListWidget::mouseDoubleClickEvent(event);
    emit taskDoubleClicked();
}

void TaskListWidget::refreshTaskListWithFilters(QLineEdit *searchEdit, QComboBox *priorityFilter,
                                                  QComboBox *projectFilter, QComboBox *userFilter,
                                                  QDateEdit *dateFilter, bool dateFilterEnabled,
                                                  QCheckBox *showCompletedCheckBox, QComboBox *sortCombo)
{
    if (!m_taskService || !searchEdit || !priorityFilter || !projectFilter || 
        !userFilter || !sortCombo) {
        return;
    }
    
    // Формируем опции фильтрации из UI элементов
    TaskService::FilterOptions filterOpts;
    filterOpts.searchText = searchEdit->text().trimmed();
    
    int priorityFilterValue = priorityFilter->currentData().toInt();
    if (priorityFilterValue >= 0) {
        filterOpts.priorityFilter = static_cast<Priority>(priorityFilterValue);
        filterOpts.priorityFilterEnabled = true;
    }
    
    int projectIndex = projectFilter->currentIndex();
    if (projectIndex > 0 && projectIndex <= m_taskService->getAllProjects().size()) {
        filterOpts.projectFilter = m_taskService->getAllProjects()[projectIndex - 1];
    }
    
    int userIndex = userFilter->currentIndex();
    if (userIndex > 0 && userIndex <= m_taskService->getAllUsers().size()) {
        filterOpts.userFilter = m_taskService->getAllUsers()[userIndex - 1];
    }
    
    if (dateFilter && dateFilterEnabled && dateFilter->date().isValid()) {
        filterOpts.dateFilter = QDateTime(dateFilter->date(), QTime(0, 0));
        filterOpts.dateFilterEnabled = true;
    }
    
    filterOpts.showCompleted = showCompletedCheckBox ? showCompletedCheckBox->isChecked() : true;
    
    // Формируем опции сортировки
    TaskService::SortOptions sortOpts;
    sortOpts.criteria = static_cast<TaskService::SortOptions::Criteria>(
        sortCombo->currentData().toInt());
    sortOpts.ascending = true;
    
    // Получаем отфильтрованные и отсортированные задачи из TaskService
    QList<Task*> tasks = m_taskService->getFilteredAndSortedTasks(filterOpts, sortOpts);
    
    // Отображаем задачи
    updateTasks(tasks);
    
    // Не эмитируем taskListChanged здесь, чтобы избежать бесконечного цикла
    // Этот метод вызывается из updateTaskList, который уже вызывается из refreshTaskList
}



