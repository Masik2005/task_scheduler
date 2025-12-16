#include "command.h"
#include "../data/taskservice.h"
#include "../models/task.h"
#include <QList>

CommandManager::CommandManager(QObject *parent)
    : QObject(parent), m_currentIndex(-1)
{
}

void CommandManager::executeCommand(ICommand *command)
{
    if (!command) return;
    
    while (m_currentIndex < m_commands.size() - 1) {
        delete m_commands.takeLast();
    }
    
    command->execute();
    m_commands.append(command);
    m_currentIndex++;
    
    if (m_commands.size() > MAX_COMMANDS) {
        delete m_commands.takeFirst();
        m_currentIndex--;
    }
    
    emit undoAvailable(canUndo());
    emit redoAvailable(canRedo());
}

void CommandManager::undo()
{
    if (canUndo()) {
        m_commands[m_currentIndex]->undo();
        m_currentIndex--;
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
    }
}

void CommandManager::redo()
{
    if (canRedo()) {
        m_currentIndex++;
        m_commands[m_currentIndex]->execute();
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
    }
}

void CommandManager::clear()
{
    qDeleteAll(m_commands);
    m_commands.clear();
    m_currentIndex = -1;
    emit undoAvailable(false);
    emit redoAvailable(false);
}

AddTaskCommand::AddTaskCommand(TaskService *service, Task *task)
    : m_service(service), m_task(task)
{
}

void AddTaskCommand::execute()
{
    if (m_service && m_task) {
        m_service->addTask(m_task);
    }
}

void AddTaskCommand::undo()
{
    if (m_service && m_task) {
        m_service->removeTask(m_task);
    }
}

RemoveTaskCommand::RemoveTaskCommand(TaskService *service, Task *task)
    : m_service(service), m_task(task)
{
}

void RemoveTaskCommand::execute()
{
    if (m_service && m_task) {
        m_service->removeTask(m_task);
    }
}

void RemoveTaskCommand::undo()
{
    if (m_service && m_task) {
        m_service->addTask(m_task);
    }
}

EditTaskCommand::EditTaskCommand(Task *task, const QString &oldTitle, const QString &newTitle,
                                 const QDateTime &oldDeadline, const QDateTime &newDeadline,
                                 Priority oldPriority, Priority newPriority)
    : m_task(task), m_oldTitle(oldTitle), m_newTitle(newTitle),
      m_oldDeadline(oldDeadline), m_newDeadline(newDeadline),
      m_oldPriority(oldPriority), m_newPriority(newPriority)
{
}

void EditTaskCommand::execute()
{
    if (m_task) {
        m_task->setTitle(m_newTitle);
        m_task->setDeadline(m_newDeadline);
        m_task->setPriority(m_newPriority);
    }
}

void EditTaskCommand::undo()
{
    if (m_task) {
        m_task->setTitle(m_oldTitle);
        m_task->setDeadline(m_oldDeadline);
        m_task->setPriority(m_oldPriority);
    }
}

CompleteTaskCommand::CompleteTaskCommand(Task *task, bool completed)
    : m_task(task), m_completed(completed)
{
}

void CompleteTaskCommand::execute()
{
    if (m_task) {
        m_task->setCompleted(m_completed);
    }
}

void CompleteTaskCommand::undo()
{
    if (m_task) {
        m_task->setCompleted(!m_completed);
    }
}

