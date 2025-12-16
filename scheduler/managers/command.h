#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include "../models/task.h"

class ICommand : public QObject
{
    Q_OBJECT

public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual QString getDescription() const = 0;
};

class CommandManager : public QObject
{
    Q_OBJECT

public:
    explicit CommandManager(QObject *parent = nullptr);
    
    void executeCommand(ICommand *command);
    void undo();
    void redo();
    
    bool canUndo() const { return m_currentIndex >= 0; }
    bool canRedo() const { return m_currentIndex < m_commands.size() - 1; }
    
    void clear();

signals:
    void undoAvailable(bool available);
    void redoAvailable(bool available);

private:
    QList<ICommand*> m_commands;
    int m_currentIndex;
    static const int MAX_COMMANDS = 50;
};

class TaskService;
class Task;

class AddTaskCommand : public ICommand
{
public:
    AddTaskCommand(TaskService *service, Task *task);
    void execute() override;
    void undo() override;
    QString getDescription() const override { return "Добавление задачи"; }

private:
    TaskService *m_service;
    Task *m_task;
};

class RemoveTaskCommand : public ICommand
{
public:
    RemoveTaskCommand(TaskService *service, Task *task);
    void execute() override;
    void undo() override;
    QString getDescription() const override { return "Удаление задачи"; }

private:
    TaskService *m_service;
    Task *m_task;
};

class EditTaskCommand : public ICommand
{
public:
    EditTaskCommand(Task *task, const QString &oldTitle, const QString &newTitle,
                    const QDateTime &oldDeadline, const QDateTime &newDeadline,
                    Priority oldPriority, Priority newPriority);
    void execute() override;
    void undo() override;
    QString getDescription() const override { return "Редактирование задачи"; }

private:
    Task *m_task;
    QString m_oldTitle, m_newTitle;
    QDateTime m_oldDeadline, m_newDeadline;
    Priority m_oldPriority, m_newPriority;
};

class CompleteTaskCommand : public ICommand
{
public:
    CompleteTaskCommand(Task *task, bool completed);
    void execute() override;
    void undo() override;
    QString getDescription() const override { return m_completed ? "Завершение задачи" : "Возобновление задачи"; }

private:
    Task *m_task;
    bool m_completed;
};

#endif // COMMAND_H

