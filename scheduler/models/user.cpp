#include "user.h"
#include "task.h"

User::User(const QString &name, int id)
    : m_id(id), m_name(name)
{
}

void User::addTask(Task *task)
{
    if (task && !m_tasks.contains(task)) {
        m_tasks.append(task);
    }
}

void User::removeTask(Task *task)
{
    m_tasks.removeAll(task);
}





