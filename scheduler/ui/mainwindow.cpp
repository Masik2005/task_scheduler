#include "mainwindow.h"
#include "taskeditor.h"
#include "usermanager.h"
#include "projectmanager.h"
#include "tasklistwidget.h"
#include "../managers/command.h"
#include "../models/task.h"
#include "../models/project.h"
#include "../models/user.h"
#include "../data/taskservice.h"
#include "../data/taskrepository.h"
#include "../data/userrepository.h"
#include "../data/projectrepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QKeySequence>
#include <QFile>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QStyle>
#include <QApplication>
#include <QCloseEvent>
#include <algorithm>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QColor>
#include <QDate>
#include <QTime>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_taskService(nullptr),
      m_commandManager(nullptr),
      m_reminderManager(nullptr),
      m_taskList(nullptr),
      m_dateFilterEnabled(false),
      m_undoAction(nullptr),
      m_redoAction(nullptr),
      m_trayIcon(nullptr)
{
    setupUi(this);
    
    TaskRepository *taskRepo = new TaskRepository(this);
    UserRepository *userRepo = new UserRepository(this);
    ProjectRepository *projectRepo = new ProjectRepository(this);
    m_taskService = new TaskService(taskRepo, userRepo, projectRepo, this);
    
    m_commandManager = new CommandManager(this);
    m_reminderManager = new ReminderManager(m_taskService, this);
    
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        m_trayIcon->setToolTip("Планировщик задач");
        m_trayIcon->show();
    }
    
    connect(m_reminderManager, &ReminderManager::reminderNotification,
            this, &MainWindow::onReminderNotification);
    connect(m_commandManager, &CommandManager::undoAvailable,
            this, [this](bool available) { if (m_undoAction) m_undoAction->setEnabled(available); });
    connect(m_commandManager, &CommandManager::redoAvailable,
            this, [this](bool available) { if (m_redoAction) m_redoAction->setEnabled(available); });
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    
    bool dataLoaded = loadData();

    if (!dataLoaded) {
        if (m_taskService->getAllUsers().isEmpty()) {
            User *user1 = new User("Иван Иванов");
            User *user2 = new User("Мария Петрова");
            m_taskService->addUser(user1);
            m_taskService->addUser(user2);
        }
        if (m_taskService->getAllProjects().isEmpty()) {
            Project *proj1 = new Project("Разработка", "Проект разработки ПО");
            Project *proj2 = new Project("Тестирование", "Проект тестирования");
            m_taskService->addProject(proj1);
            m_taskService->addProject(proj2);
        }
        refreshTaskList();
    }
    
    if (m_reminderManager) {
        m_reminderManager->removeAllReminders();
        for (Task *task : m_taskService->getAllTasks()) {
            if (!task->isCompleted()) {
                m_reminderManager->addReminder(task, task->getReminderMinutes());
            }
        }
    }
    
    setWindowTitle("Планировщик задач");
    resize(1200, 800);
    applyStyles();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    searchEdit->setPlaceholderText("Введите название задачи...");
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    priorityFilter->addItem("Все", -1);
    priorityFilter->addItem("Низкий", static_cast<int>(Priority::Low));
    priorityFilter->addItem("Средний", static_cast<int>(Priority::Medium));
    priorityFilter->addItem("Высокий", static_cast<int>(Priority::High));
    connect(priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    
    projectFilter->addItem("Все", -1);
    connect(projectFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    
    userFilter->addItem("Все", -1);
    connect(userFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    
    dateFilter->setDate(QDate::currentDate());
    dateFilter->setMinimumDate(QDate(2000, 1, 1));
    dateFilter->setMaximumDate(QDate(2100, 12, 31));
    connect(dateFilter, &QDateEdit::dateChanged, this, [this]() {
        m_dateFilterEnabled = true;
        onFilterChanged();
    });
    
    clearDateButton->setStyleSheet("");
    clearDateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(clearDateButton, &QPushButton::clicked, this, [this]() {
        m_dateFilterEnabled = false;
        dateFilter->blockSignals(true);
        dateFilter->setDate(QDate::currentDate());
        dateFilter->blockSignals(false);
        onFilterChanged();
    });
    
    enum SortCriteria {
        SortByDate = 0,
        SortByPriority = 1,
        SortByTitle = 2,
        SortByProject = 3
    };
    sortCombo->addItem("По дате", SortByDate);
    sortCombo->addItem("По приоритету", SortByPriority);
    sortCombo->addItem("По названию", SortByTitle);
    sortCombo->addItem("По проекту", SortByProject);
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortChanged);
    
    if (showCompletedCheckBox) {
        showCompletedCheckBox->setChecked(true);
        connect(showCompletedCheckBox, &QCheckBox::toggled, this, &MainWindow::onFilterChanged);
    }
    
    QListWidget *oldList = taskList;
    m_taskList = new TaskListWidget(this);
    TaskListDelegate *delegate = new TaskListDelegate(this);
    m_taskList->setItemDelegate(delegate);
    m_taskList->setAlternatingRowColors(false);
    m_taskList->setMinimumWidth(600);
    connect(m_taskList, &QListWidget::itemDoubleClicked, this, &MainWindow::onTaskDoubleClicked);
    connect(m_taskList, &QListWidget::itemSelectionChanged, this, &MainWindow::updateCompleteButtonText);
    
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(QMainWindow::centralWidget()->layout());
    if (mainLayout) {
        int index = mainLayout->indexOf(oldList);
        if (index >= 0) {
            mainLayout->removeWidget(oldList);
            mainLayout->insertWidget(index, m_taskList);
            oldList->deleteLater();
        }
    }
    
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddTask);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditTask);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteTask);
    connect(completeButton, &QPushButton::clicked, this, &MainWindow::onCompleteTask);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = QMainWindow::menuBar()->addMenu("Файл");
    fileMenu->addAction("Импорт задач...", this, &MainWindow::onImportTasks);
    fileMenu->addAction("Экспорт задач...", this, &MainWindow::onExportTasks);
    fileMenu->addSeparator();
    fileMenu->addAction("Выход", this, &QWidget::close);
    
    QMenu *editMenu = QMainWindow::menuBar()->addMenu("Правка");
    m_undoAction = editMenu->addAction("Отменить", this, &MainWindow::onUndo);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setEnabled(false);
    m_redoAction = editMenu->addAction("Повторить", this, &MainWindow::onRedo);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setEnabled(false);
    
    QMenu *toolsMenu = QMainWindow::menuBar()->addMenu("Инструменты");
    toolsMenu->addAction("Управление пользователями...", this, &MainWindow::onUserManager);
    toolsMenu->addAction("Управление проектами...", this, &MainWindow::onProjectManager);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Главная");
    toolBar->setMovable(false);
    toolBar->addAction("Добавить", this, &MainWindow::onAddTask);
    toolBar->addAction("Редактировать", this, &MainWindow::onEditTask);
    toolBar->addAction("Удалить", this, &MainWindow::onDeleteTask);
    toolBar->addSeparator();
    toolBar->addAction(m_undoAction);
    toolBar->addAction(m_redoAction);
}

void MainWindow::setupStatusBar()
{
    updateStatusBar();
}

void MainWindow::updateTaskList()
{
    if (!m_taskList || !m_taskService || !searchEdit || !priorityFilter || 
        !projectFilter || !userFilter || !sortCombo) {
        return;
    }
    
    m_taskList->clear();
    
    QList<Task*> tasks = m_taskService->getAllTasks();
    
    QString searchText = searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        QList<Task*> searchResults = m_taskService->searchByTitle(searchText);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (searchResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    int priorityFilterValue = priorityFilter->currentData().toInt();
    if (priorityFilterValue >= 0) {
        QList<Task*> priorityResults = m_taskService->filterByPriority(static_cast<Priority>(priorityFilterValue));
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (priorityResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    int projectIndex = projectFilter->currentIndex();
    if (projectIndex > 0 && projectIndex <= m_taskService->getAllProjects().size()) {
        Project *project = m_taskService->getAllProjects()[projectIndex - 1];
        QList<Task*> projectResults = m_taskService->filterByProject(project);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (projectResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    int userIndex = userFilter->currentIndex();
    if (userIndex > 0 && userIndex <= m_taskService->getAllUsers().size()) {
        User *user = m_taskService->getAllUsers()[userIndex - 1];
        QList<Task*> userResults = m_taskService->filterByUser(user);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (userResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    if (dateFilter && m_dateFilterEnabled && dateFilter->date().isValid()) {
        QDateTime filterDate = QDateTime(dateFilter->date(), QTime(0, 0));
        QList<Task*> dateResults = m_taskService->filterByDate(filterDate);
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (dateResults.contains(task)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    bool showCompleted = showCompletedCheckBox ? showCompletedCheckBox->isChecked() : true;
    if (!showCompleted) {
        QList<Task*> filtered;
        for (Task *task : tasks) {
            if (!task->isCompleted()) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }
    
    enum SortCriteria {
        SortByDate = 0,
        SortByPriority = 1,
        SortByTitle = 2,
        SortByProject = 3
    };
    
    SortCriteria criteria = static_cast<SortCriteria>(
        sortCombo->currentData().toInt());
    
    std::sort(tasks.begin(), tasks.end(), [criteria](Task *a, Task *b) {
        bool aCompleted = a->isCompleted();
        bool bCompleted = b->isCompleted();
        
        if (aCompleted != bCompleted) {
            return !aCompleted;
        }
        
        bool less;
        switch (criteria) {
        case SortByDate:
            less = a->getDeadline() < b->getDeadline();
            break;
        case SortByPriority:
            less = static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
            break;
        case SortByTitle:
            less = a->getTitle() < b->getTitle();
            break;
        case SortByProject:
            {
                QString projA = a->getProject() ? a->getProject()->getName() : "";
                QString projB = b->getProject() ? b->getProject()->getName() : "";
                less = projA < projB;
            }
            break;
        default:
            less = false;
        }
        return less;
    });
    
    for (Task *task : tasks) {
        QListWidgetItem *item = new QListWidgetItem();
        m_taskList->addItem(item);
        showTaskInList(task, item);
    }
    
    updateCompleteButtonText();
}

void MainWindow::showTaskInList(Task *task, QListWidgetItem *item)
{
    QString priorityText = Task::priorityToString(task->getPriority());
    
    QString title = task->getTitle();
    if (task->isCompleted()) {
        title = QString("✓ %1").arg(title);
    }
    
    QString text = QString("[%1] %2")
                   .arg(priorityText)
                   .arg(title);
    
    QString deadlineStr = task->getDeadline().toString("dd.MM.yyyy HH:mm");
    text += QString(" | Дедлайн: %1").arg(deadlineStr);
    
    if (task->getProject()) {
        text += QString(" | 📁 %1").arg(task->getProject()->getName());
    }
    if (task->getOwner()) {
        text += QString(" | 👤 %1").arg(task->getOwner()->getName());
    }
    
    item->setText(text);
    item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(task)));
    
    QColor bgColor, textColor;
    
    if (task->isCompleted()) {
        bgColor = QColor("#ffffff");
        textColor = QColor("#7f8c8d");
    } else {
        switch (task->getPriority()) {
        case Priority::Low:
            bgColor = QColor("#d5f4e6");
            textColor = QColor("#2c3e50");
            break;
        case Priority::Medium:
            bgColor = QColor("#fff9c4");
            textColor = QColor("#2c3e50");
            break;
        case Priority::High:
            bgColor = QColor("#ffcdd2");
            textColor = QColor("#2c3e50");
            break;
        default:
            bgColor = QColor("#ffffff");
            textColor = QColor("#2c3e50");
        }
    }
    
    QBrush bgBrush(bgColor);
    QBrush textBrush(textColor);
    
    item->setData(Qt::BackgroundRole, bgBrush);
    item->setData(Qt::ForegroundRole, textBrush);
    item->setBackground(bgBrush);
    item->setForeground(textBrush);
    
    int itemWidth = m_taskList ? qMax(m_taskList->width() - 20, 550) : 550;
    item->setSizeHint(QSize(itemWidth, 60));
}

Task* MainWindow::getSelectedTask() const
{
    QListWidgetItem *item = m_taskList->currentItem();
    if (item) {
        return static_cast<Task*>(item->data(Qt::UserRole).value<void*>());
    }
    return nullptr;
}

void MainWindow::onAddTask()
{
    TaskEditorDialog *editor = new TaskEditorDialog(m_taskService, this);
    
    if (editor->exec() == QDialog::Accepted) {
        Task *task = editor->getCreatedTask();
        if (task) {
            AddTaskCommand *cmd = new AddTaskCommand(m_taskService, task);
            m_commandManager->executeCommand(cmd);
            if (m_reminderManager && task) {
                m_reminderManager->addReminder(task, task->getReminderMinutes());
            }
            refreshTaskList();
        }
    }
    delete editor;
}

void MainWindow::onEditTask()
{
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(this, "Информация", "Выберите задачу для редактирования");
        return;
    }
    
    TaskEditorDialog *editor = new TaskEditorDialog(m_taskService, this);
    editor->setTask(task);
    
    QString oldTitle = task->getTitle();
    QDateTime oldDeadline = task->getDeadline();
    Priority oldPriority = task->getPriority();
    
    if (editor->exec() == QDialog::Accepted) {
        Task *editedTask = editor->getCreatedTask();
        if (editedTask) {
            EditTaskCommand *cmd = new EditTaskCommand(
                task,
                oldTitle, editedTask->getTitle(),
                oldDeadline, editedTask->getDeadline(),
                oldPriority, editedTask->getPriority()
            );
            m_commandManager->executeCommand(cmd);
            task->setDescription(editedTask->getDescription());
            task->setProject(editedTask->getProject());
            task->setOwner(editedTask->getOwner());
            task->setReminderMinutes(editedTask->getReminderMinutes());
            if (m_reminderManager && task) {
                m_reminderManager->addReminder(task, task->getReminderMinutes());
            }
            refreshTaskList();
        }
    }
    delete editor;
}

void MainWindow::onDeleteTask()
{
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(this, "Информация", "Выберите задачу для удаления");
        return;
    }
    
    if (QMessageBox::question(this, "Подтверждение", 
                              QString("Удалить задачу '%1'?").arg(task->getTitle())) == QMessageBox::Yes) {
        RemoveTaskCommand *cmd = new RemoveTaskCommand(m_taskService, task);
        m_commandManager->executeCommand(cmd);
        refreshTaskList();
    }
}

void MainWindow::onCompleteTask()
{
    Task *task = getSelectedTask();
    if (!task) {
        QMessageBox::information(this, "Информация", "Выберите задачу");
        return;
    }
    
    bool completed = !task->isCompleted();
    CompleteTaskCommand *cmd = new CompleteTaskCommand(task, completed);
    m_commandManager->executeCommand(cmd);
    
    if (m_reminderManager && task) {
        if (completed) {
            m_reminderManager->removeReminder(task);
        } else {
            m_reminderManager->addReminder(task, task->getReminderMinutes());
        }
    }
    
    refreshTaskList();
}

void MainWindow::onTaskDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    onEditTask();
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    updateTaskList();
}

void MainWindow::onFilterChanged()
{
    updateTaskList();
}

void MainWindow::onSortChanged()
{
    updateTaskList();
}

void MainWindow::onUserManager()
{
    UserManagerDialog *dialog = new UserManagerDialog(m_taskService, this);
    dialog->exec();
    delete dialog;
    refreshTaskList();
}

void MainWindow::onProjectManager()
{
    ProjectManagerDialog *dialog = new ProjectManagerDialog(m_taskService, this);
    dialog->exec();
    delete dialog;
    refreshTaskList();
}

void MainWindow::onImportTasks()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Импорт задач", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        QMessageBox::critical(this, "Ошибка", "Неверный формат файла");
        return;
    }
    
    QJsonArray array = doc.array();
    int imported = 0;
    int skipped = 0;
    
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        QString title = obj["title"].toString();
        QDateTime deadline = QDateTime::fromString(obj["deadline"].toString(), Qt::ISODate);
        Priority priority = Task::stringToPriority(obj["priority"].toString());
        
        QString ownerName = obj["owner"].toString();
        User *owner = m_taskService->findUserByName(ownerName);
        if (!owner) {
            owner = new User(ownerName);
            m_taskService->addUser(owner);
        }
        
        QString projectName = obj["project"].toString();
        Project *project = nullptr;
        if (!projectName.isEmpty()) {
            project = m_taskService->findProjectByName(projectName);
            if (!project) {
                project = new Project(projectName);
                m_taskService->addProject(project);
            }
        }
        
        bool isDuplicate = false;
        if (m_taskService) {
            QList<Task*> allTasks = m_taskService->getAllTasks();
            for (Task *existingTask : allTasks) {
                if (existingTask->getTitle() == title &&
                    existingTask->getDeadline() == deadline &&
                    existingTask->getOwner() == owner) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        
        if (isDuplicate) {
            skipped++;
            continue;
        }
        
        int reminderMinutes = obj["reminderMinutes"].toInt(60);
        if (reminderMinutes < 2) reminderMinutes = 2;
        Task *task = new Task(title, deadline, priority, owner, project, -1, reminderMinutes);
        task->setDescription(obj["description"].toString());
        task->setCompleted(obj["completed"].toBool());
        
        AddTaskCommand *cmd = new AddTaskCommand(m_taskService, task);
        m_commandManager->executeCommand(cmd);
        imported++;
    }
    
    QString message = QString("Импортировано задач: %1").arg(imported);
    if (skipped > 0) {
        message += QString("\nПропущено дубликатов: %1").arg(skipped);
    }
    QMessageBox::information(this, "Импорт", message);
    refreshTaskList();
}

void MainWindow::onExportTasks()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт задач", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    
    QJsonArray array;
    for (Task *task : m_taskService->getAllTasks()) {
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
    
    QJsonDocument doc(array);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
        return;
    }
    
    file.write(doc.toJson());
    file.close();
    
    QMessageBox::information(this, "Экспорт", QString("Экспортировано задач: %1").arg(array.size()));
}

void MainWindow::onUndo()
{
    m_commandManager->undo();
    refreshTaskList();
}

void MainWindow::onRedo()
{
    m_commandManager->redo();
    refreshTaskList();
}

void MainWindow::onReminderNotification(const QString &message, Task *task)
{
    Q_UNUSED(task);
    
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(
            "Напоминание о задаче",
            message,
            QSystemTrayIcon::Information,
            10000
        );
    }
    
    refreshTaskList();
}

void MainWindow::refreshTaskList()
{
    if (!projectFilter || !userFilter || !m_taskService) {
        return;
    }
    
    projectFilter->clear();
    projectFilter->addItem("Все", -1);
    for (Project *project : m_taskService->getAllProjects()) {
        projectFilter->addItem(project->getName(), project->getId());
    }
    
    userFilter->clear();
    userFilter->addItem("Все", -1);
    for (User *user : m_taskService->getAllUsers()) {
        userFilter->addItem(user->getName(), user->getId());
    }
    
    updateTaskList();
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    if (!m_taskService) {
        return;
    }
    
    int total = m_taskService->getAllTasks().size();
    int completed = m_taskService->filterCompleted(true).size();
    QMainWindow::statusBar()->showMessage(QString("Всего задач: %1 | Завершено: %2 | Активных: %3")
                           .arg(total).arg(completed).arg(total - completed));
}

void MainWindow::updateCompleteButtonText()
{
    Task *task = getSelectedTask();
    if (task) {
        if (task->isCompleted()) {
            completeButton->setText("Возобновить");
            completeButton->setStyleSheet(
                "QPushButton {"
                "    background-color: #27ae60;"
                "}"
                "QPushButton:hover {"
                "    background-color: #229954;"
                "}"
            );
        } else {
            completeButton->setText("Завершить");
            completeButton->setStyleSheet(
                "QPushButton {"
                "    background-color: #9b59b6;"
                "}"
                "QPushButton:hover {"
                "    background-color: #8e44ad;"
                "}"
            );
        }
    } else {
        completeButton->setText("Завершить");
    }
}

void MainWindow::saveData()
{
    if (!m_taskService) return;
    QString path = QCoreApplication::applicationDirPath() + "/data.json";
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    QJsonObject obj = m_taskService->toJson();
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
}

bool MainWindow::loadData()
{
    if (!m_taskService) return false;
    QString path = QCoreApplication::applicationDirPath() + "/data.json";
    QFile file(path);
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return false;
    m_taskService->fromJson(doc.object());
    
    refreshTaskList();
    updateTaskList();
    updateStatusBar();
    
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    QMainWindow::closeEvent(event);
}

void MainWindow::applyStyles()
{
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"
        
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #d0d0d0;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 0 8px;"
        "    color: #2c3e50;"
        "    font-size: 12pt;"
        "}"
        
        "QLineEdit {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "}"
        
        "QLineEdit:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #ecf0f1;"
        "}"
        
        "QComboBox {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 6px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "    min-width: 120px;"
        "}"
        
        "QComboBox:hover {"
        "    border: 2px solid #3498db;"
        "}"
        
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 5px solid transparent;"
        "    border-right: 5px solid transparent;"
        "    border-top: 6px solid #34495e;"
        "    margin-right: 5px;"
        "}"
        
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "    min-width: 100px;"
        "}"
        
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        
        "QPushButton:disabled {"
        "    background-color: #bdc3c7;"
        "    color: #7f8c8d;"
        "}"
        
        "QListWidget {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "    padding: 5px;"
        "    font-size: 11pt;"
        "}"
        
        "QListWidget::item {"
        "    border: 1px solid #ecf0f1;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "    margin: 3px;"
        "    background-color: transparent;"
        "}"
        
        "QListWidget::item:hover {"
        "    border: 2px solid #3498db;"
        "}"
        
        "QListWidget::item:selected {"
        "    border: 2px solid #2980b9;"
        "    background-color: rgba(52, 152, 219, 0.5);"
        "}"
        
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 11pt;"
        "}"
        
        "QStatusBar {"
        "    background-color: #34495e;"
        "    color: white;"
        "    font-size: 10pt;"
        "    padding: 5px;"
        "}"
        
        "QStatusBar QLabel {"
        "    color: white;"
        "    background-color: transparent;"
        "}"
        
        "QMenuBar {"
        "    background-color: #34495e;"
        "    color: white;"
        "    font-size: 11pt;"
        "    padding: 5px;"
        "}"
        
        "QMenuBar::item:selected {"
        "    background-color: #3498db;"
        "    border-radius: 3px;"
        "}"
        
        "QMenu {"
        "    background-color: white;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
        
        "QMenu::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border-radius: 3px;"
        "}"
        
        "QToolBar {"
        "    background-color: #ecf0f1;"
        "    border: none;"
        "    spacing: 5px;"
        "    padding: 5px;"
        "}"
        
        "QToolBar::separator {"
        "    background-color: #bdc3c7;"
        "    width: 2px;"
        "    margin: 5px;"
        "}"
    );
    
    if (addButton) {
        addButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #1e8449;"
            "}"
        );
    }
    
    if (editButton) {
        editButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #f39c12;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e67e22;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #d35400;"
            "}"
        );
    }
    
    if (deleteButton) {
        deleteButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #e74c3c;"
            "}"
            "QPushButton:hover {"
            "    background-color: #c0392b;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #a93226;"
            "}"
        );
    }
    
    if (completeButton) {
        completeButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #9b59b6;"
            "}"
            "QPushButton:hover {"
            "    background-color: #8e44ad;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #7d3c98;"
            "}"
        );
    }
}
