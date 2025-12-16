#ifndef USER_H
#define USER_H

#include <QString>
#include <QList>

class Task;

class User
{
public:
    User(const QString &name, int id = -1);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    QString getName() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    
    void addTask(Task *task);
    void removeTask(Task *task);
    QList<Task*> getTasks() const { return m_tasks; }
    
private:
    int m_id;
    QString m_name;
    QList<Task*> m_tasks;
};

#endif // USER_H


