#ifndef TASKSERVICE_H
#define TASKSERVICE_H

#include "repositories.h"
#include "strategies.h"
#include <QObject>
#include <QList>
#include <QJsonObject>

class TaskService : public QObject
{
    Q_OBJECT

public:
    explicit TaskService(ITaskRepository *taskRepo, 
                        IUserRepository *userRepo,
                        IProjectRepository *projectRepo,
                        QObject *parent = nullptr);
    
    void addTask(Task *task);
    void removeTask(Task *task);
    QList<Task*> getAllTasks() const;
    
    void addUser(User *user);
    void removeUser(User *user);
    QList<User*> getAllUsers() const;
    User* findUserById(int id) const;
    User* findUserByName(const QString &name) const;
    
    void addProject(Project *project);
    void removeProject(Project *project);
    QList<Project*> getAllProjects() const;
    Project* findProjectById(int id) const;
    Project* findProjectByName(const QString &name) const;
    
    QList<Task*> filterTasks(const QList<IFilterStrategy*> &filters) const;
    QList<Task*> filterByPriority(Priority priority) const;
    QList<Task*> filterByDate(const QDateTime &date) const;
    QList<Task*> filterByProject(Project *project) const;
    QList<Task*> filterByUser(User *user) const;
    QList<Task*> filterCompleted(bool completed) const;
    QList<Task*> searchByTitle(const QString &keyword) const;
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);
    void clearAll();

signals:
    void taskAdded(Task *task);
    void taskRemoved(Task *task);
    void taskUpdated(Task *task);

private:
    ITaskRepository *m_taskRepository;
    IUserRepository *m_userRepository;
    IProjectRepository *m_projectRepository;
};

#endif // TASKSERVICE_H



