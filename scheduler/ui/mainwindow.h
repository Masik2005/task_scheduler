#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "../data/taskservice.h"
#include "../managers/command.h"
#include "../managers/remindermanager.h"
#include "ui_mainwindow.h"

class Task;

QT_BEGIN_NAMESPACE
class QListWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QLabel;
class QMenuBar;
class QStatusBar;
class QAction;
class QSystemTrayIcon;
QT_END_NAMESPACE

class TaskEditorDialog;
class UserManagerDialog;
class ProjectManagerDialog;
class TaskListWidget;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTask();
    void onEditTask();
    void onDeleteTask();
    void onCompleteTask();
    void onTaskDoubleClicked(QListWidgetItem *item);
    void onSearchTextChanged(const QString &text);
    void onFilterChanged();
    void onSortChanged();
    void onUserManager();
    void onProjectManager();
    void onImportTasks();
    void onExportTasks();
    void onUndo();
    void onRedo();
    void onReminderNotification(const QString &message, Task *task);
    void refreshTaskList();
    void saveData();
    bool loadData();
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void updateTaskList();
    void updateStatusBar();
    Task* getSelectedTask() const;
    void showTaskInList(Task *task, QListWidgetItem *item);
    void applyStyles();
    void updateCompleteButtonText();
    
    TaskService *m_taskService;
    CommandManager *m_commandManager;
    ReminderManager *m_reminderManager;
    
    TaskListWidget *m_taskList;
    bool m_dateFilterEnabled;
    
    QAction *m_undoAction;
    QAction *m_redoAction;
    QSystemTrayIcon *m_trayIcon;
};

#endif // MAINWINDOW_H
