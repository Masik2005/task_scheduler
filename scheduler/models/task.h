#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QObject>

class Project;
class User;

enum class Priority {
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3
};

class Task : public QObject
{
    Q_OBJECT

public:
    Task(const QString &title, const QDateTime &deadline, Priority priority, 
         User *owner, Project *project = nullptr, int id = -1, int reminderMinutes = 60);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    QString getTitle() const { return m_title; }
    void setTitle(const QString &title);
    
    QDateTime getDeadline() const { return m_deadline; }
    void setDeadline(const QDateTime &deadline);
    
    Priority getPriority() const { return m_priority; }
    void setPriority(Priority priority);
    
    bool isCompleted() const { return m_completed; }
    void setCompleted(bool completed);
    
    User* getOwner() const { return m_owner; }
    void setOwner(User *owner);
    
    Project* getProject() const { return m_project; }
    void setProject(Project *project);
    
    QString getDescription() const { return m_description; }
    void setDescription(const QString &description);

    int getReminderMinutes() const { return m_reminderMinutes; }
    void setReminderMinutes(int minutes);
    
    static QString priorityToString(Priority priority);
    static Priority stringToPriority(const QString &str);

signals:
    void taskChanged();

private:
    int m_id;
    QString m_title;
    QString m_description;
    QDateTime m_deadline;
    Priority m_priority;
    bool m_completed;
    User *m_owner;
    Project *m_project;
    int m_reminderMinutes;
};

#endif // TASK_H


