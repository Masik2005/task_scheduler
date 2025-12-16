#ifndef TASKREPOSITORY_H
#define TASKREPOSITORY_H

#include "repositories.h"
#include <QObject>

// Реализация репозитория задач - хранит задачи в памяти (QList)
// Эмитирует сигналы при изменениях для уведомления подписчиков
class TaskRepository : public QObject, public ITaskRepository
{
    Q_OBJECT

public:
    explicit TaskRepository(QObject *parent = nullptr);
    
    // IRepository interface
    void add(Task *task) override;
    void remove(Task *task) override;
    QList<Task*> getAll() const override { return m_tasks; }
    Task* findById(int id) const override;
    void clear() override;
    
    // ITaskRepository interface
    QList<Task*> searchByTitle(const QString &keyword) const override;
    
    int getNextId() { return m_nextTaskId++; }
    void setNextId(int id) { m_nextTaskId = id; }

signals:
    void taskAdded(Task *task);
    void taskRemoved(Task *task);
    void taskUpdated(Task *task);

private:
    QList<Task*> m_tasks;
    int m_nextTaskId;
};

#endif // TASKREPOSITORY_H



