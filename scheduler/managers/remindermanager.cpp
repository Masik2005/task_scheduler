#include "remindermanager.h"
#include "../data/taskservice.h"
#include "../models/task.h"
#include "../models/reminder.h"

ReminderManager::ReminderManager(TaskService *taskService, QObject *parent)
    : QObject(parent), m_taskService(taskService)
{
    // Подписываемся на сигналы TaskService для автоматического управления напоминаниями
    if (m_taskService) {
        connect(m_taskService, &TaskService::taskAdded, this, &ReminderManager::onTaskAdded);
        connect(m_taskService, &TaskService::taskRemoved, this, &ReminderManager::onTaskRemoved);
    }
}

ReminderManager::~ReminderManager()
{
    removeAllReminders();
}

// Добавляет напоминание для задачи
// Если напоминание уже существует - удаляет старое и создает новое
void ReminderManager::addReminder(Task *task, int minutesBeforeDeadline)
{
    if (!task || task->isCompleted()) {
        return;
    }
    
    // Удаляем старое напоминание если есть
    removeReminder(task);
    
    if (minutesBeforeDeadline < 2) {
        minutesBeforeDeadline = 2;
    }
    
    Reminder *reminder = new Reminder(task, minutesBeforeDeadline, this);
    // Подключаемся к сигналу срабатывания напоминания
    connect(reminder, &Reminder::reminderTriggered, this, &ReminderManager::onReminderTriggered);
    
    m_reminders.append(reminder);
    reminder->activate(); // Запускаем таймер
    
    // Подключаемся к сигналам задачи для отслеживания изменений
    connectTaskSignals(task);
}

void ReminderManager::removeReminder(Task *task)
{
    Reminder *reminder = findReminderByTask(task);
    if (reminder) {
        reminder->deactivate();
        m_reminders.removeAll(reminder);
        reminder->deleteLater();
    }
}

void ReminderManager::removeAllReminders()
{
    for (Reminder *reminder : m_reminders) {
        reminder->deactivate();
        reminder->deleteLater();
    }
    m_reminders.clear();
}

Reminder* ReminderManager::findReminderByTask(Task *task) const
{
    for (Reminder *reminder : m_reminders) {
        if (reminder->getTask() == task) {
            return reminder;
        }
    }
    return nullptr;
}

// Обработчик срабатывания напоминания
// Эмитирует сигнал для показа уведомления в UI
void ReminderManager::onReminderTriggered(Reminder *reminder)
{
    if (!reminder || !reminder->getTask()) {
        return;
    }
    
    Task *task = reminder->getTask();
    
    // Не показываем напоминание для завершенных задач
    if (task->isCompleted()) {
        return;
    }
    
    QString message = QString("Задача '%1' должна быть выполнена через %2 минут")
                     .arg(task->getTitle())
                     .arg(reminder->getMinutesBeforeDeadline());
    
    // Уведомляем UI (MainWindow покажет уведомление в системном трее)
    emit reminderNotification(message, task);
    removeReminder(task); // Удаляем напоминание после срабатывания
}

// Автоматически создает напоминание для новой задачи
// Вызывается при получении сигнала taskAdded от TaskService
void ReminderManager::onTaskAdded(Task *task)
{
    if (task && !task->isCompleted()) {
        int reminderMinutes = task->getReminderMinutes();
        if (reminderMinutes < 2) {
            reminderMinutes = 2;
        }
        addReminder(task, reminderMinutes);
    }
}

void ReminderManager::onTaskRemoved(Task *task)
{
    if (task) {
        removeReminder(task);
    }
}

// Подключается к сигналу taskChanged для автоматического обновления напоминания
// при изменении дедлайна или статуса задачи
void ReminderManager::connectTaskSignals(Task *task)
{
    if (!task) {
        return;
    }
    
    connect(task, &Task::taskChanged, this, [this, task]() {
        Reminder *reminder = findReminderByTask(task);
        if (reminder) {
            if (task->isCompleted()) {
                // Удаляем напоминание для завершенных задач
                removeReminder(task);
            } else {
                // Пересоздаем напоминание при изменении задачи (например, дедлайна)
                int reminderMinutes = task->getReminderMinutes();
                if (reminderMinutes < 2) {
                    reminderMinutes = 2;
                }
                removeReminder(task);
                addReminder(task, reminderMinutes);
            }
        }
    });
}


