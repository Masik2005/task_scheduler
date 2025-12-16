#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "../models/task.h"
#include <QList>
#include <QDateTime>
#include <QString>

class User;
class Project;

// Интерфейсы стратегий (Strategy Pattern) для фильтрации и сортировки
// Позволяет легко добавлять новые алгоритмы без изменения существующего кода

class IFilterStrategy
{
public:
    virtual ~IFilterStrategy() = default;
    virtual QList<Task*> filter(const QList<Task*> &tasks) const = 0;
};

class ISortStrategy
{
public:
    virtual ~ISortStrategy() = default;
    virtual void sort(QList<Task*> &tasks) const = 0;
    virtual bool ascending() const = 0;
};

class PriorityFilterStrategy : public IFilterStrategy
{
public:
    explicit PriorityFilterStrategy(Priority priority);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    Priority m_priority;
};

class DateFilterStrategy : public IFilterStrategy
{
public:
    explicit DateFilterStrategy(const QDateTime &date);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    QDateTime m_date;
};

class ProjectFilterStrategy : public IFilterStrategy
{
public:
    explicit ProjectFilterStrategy(Project *project);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    Project *m_project;
};

class UserFilterStrategy : public IFilterStrategy
{
public:
    explicit UserFilterStrategy(User *user);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    User *m_user;
};

class CompletedFilterStrategy : public IFilterStrategy
{
public:
    explicit CompletedFilterStrategy(bool completed);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    bool m_completed;
};

class TitleSearchFilterStrategy : public IFilterStrategy
{
public:
    explicit TitleSearchFilterStrategy(const QString &keyword);
    QList<Task*> filter(const QList<Task*> &tasks) const override;

private:
    QString m_keyword;
};

class SortByDateStrategy : public ISortStrategy
{
public:
    explicit SortByDateStrategy(bool ascending = true);
    void sort(QList<Task*> &tasks) const override;
    bool ascending() const override { return m_ascending; }

private:
    bool m_ascending;
};

class SortByPriorityStrategy : public ISortStrategy
{
public:
    explicit SortByPriorityStrategy(bool ascending = true);
    void sort(QList<Task*> &tasks) const override;
    bool ascending() const override { return m_ascending; }

private:
    bool m_ascending;
};

class SortByTitleStrategy : public ISortStrategy
{
public:
    explicit SortByTitleStrategy(bool ascending = true);
    void sort(QList<Task*> &tasks) const override;
    bool ascending() const override { return m_ascending; }

private:
    bool m_ascending;
};

class SortByProjectStrategy : public ISortStrategy
{
public:
    explicit SortByProjectStrategy(bool ascending = true);
    void sort(QList<Task*> &tasks) const override;
    bool ascending() const override { return m_ascending; }

private:
    bool m_ascending;
};

#endif // STRATEGIES_H

