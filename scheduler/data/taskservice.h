#ifndef TASKSERVICE_H
#define TASKSERVICE_H

#include "repositories.h"
#include "strategies.h"
#include <QObject>
#include <QList>
#include <QJsonObject>

// Фасад (Facade Pattern) для работы с данными
// Объединяет работу с репозиториями задач, пользователей и проектов
// Предоставляет высокоуровневый API для UI
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
    
    // Фильтрация задач через Strategy Pattern
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
    
    // Импорт/экспорт задач в формате JSON массива
    QJsonArray exportTasksToJsonArray() const;
    int importTasksFromJsonArray(const QJsonArray &array, bool skipDuplicates = true);
    
    // Импорт/экспорт задач из/в файл
    // Возвращает количество импортированных задач, -1 при ошибке
    int importTasksFromFile(const QString &fileName, bool skipDuplicates = true);
    // Возвращает true при успехе, false при ошибке
    bool exportTasksToFile(const QString &fileName) const;
    
    // Комбинированная фильтрация и сортировка задач
    struct FilterOptions {
        QString searchText;
        Priority priorityFilter = Priority::Low; // -1 означает "все"
        bool priorityFilterEnabled = false;
        Project *projectFilter = nullptr;
        User *userFilter = nullptr;
        QDateTime dateFilter;
        bool dateFilterEnabled = false;
        bool showCompleted = true;
    };
    
    struct SortOptions {
        enum Criteria { SortByDate, SortByPriority, SortByTitle, SortByProject };
        Criteria criteria = SortByDate;
        bool ascending = true;
    };
    
    QList<Task*> getFilteredAndSortedTasks(const FilterOptions &filterOpts, const SortOptions &sortOpts) const;
    
    // Статистика задач
    struct TaskStatistics {
        int total;
        int completed;
        int active;
    };
    TaskStatistics getStatistics() const;
    
    // Инициализация тестовых данных (для первого запуска)
    void initializeDefaultData();
    
    // Сохранение и загрузка данных из файла
    bool saveToFile(const QString &fileName = QString()) const;
    bool loadFromFile(const QString &fileName = QString());

signals:
    // Сигналы пробрасываются из репозитория для уведомления UI об изменениях
    void taskAdded(Task *task);
    void taskRemoved(Task *task);
    void taskUpdated(Task *task);

private:
    ITaskRepository *m_taskRepository;
    IUserRepository *m_userRepository;
    IProjectRepository *m_projectRepository;
};

#endif // TASKSERVICE_H



