#ifndef REPOSITORIES_H
#define REPOSITORIES_H

#include <QList>
#include <QDateTime>
#include <QString>
#include "../models/task.h"

class User;
class Project;

// Базовый интерфейс репозитория (Repository Pattern)
// Инкапсулирует логику доступа к данным
template<typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;
    virtual void add(T *item) = 0;
    virtual void remove(T *item) = 0;
    virtual QList<T*> getAll() const = 0;
    virtual T* findById(int id) const = 0;
    virtual void clear() = 0;
};

class ITaskRepository : public IRepository<Task>
{
public:
    virtual ~ITaskRepository() = default;
    virtual Task* findById(int id) const override = 0;
    virtual QList<Task*> searchByTitle(const QString &keyword) const = 0;
};

class IUserRepository : public IRepository<User>
{
public:
    virtual ~IUserRepository() = default;
    virtual User* findById(int id) const override = 0;
    virtual User* findByName(const QString &name) const = 0;
};

class IProjectRepository : public IRepository<Project>
{
public:
    virtual ~IProjectRepository() = default;
    virtual Project* findById(int id) const override = 0;
    virtual Project* findByName(const QString &name) const = 0;
};

#endif // REPOSITORIES_H

