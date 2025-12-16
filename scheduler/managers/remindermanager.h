#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QList>
#include "../models/reminder.h"

class Task;
class TaskService;

// Менеджер напоминаний (Observer Pattern)
// Отслеживает изменения задач и автоматически управляет напоминаниями
class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(TaskService *taskService, QObject *parent = nullptr);
    ~ReminderManager();
    
    void addReminder(Task *task, int minutesBeforeDeadline);
    void removeReminder(Task *task);
    void removeAllReminders();
    
    Reminder* findReminderByTask(Task *task) const;

signals:
    void reminderNotification(const QString &message, Task *task);

private slots:
    void onReminderTriggered(Reminder *reminder);
    void onTaskAdded(Task *task);
    void onTaskRemoved(Task *task);

private:
    TaskService *m_taskService;
    QList<Reminder*> m_reminders;
    void connectTaskSignals(Task *task);
};

#endif // REMINDERMANAGER_H





