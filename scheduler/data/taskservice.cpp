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
#include <QJsonDocument>
#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QCoreApplication>
#include <algorithm>

TaskService::TaskService(ITaskRepository *taskRepo, 
                         IUserRepository *userRepo,
                         IProjectRepository *projectRepo,
                         QObject *parent)
    : QObject(parent),
      m_taskRepository(taskRepo),
      m_userRepository(userRepo),
      m_projectRepository(projectRepo)
{
    // Пробрасываем сигналы из репозитория для уведомления подписчиков (UI, ReminderManager)
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

// Применяет несколько фильтров последовательно (пересечение результатов)
QList<Task*> TaskService::filterTasks(const QList<IFilterStrategy*> &filters) const
{
    QList<Task*> tasks = getAllTasks();
    
    // Каждая стратегия фильтрует результат предыдущей
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

// Сериализация всех данных в JSON для сохранения
// Сохраняет связи через ID (ownerId, projectId)
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
        // Сохраняем связи через ID для восстановления при загрузке
        t["ownerId"] = task->getOwner() ? task->getOwner()->getId() : -1;
        t["projectId"] = task->getProject() ? task->getProject()->getId() : -1;
        t["reminderMinutes"] = task->getReminderMinutes();
        tasksArr.append(t);
    }
    root["tasks"] = tasksArr;
    
    return root;
}

// Десериализация данных из JSON
// Восстанавливает связи между задачами, пользователями и проектами по ID
void TaskService::fromJson(const QJsonObject &obj)
{
    clearAll();
    
    // Сначала загружаем пользователей и проекты, создаем карты для быстрого поиска
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
    
    // Затем загружаем задачи и восстанавливаем связи по ID
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
        
        // Восстанавливаем связи через карты
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

// Экспорт задач в JSON массив (для импорта/экспорта файлов)
// Сохраняет связи через имена пользователей и проектов
QJsonArray TaskService::exportTasksToJsonArray() const
{
    QJsonArray array;
    for (Task *task : getAllTasks()) {
        QJsonObject obj;
        obj["title"] = task->getTitle();
        obj["description"] = task->getDescription();
        obj["deadline"] = task->getDeadline().toString(Qt::ISODate);
        obj["priority"] = Task::priorityToString(task->getPriority());
        obj["completed"] = task->isCompleted();
        obj["owner"] = task->getOwner() ? task->getOwner()->getName() : "";
        obj["project"] = task->getProject() ? task->getProject()->getName() : "";
        obj["reminderMinutes"] = task->getReminderMinutes();
        array.append(obj);
    }
    return array;
}

// Импорт задач из JSON массива
// Автоматически создает пользователей и проекты если их нет
// Проверяет дубликаты если skipDuplicates = true
int TaskService::importTasksFromJsonArray(const QJsonArray &array, bool skipDuplicates)
{
    int imported = 0;
    
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        QString title = obj["title"].toString();
        QDateTime deadline = QDateTime::fromString(obj["deadline"].toString(), Qt::ISODate);
        Priority priority = Task::stringToPriority(obj["priority"].toString());
        
        // Создаем пользователя если его нет
        QString ownerName = obj["owner"].toString();
        User *owner = findUserByName(ownerName);
        if (!owner) {
            owner = new User(ownerName);
            addUser(owner);
        }
        
        // Создаем проект если его нет
        QString projectName = obj["project"].toString();
        Project *project = nullptr;
        if (!projectName.isEmpty()) {
            project = findProjectByName(projectName);
            if (!project) {
                project = new Project(projectName);
                addProject(project);
            }
        }
        
        // Проверка дубликатов по названию, дедлайну и владельцу
        if (skipDuplicates) {
            bool isDuplicate = false;
            QList<Task*> allTasks = getAllTasks();
            for (Task *existingTask : allTasks) {
                if (existingTask->getTitle() == title &&
                    existingTask->getDeadline() == deadline &&
                    existingTask->getOwner() == owner) {
                    isDuplicate = true;
                    break;
                }
            }
            if (isDuplicate) {
                continue;
            }
        }
        
        int reminderMinutes = obj["reminderMinutes"].toInt(60);
        if (reminderMinutes < 2) reminderMinutes = 2;
        Task *task = new Task(title, deadline, priority, owner, project, -1, reminderMinutes);
        task->setDescription(obj["description"].toString());
        task->setCompleted(obj["completed"].toBool());
        
        addTask(task);
        imported++;
    }
    
    return imported;
}

// Импорт задач из файла
int TaskService::importTasksFromFile(const QString &fileName, bool skipDuplicates)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1; // Ошибка открытия файла
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        return -1; // Неверный формат файла
    }
    
    QJsonArray array = doc.array();
    return importTasksFromJsonArray(array, skipDuplicates);
}

// Экспорт задач в файл
bool TaskService::exportTasksToFile(const QString &fileName) const
{
    QJsonArray array = exportTasksToJsonArray();
    
    QJsonDocument doc(array);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false; // Ошибка сохранения файла
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

// Комбинированная фильтрация и сортировка задач
// Применяет все фильтры последовательно, затем сортирует
QList<Task*> TaskService::getFilteredAndSortedTasks(const FilterOptions &filterOpts, const SortOptions &sortOpts) const
{
    QList<Task*> tasks = getAllTasks();
    
    // Поиск по названию
    if (!filterOpts.searchText.isEmpty()) {
        QList<Task*> searchResults = searchByTitle(filterOpts.searchText);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (searchResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Фильтр по приоритету
    if (filterOpts.priorityFilterEnabled) {
        QList<Task*> priorityResults = filterByPriority(filterOpts.priorityFilter);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (priorityResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Фильтр по проекту
    if (filterOpts.projectFilter) {
        QList<Task*> projectResults = filterByProject(filterOpts.projectFilter);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (projectResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Фильтр по пользователю
    if (filterOpts.userFilter) {
        QList<Task*> userResults = filterByUser(filterOpts.userFilter);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (userResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Фильтр по дате
    if (filterOpts.dateFilterEnabled && filterOpts.dateFilter.isValid()) {
        QList<Task*> dateResults = filterByDate(filterOpts.dateFilter);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (dateResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Фильтр по статусу завершения
    if (!filterOpts.showCompleted) {
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (!task->isCompleted()) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    // Сортировка: сначала незавершенные, потом завершенные, затем по выбранному критерию
    std::sort(tasks.begin(), tasks.end(), [sortOpts](Task *a, Task *b) {
        bool aCompleted = a->isCompleted();
        bool bCompleted = b->isCompleted();
        
        // Незавершенные всегда перед завершенными
        if (aCompleted != bCompleted) {
            return !aCompleted;
        }
        
        bool less;
        switch (sortOpts.criteria) {
        case SortOptions::SortByDate:
            less = a->getDeadline() < b->getDeadline();
            break;
        case SortOptions::SortByPriority:
            less = static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
            break;
        case SortOptions::SortByTitle:
            less = a->getTitle() < b->getTitle();
            break;
        case SortOptions::SortByProject:
            {
                QString projA = a->getProject() ? a->getProject()->getName() : "";
                QString projB = b->getProject() ? b->getProject()->getName() : "";
                less = projA < projB;
            }
            break;
        default:
            less = false;
        }
        return sortOpts.ascending ? less : !less;
    });
    
    return tasks;
}

// Получение статистики по задачам
TaskService::TaskStatistics TaskService::getStatistics() const
{
    TaskStatistics stats;
    stats.total = getAllTasks().size();
    stats.completed = filterCompleted(true).size();
    stats.active = stats.total - stats.completed;
    return stats;
}

// Инициализация тестовых данных при первом запуске
void TaskService::initializeDefaultData()
{
    if (getAllUsers().isEmpty()) {
        User *user1 = new User("Иван Иванов");
        User *user2 = new User("Мария Петрова");
        addUser(user1);
        addUser(user2);
    }
    if (getAllProjects().isEmpty()) {
        Project *proj1 = new Project("Разработка", "Проект разработки ПО");
        Project *proj2 = new Project("Тестирование", "Проект тестирования");
        addProject(proj1);
        addProject(proj2);
    }
}

// Сохранение данных в файл
bool TaskService::saveToFile(const QString &fileName) const
{
    QString path = fileName.isEmpty() ? 
        QCoreApplication::applicationDirPath() + "/data.json" : fileName;
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    // TaskService сериализует все данные (задачи, пользователи, проекты)
    QJsonObject obj = toJson();
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

// Загрузка данных из файла
bool TaskService::loadFromFile(const QString &fileName)
{
    QString path = fileName.isEmpty() ? 
        QCoreApplication::applicationDirPath() + "/data.json" : fileName;
    
    QFile file(path);
    if (!file.exists()) {
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return false;
    }
    
    // TaskService восстанавливает все данные и связи
    fromJson(doc.object());
    
    return true;
}

