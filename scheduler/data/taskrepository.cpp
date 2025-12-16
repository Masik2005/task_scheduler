#include "taskrepository.h"
#include "../models/task.h"
#include "../models/user.h"
#include "../models/project.h"

TaskRepository::TaskRepository(QObject *parent)
    : QObject(parent), m_nextTaskId(1)
{
}

void TaskRepository::add(Task *task)
{
    if (task && !m_tasks.contains(task)) {
        if (task->getId() < 0) {
            task->setId(m_nextTaskId++);
        }
        m_tasks.append(task);
        emit taskAdded(task);
    }
}

void TaskRepository::remove(Task *task)
{
    if (m_tasks.removeAll(task) > 0) {
        emit taskRemoved(task);
    }
}

Task* TaskRepository::findById(int id) const
{
    for (Task *task : m_tasks) {
        if (task->getId() == id) {
            return task;
        }
    }
    return nullptr;
}

void TaskRepository::clear()
{
    m_tasks.clear();
    m_nextTaskId = 1;
}

QList<Task*> TaskRepository::searchByTitle(const QString &keyword) const
{
    QList<Task*> result;
    QString lowerKeyword = keyword.toLower();
    for (Task *task : m_tasks) {
        if (task->getTitle().toLower().contains(lowerKeyword)) {
            result.append(task);
        }
    }
    return result;
}



