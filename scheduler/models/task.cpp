#include "task.h"
#include "project.h"
#include "user.h"

Task::Task(const QString &title, const QDateTime &deadline, Priority priority,
           User *owner, Project *project, int id, int reminderMinutes)
    : m_id(id), m_title(title), m_deadline(deadline), m_priority(priority),
      m_completed(false), m_owner(owner), m_project(project), m_reminderMinutes(reminderMinutes)
{
    // Автоматически добавляем задачу в список задач пользователя (двунаправленная связь)
    if (m_owner) {
        m_owner->addTask(this);
    }
}

void Task::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit taskChanged();
    }
}

void Task::setDeadline(const QDateTime &deadline)
{
    if (m_deadline != deadline) {
        m_deadline = deadline;
        emit taskChanged();
    }
}

void Task::setPriority(Priority priority)
{
    if (m_priority != priority) {
        m_priority = priority;
        emit taskChanged();
    }
}

void Task::setCompleted(bool completed)
{
    if (m_completed != completed) {
        m_completed = completed;
        emit taskChanged();
    }
}

void Task::setOwner(User *owner)
{
    // При смене владельца обновляем связи в обе стороны
    if (m_owner != owner) {
        if (m_owner) {
            m_owner->removeTask(this);
        }
        m_owner = owner;
        if (m_owner) {
            m_owner->addTask(this);
        }
        emit taskChanged();
    }
}

void Task::setProject(Project *project)
{
    if (m_project != project) {
        m_project = project;
        emit taskChanged();
    }
}

void Task::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        emit taskChanged();
    }
}

void Task::setReminderMinutes(int minutes)
{
    if (minutes < 0) {
        minutes = 0;
    }
    if (m_reminderMinutes != minutes) {
        m_reminderMinutes = minutes;
        emit taskChanged();
    }
}

QString Task::priorityToString(Priority priority)
{
    switch (priority) {
    case Priority::Low: return "Низкий";
    case Priority::Medium: return "Средний";
    case Priority::High: return "Высокий";
    default: return "Неизвестно";
    }
}

Priority Task::stringToPriority(const QString &str)
{
    if (str == "Низкий" || str == "Low") return Priority::Low;
    if (str == "Средний" || str == "Medium") return Priority::Medium;
    if (str == "Высокий" || str == "High") return Priority::High;
    return Priority::Medium;
}

