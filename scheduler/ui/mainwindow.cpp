#include "mainwindow.h"
#include "taskeditor.h"
#include "usermanager.h"
#include "projectmanager.h"
#include "tasklistwidget.h"
#include "appstyles.h"
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
    
    // Создаем репозитории и сервис
    // Используем интерфейсы (ITaskRepository, IUserRepository, IProjectRepository) для соблюдения DIP
    ITaskRepository *taskRepo = new TaskRepository(this);
    IUserRepository *userRepo = new UserRepository(this);
    IProjectRepository *projectRepo = new ProjectRepository(this);
    m_taskService = new TaskService(taskRepo, userRepo, projectRepo, this);
    
    // Создаем менеджеры
    m_commandManager = new CommandManager(this);
    m_reminderManager = new ReminderManager(m_taskService, this);
    
    // Настраиваем системный трей для уведомлений
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        m_trayIcon->setToolTip("Планировщик задач");
        m_trayIcon->show();
    }
    
    // Подключаем сигналы для связи компонентов
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
    
    // Загружаем данные или создаем тестовые
    bool dataLoaded = loadData();

    if (!dataLoaded) {
        // Инициализируем тестовые данные через TaskService
        if (m_taskService) {
            m_taskService->initializeDefaultData();
        }
        refreshTaskList();
    }
    
    // Восстанавливаем напоминания для всех незавершенных задач
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
    // Двойной клик обрабатывается через сигнал TaskListWidget
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
    
    // Подключаем кнопки к методам TaskListWidget
    if (addButton) {
        connect(addButton, &QPushButton::clicked, m_taskList, &TaskListWidget::addTask);
    }
    if (editButton) {
        connect(editButton, &QPushButton::clicked, m_taskList, &TaskListWidget::editTask);
    }
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, m_taskList, &TaskListWidget::deleteTask);
    }
    if (completeButton) {
        connect(completeButton, &QPushButton::clicked, m_taskList, &TaskListWidget::completeTask);
    }
    
    // Устанавливаем зависимости для TaskListWidget
    m_taskList->setDependencies(m_taskService, m_commandManager, m_reminderManager);
    
    // Подключаем сигналы TaskListWidget для обновления UI
    connect(m_taskList, &TaskListWidget::taskListChanged, this, &MainWindow::refreshTaskList);
    connect(m_taskList, &TaskListWidget::taskDoubleClicked, this, [this]() {
        if (m_taskList) {
            m_taskList->editTask();
        }
    });
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
    toolBar->addAction("Добавить", m_taskList, &TaskListWidget::addTask);
    toolBar->addAction("Редактировать", m_taskList, &TaskListWidget::editTask);
    toolBar->addAction("Удалить", m_taskList, &TaskListWidget::deleteTask);
    toolBar->addSeparator();
    toolBar->addAction(m_undoAction);
    toolBar->addAction(m_redoAction);
}

void MainWindow::setupStatusBar()
{
    updateStatusBar();
}

// Применяет фильтры и сортировку, затем отображает задачи в списке
// Делегирует всю логику в TaskListWidget
void MainWindow::updateTaskList()
{
    if (!m_taskList || !m_taskService) return;
    
    // Проверяем наличие всех необходимых UI элементов
    if (!searchEdit || !priorityFilter || !projectFilter || !userFilter || !sortCombo) {
        return;
    }
    
    // Делегируем формирование фильтров и обновление списка в TaskListWidget
    m_taskList->refreshTaskListWithFilters(searchEdit, priorityFilter, projectFilter, 
                                            userFilter, dateFilter, m_dateFilterEnabled,
                                            showCompletedCheckBox, sortCombo);
    
    updateCompleteButtonText();
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
    showUserManagerDialog();
}

void MainWindow::onProjectManager()
{
    showProjectManagerDialog();
}

void MainWindow::showUserManagerDialog()
{
    UserManagerDialog *dialog = new UserManagerDialog(m_taskService, this);
    dialog->exec();
    delete dialog;
    refreshTaskList();
}

void MainWindow::showProjectManagerDialog()
{
    ProjectManagerDialog *dialog = new ProjectManagerDialog(m_taskService, this);
    dialog->exec();
    delete dialog;
    refreshTaskList();
}

// Импорт задач из JSON файла
// Использует TaskService для работы с файлом, затем создает команды для поддержки undo/redo
void MainWindow::onImportTasks()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Импорт задач", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    
    int totalTasks = m_taskService->getAllTasks().size();
    
    // Импортируем через TaskService (создает пользователей/проекты при необходимости)
    int imported = m_taskService->importTasksFromFile(fileName, true);
    
    if (imported < 0) {
        QMessageBox::critical(this, "Ошибка", "Не удалось импортировать файл");
        return;
    }
    
    // Выполняем импорт через команды для поддержки undo/redo
    QList<Task*> allTasks = m_taskService->getAllTasks();
    for (int i = totalTasks; i < allTasks.size(); ++i) {
        AddTaskCommand *cmd = new AddTaskCommand(m_taskService, allTasks[i]);
        m_commandManager->executeCommand(cmd);
        // Создаем напоминания для импортированных задач
        if (m_reminderManager && allTasks[i]) {
            m_reminderManager->addReminder(allTasks[i], allTasks[i]->getReminderMinutes());
        }
    }
    
    // Получаем информацию о пропущенных дубликатах из исходного файла
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isArray()) {
            int skipped = doc.array().size() - imported;
            QString message = QString("Импортировано задач: %1").arg(imported);
            if (skipped > 0) {
                message += QString("\nПропущено дубликатов: %1").arg(skipped);
            }
            QMessageBox::information(this, "Импорт", message);
        }
    }
    
    refreshTaskList();
}

void MainWindow::onExportTasks()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт задач", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    
    if (!m_taskService->exportTasksToFile(fileName)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
        return;
    }
    
    int taskCount = m_taskService->getAllTasks().size();
    QMessageBox::information(this, "Экспорт", QString("Экспортировано задач: %1").arg(taskCount));
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

// Обновляет списки проектов и пользователей в фильтрах, затем обновляет список задач
void MainWindow::refreshTaskList()
{
    if (!m_taskService || !m_taskList) {
        return;
    }
    
    // Делегируем обновление фильтров в TaskListWidget
    if (projectFilter && userFilter) {
        m_taskList->updateFilters(m_taskService, projectFilter, userFilter);
    }
    
    updateTaskList();
    updateStatusBar();
}

// Обновляет статус-бар со статистикой задач
void MainWindow::updateStatusBar()
{
    if (!m_taskService) {
        return;
    }
    
    // Получаем статистику из TaskService
    TaskService::TaskStatistics stats = m_taskService->getStatistics();
    QMainWindow::statusBar()->showMessage(QString("Всего задач: %1 | Завершено: %2 | Активных: %3")
                           .arg(stats.total).arg(stats.completed).arg(stats.active));
}

void MainWindow::updateCompleteButtonText()
{
    if (!completeButton) return;
    
    Task *task = m_taskList ? m_taskList->getSelectedTask() : nullptr;
    if (task) {
        if (task->isCompleted()) {
            completeButton->setText("Возобновить");
            completeButton->setStyleSheet(AppStyles::getButtonStyles("resume"));
        } else {
            completeButton->setText("Завершить");
            completeButton->setStyleSheet(AppStyles::getButtonStyles("complete"));
        }
    } else {
        completeButton->setText("Завершить");
        completeButton->setStyleSheet(AppStyles::getButtonStyles("complete"));
    }
}

// Сохранение всех данных в JSON файл (вызывается при закрытии приложения)
// Сохранение данных - делегируется в TaskService
void MainWindow::saveData()
{
    if (m_taskService) {
        m_taskService->saveToFile();
    }
}

// Загрузка данных - делегируется в TaskService
bool MainWindow::loadData()
{
    if (!m_taskService) return false;
    
    bool loaded = m_taskService->loadFromFile();
    
    if (loaded) {
        refreshTaskList();
        updateTaskList();
        updateStatusBar();
    }
    
    return loaded;
}

// Автоматическое сохранение данных при закрытии приложения
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    QMainWindow::closeEvent(event);
}

void MainWindow::applyStyles()
{
    setStyleSheet(AppStyles::getMainWindowStyles() + AppStyles::getButtonStyles(""));
    
    if (addButton) {
        addButton->setStyleSheet(AppStyles::getButtonStyles("add"));
    }
    
    if (editButton) {
        editButton->setStyleSheet(AppStyles::getButtonStyles("edit"));
    }
    
    if (deleteButton) {
        deleteButton->setStyleSheet(AppStyles::getButtonStyles("delete"));
    }
    
    if (completeButton) {
        completeButton->setStyleSheet(AppStyles::getButtonStyles("complete"));
    }
}
