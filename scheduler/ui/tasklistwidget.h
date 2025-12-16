#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QListWidget>
#include <QStyledItemDelegate>
#include <QMouseEvent>

class Task;
class QListWidgetItem;
class TaskService;
class QComboBox;
class QLineEdit;
class QDateEdit;
class QCheckBox;
class CommandManager;
class ReminderManager;
class TaskEditorDialog;

class TaskListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit TaskListWidget(QWidget *parent = nullptr);
    
    // Устанавливает зависимости для работы с задачами
    void setDependencies(TaskService *service, CommandManager *commandManager, ReminderManager *reminderManager);
    
    // Статический метод для форматирования задачи в элемент списка
    static void formatTaskItem(Task *task, QListWidgetItem *item, int itemWidth = 550);
    
    // Обновляет список задач с применением фильтров и сортировки
    void updateTasks(const QList<Task*> &tasks);
    
    // Обновляет списки проектов и пользователей в фильтрах
    void updateFilters(TaskService *service, QComboBox *projectFilter, QComboBox *userFilter);
    
    // Обновляет список задач на основе UI фильтров (формирует FilterOptions из UI элементов)
    void refreshTaskListWithFilters(QLineEdit *searchEdit, QComboBox *priorityFilter,
                                    QComboBox *projectFilter, QComboBox *userFilter,
                                    QDateEdit *dateFilter, bool dateFilterEnabled,
                                    QCheckBox *showCompletedCheckBox, QComboBox *sortCombo);
    
    // Действия с задачами
    void addTask();
    void editTask();
    void deleteTask();
    void completeTask();
    
    // Получить выбранную задачу
    Task* getSelectedTask() const;

signals:
    // Сигналы для уведомления об изменениях (для обновления UI)
    void taskListChanged();
    void taskDoubleClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    TaskService *m_taskService;
    CommandManager *m_commandManager;
    ReminderManager *m_reminderManager;
};

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



