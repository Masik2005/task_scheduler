#include "strategies.h"
#include "../models/project.h"
#include "../models/user.h"
#include <algorithm>

PriorityFilterStrategy::PriorityFilterStrategy(Priority priority)
    : m_priority(priority)
{
}

QList<Task*> PriorityFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->getPriority() == m_priority) {
            result.append(task);
        }
    }
    return result;
}

DateFilterStrategy::DateFilterStrategy(const QDateTime &date)
    : m_date(date)
{
}

QList<Task*> DateFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->getDeadline().date() == m_date.date()) {
            result.append(task);
        }
    }
    return result;
}

ProjectFilterStrategy::ProjectFilterStrategy(Project *project)
    : m_project(project)
{
}

QList<Task*> ProjectFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->getProject() == m_project) {
            result.append(task);
        }
    }
    return result;
}

UserFilterStrategy::UserFilterStrategy(User *user)
    : m_user(user)
{
}

QList<Task*> UserFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->getOwner() == m_user) {
            result.append(task);
        }
    }
    return result;
}

CompletedFilterStrategy::CompletedFilterStrategy(bool completed)
    : m_completed(completed)
{
}

QList<Task*> CompletedFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->isCompleted() == m_completed) {
            result.append(task);
        }
    }
    return result;
}

TitleSearchFilterStrategy::TitleSearchFilterStrategy(const QString &keyword)
    : m_keyword(keyword.toLower())
{
}

QList<Task*> TitleSearchFilterStrategy::filter(const QList<Task*> &tasks) const
{
    QList<Task*> result;
    for (Task *task : tasks) {
        if (task->getTitle().toLower().contains(m_keyword)) {
            result.append(task);
        }
    }
    return result;
}

SortByDateStrategy::SortByDateStrategy(bool ascending)
    : m_ascending(ascending)
{
}

void SortByDateStrategy::sort(QList<Task*> &tasks) const
{
    std::sort(tasks.begin(), tasks.end(), [this](Task *a, Task *b) {
        bool less = a->getDeadline() < b->getDeadline();
        return m_ascending ? less : !less;
    });
}

SortByPriorityStrategy::SortByPriorityStrategy(bool ascending)
    : m_ascending(ascending)
{
}

void SortByPriorityStrategy::sort(QList<Task*> &tasks) const
{
    std::sort(tasks.begin(), tasks.end(), [this](Task *a, Task *b) {
        bool less = static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
        return m_ascending ? less : !less;
    });
}

SortByTitleStrategy::SortByTitleStrategy(bool ascending)
    : m_ascending(ascending)
{
}

void SortByTitleStrategy::sort(QList<Task*> &tasks) const
{
    std::sort(tasks.begin(), tasks.end(), [this](Task *a, Task *b) {
        bool less = a->getTitle() < b->getTitle();
        return m_ascending ? less : !less;
    });
}

SortByProjectStrategy::SortByProjectStrategy(bool ascending)
    : m_ascending(ascending)
{
}

void SortByProjectStrategy::sort(QList<Task*> &tasks) const
{
    std::sort(tasks.begin(), tasks.end(), [this](Task *a, Task *b) {
        QString projA = a->getProject() ? a->getProject()->getName() : "";
        QString projB = b->getProject() ? b->getProject()->getName() : "";
        bool less = projA < projB;
        return m_ascending ? less : !less;
    });
}

