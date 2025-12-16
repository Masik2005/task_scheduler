#include "taskservice.h"
#include "taskrepository.h"
#include "userrepository.h"
#include "projectrepository.h"
#include "strategies.h"
#include "../models/task.h"
#include "../models/user.h"
#include "../models/project.h"
#include <QMap>
#include <QJsonArray>
#include <QJsonValue>

TaskService::TaskService(ITaskRepository *taskRepo, 
                         IUserRepository *userRepo,
                         IProjectRepository *projectRepo,
                         QObject *parent)
    : QObject(parent),
      m_taskRepository(taskRepo),
      m_userRepository(userRepo),
      m_projectRepository(projectRepo)
{
    TaskRepository *repo = dynamic_cast<TaskRepository*>(taskRepo);
    if (repo) {
        connect(repo, &TaskRepository::taskAdded, this, &TaskService::taskAdded);
        connect(repo, &TaskRepository::taskRemoved, this, &TaskService::taskRemoved);
        connect(repo, &TaskRepository::taskUpdated, this, &TaskService::taskUpdated);
    }
}

void TaskService::addTask(Task *task)
{
    if (m_taskRepository) {
        m_taskRepository->add(task);
    }
}

void TaskService::removeTask(Task *task)
{
    if (m_taskRepository) {
        m_taskRepository->remove(task);
    }
}

QList<Task*> TaskService::getAllTasks() const
{
    return m_taskRepository ? m_taskRepository->getAll() : QList<Task*>();
}

void TaskService::addUser(User *user)
{
    if (m_userRepository) {
        m_userRepository->add(user);
    }
}

void TaskService::removeUser(User *user)
{
    if (m_userRepository) {
        m_userRepository->remove(user);
    }
}

QList<User*> TaskService::getAllUsers() const
{
    return m_userRepository ? m_userRepository->getAll() : QList<User*>();
}

User* TaskService::findUserById(int id) const
{
    return m_userRepository ? m_userRepository->findById(id) : nullptr;
}

User* TaskService::findUserByName(const QString &name) const
{
    return m_userRepository ? m_userRepository->findByName(name) : nullptr;
}

void TaskService::addProject(Project *project)
{
    if (m_projectRepository) {
        m_projectRepository->add(project);
    }
}

void TaskService::removeProject(Project *project)
{
    if (m_projectRepository) {
        m_projectRepository->remove(project);
    }
}

QList<Project*> TaskService::getAllProjects() const
{
    return m_projectRepository ? m_projectRepository->getAll() : QList<Project*>();
}

Project* TaskService::findProjectById(int id) const
{
    return m_projectRepository ? m_projectRepository->findById(id) : nullptr;
}

Project* TaskService::findProjectByName(const QString &name) const
{
    return m_projectRepository ? m_projectRepository->findByName(name) : nullptr;
}

QList<Task*> TaskService::filterTasks(const QList<IFilterStrategy*> &filters) const
{
    QList<Task*> tasks = getAllTasks();
    
    for (IFilterStrategy *filter : filters) {
        if (filter) {
            tasks = filter->filter(tasks);
        }
    }
    
    return tasks;
}

QList<Task*> TaskService::filterByPriority(Priority priority) const
{
    PriorityFilterStrategy strategy(priority);
    QList<IFilterStrategy*> filters;
    filters.append(&strategy);
    return filterTasks(filters);
}

QList<Task*> TaskService::filterByDate(const QDateTime &date) const
{
    DateFilterStrategy strategy(date);
    QList<IFilterStrategy*> filters;
    filters.append(&strategy);
    return filterTasks(filters);
}

QList<Task*> TaskService::filterByProject(Project *project) const
{
    ProjectFilterStrategy strategy(project);
    QList<IFilterStrategy*> filters;
    filters.append(&strategy);
    return filterTasks(filters);
}

QList<Task*> TaskService::filterByUser(User *user) const
{
    UserFilterStrategy strategy(user);
    QList<IFilterStrategy*> filters;
    filters.append(&strategy);
    return filterTasks(filters);
}

QList<Task*> TaskService::filterCompleted(bool completed) const
{
    CompletedFilterStrategy strategy(completed);
    QList<IFilterStrategy*> filters;
    filters.append(&strategy);
    return filterTasks(filters);
}

QList<Task*> TaskService::searchByTitle(const QString &keyword) const
{
    return m_taskRepository ? m_taskRepository->searchByTitle(keyword) : QList<Task*>();
}

QJsonObject TaskService::toJson() const
{
    QJsonObject root;
    
    QJsonArray usersArr;
    for (User *user : getAllUsers()) {
        QJsonObject u;
        u["id"] = user->getId();
        u["name"] = user->getName();
        usersArr.append(u);
    }
    root["users"] = usersArr;
    
    QJsonArray projectsArr;
    for (Project *project : getAllProjects()) {
        QJsonObject p;
        p["id"] = project->getId();
        p["name"] = project->getName();
        p["description"] = project->getDescription();
        projectsArr.append(p);
    }
    root["projects"] = projectsArr;
    
    QJsonArray tasksArr;
    for (Task *task : getAllTasks()) {
        QJsonObject t;
        t["id"] = task->getId();
        t["title"] = task->getTitle();
        t["description"] = task->getDescription();
        t["deadline"] = task->getDeadline().toString(Qt::ISODate);
        t["priority"] = Task::priorityToString(task->getPriority());
        t["completed"] = task->isCompleted();
        t["ownerId"] = task->getOwner() ? task->getOwner()->getId() : -1;
        t["projectId"] = task->getProject() ? task->getProject()->getId() : -1;
        t["reminderMinutes"] = task->getReminderMinutes();
        tasksArr.append(t);
    }
    root["tasks"] = tasksArr;
    
    return root;
}

void TaskService::fromJson(const QJsonObject &obj)
{
    clearAll();
    
    QMap<int, User*> usersById;
    QMap<int, Project*> projectsById;
    
    QJsonArray usersArr = obj["users"].toArray();
    for (const QJsonValue &val : usersArr) {
        QJsonObject u = val.toObject();
        int id = u["id"].toInt(-1);
        QString name = u["name"].toString();
        User *user = new User(name, id);
        addUser(user);
        usersById[id] = user;
    }
    
    QJsonArray projectsArr = obj["projects"].toArray();
    for (const QJsonValue &val : projectsArr) {
        QJsonObject p = val.toObject();
        int id = p["id"].toInt(-1);
        QString name = p["name"].toString();
        QString desc = p["description"].toString();
        Project *proj = new Project(name, desc, id);
        addProject(proj);
        projectsById[id] = proj;
    }
    
    QJsonArray tasksArr = obj["tasks"].toArray();
    for (const QJsonValue &val : tasksArr) {
        QJsonObject t = val.toObject();
        int id = t["id"].toInt(-1);
        QString title = t["title"].toString();
        QString description = t["description"].toString();
        QDateTime deadline = QDateTime::fromString(t["deadline"].toString(), Qt::ISODate);
        Priority pr = Task::stringToPriority(t["priority"].toString());
        bool completed = t["completed"].toBool(false);
        int ownerId = t["ownerId"].toInt(-1);
        int projectId = t["projectId"].toInt(-1);
        int reminderMinutes = t["reminderMinutes"].toInt(60);
        
        User *owner = usersById.value(ownerId, nullptr);
        Project *proj = projectsById.value(projectId, nullptr);
        
        Task *task = new Task(title, deadline, pr, owner, proj, id, reminderMinutes);
        task->setDescription(description);
        task->setCompleted(completed);
        addTask(task);
    }
    
    TaskRepository *taskRepo = dynamic_cast<TaskRepository*>(m_taskRepository);
    if (taskRepo) {
        int maxTaskId = 0;
        for (Task *t : getAllTasks()) {
            maxTaskId = qMax(maxTaskId, t->getId());
        }
        taskRepo->setNextId(maxTaskId + 1);
    }
    
    UserRepository *userRepo = dynamic_cast<UserRepository*>(m_userRepository);
    if (userRepo) {
        int maxUserId = 0;
        for (User *u : getAllUsers()) {
            maxUserId = qMax(maxUserId, u->getId());
        }
        userRepo->setNextId(maxUserId + 1);
    }
    
    ProjectRepository *projectRepo = dynamic_cast<ProjectRepository*>(m_projectRepository);
    if (projectRepo) {
        int maxProjectId = 0;
        for (Project *p : getAllProjects()) {
            maxProjectId = qMax(maxProjectId, p->getId());
        }
        projectRepo->setNextId(maxProjectId + 1);
    }
}

void TaskService::clearAll()
{
    if (m_taskRepository) m_taskRepository->clear();
    if (m_userRepository) m_userRepository->clear();
    if (m_projectRepository) m_projectRepository->clear();
}

