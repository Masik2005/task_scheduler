#include "taskeditor.h"
#include "../data/taskservice.h"
#include "../models/task.h"
#include "../models/project.h"
#include "../models/user.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialog>
#include <QDateTime>
#include <QDate>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

TaskEditorDialog::TaskEditorDialog(TaskService *taskManager, QWidget *parent)
    : QDialog(parent), m_taskService(taskManager), m_task(nullptr), m_isEditMode(false)
{
    setWindowTitle("Новая задача");
    setModal(true);
    resize(600, 500);
    setupUI();
    applyStyles();
}

void TaskEditorDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("Введите название задачи");
    formLayout->addRow("Название:", m_titleEdit);
    
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("Описание:", m_descriptionEdit);
    
    m_deadlineEdit = new QDateTimeEdit(this);
    m_deadlineEdit->setCalendarPopup(true);
    m_deadlineEdit->setDateTime(QDateTime::currentDateTime().addDays(1));
    m_deadlineEdit->setMinimumDate(QDate(2000, 1, 1));
    m_deadlineEdit->setMaximumDate(QDate(2100, 12, 31));
    formLayout->addRow("Дедлайн:", m_deadlineEdit);
    
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem("Низкий", static_cast<int>(Priority::Low));
    m_priorityCombo->addItem("Средний", static_cast<int>(Priority::Medium));
    m_priorityCombo->addItem("Высокий", static_cast<int>(Priority::High));
    m_priorityCombo->setCurrentIndex(1);
    formLayout->addRow("Приоритет:", m_priorityCombo);
    
    m_projectCombo = new QComboBox(this);
    m_projectCombo->addItem("Без проекта", -1);
    formLayout->addRow("Проект:", m_projectCombo);
    
    m_userCombo = new QComboBox(this);
    formLayout->addRow("Владелец:", m_userCombo);
    
    m_reminderMinutes = new QSpinBox(this);
    m_reminderMinutes->setRange(2, 10080);
    m_reminderMinutes->setValue(10);
    m_reminderMinutes->setSuffix(" минут до дедлайна");
    formLayout->addRow("Напоминание:", m_reminderMinutes);
    
    mainLayout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Отмена", this);
    
    connect(m_okButton, &QPushButton::clicked, this, &TaskEditorDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &TaskEditorDialog::onCancel);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    if (m_taskService) {
        for (Project *project : m_taskService->getAllProjects()) {
            m_projectCombo->addItem(project->getName(), project->getId());
        }
        
        for (User *user : m_taskService->getAllUsers()) {
            m_userCombo->addItem(user->getName(), user->getId());
        }
    }
}

void TaskEditorDialog::setTask(Task *task)
{
    m_task = task;
    m_isEditMode = (task != nullptr);
    setWindowTitle(m_isEditMode ? "Редактирование задачи" : "Новая задача");
    loadTaskData();
}

void TaskEditorDialog::loadTaskData()
{
    if (m_isEditMode && m_task) {
        m_titleEdit->setText(m_task->getTitle());
        m_descriptionEdit->setPlainText(m_task->getDescription());
        m_deadlineEdit->setDateTime(m_task->getDeadline());
        
        int priorityIndex = static_cast<int>(m_task->getPriority());
        m_priorityCombo->setCurrentIndex(priorityIndex);
        
        if (m_task->getProject()) {
            int index = m_projectCombo->findData(m_task->getProject()->getId());
            if (index >= 0) {
                m_projectCombo->setCurrentIndex(index);
            }
        }
        
        if (m_task->getOwner()) {
            int index = m_userCombo->findData(m_task->getOwner()->getId());
            if (index >= 0) {
                m_userCombo->setCurrentIndex(index);
            }
        }

        m_reminderMinutes->setValue(qMax(2, m_task->getReminderMinutes()));
    }
}

void TaskEditorDialog::onAccept()
{
    if (m_titleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название задачи");
        return;
    }
    
    if (m_userCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите владельца задачи");
        return;
    }
    
    Priority priority = static_cast<Priority>(m_priorityCombo->currentData().toInt());
    QDateTime deadline = m_deadlineEdit->dateTime();
    
    User *owner = nullptr;
    if (m_taskService && m_userCombo->currentIndex() >= 0) {
        int userId = m_userCombo->currentData().toInt();
        owner = m_taskService->findUserById(userId);
    }
    
    Project *project = nullptr;
    if (m_projectCombo->currentIndex() > 0 && m_taskService) {
        int projectId = m_projectCombo->currentData().toInt();
        project = m_taskService->findProjectById(projectId);
    }
    
    if (m_isEditMode && m_task) {
        m_task->setTitle(m_titleEdit->text());
        m_task->setDescription(m_descriptionEdit->toPlainText());
        m_task->setDeadline(deadline);
        m_task->setPriority(priority);
        m_task->setOwner(owner);
        m_task->setProject(project);
        m_task->setReminderMinutes(m_reminderMinutes->value());
    } else {
        m_task = new Task(m_titleEdit->text(), deadline, priority, owner, project, -1, m_reminderMinutes->value());
        m_task->setDescription(m_descriptionEdit->toPlainText());
        m_task->setReminderMinutes(m_reminderMinutes->value());
    }
    
    accept();
}

void TaskEditorDialog::onCancel()
{
    reject();
}

void TaskEditorDialog::applyStyles()
{
    setStyleSheet(
        "QDialog {"
        "    background-color: #f5f5f5;"
        "}"
        
        "QLineEdit, QTextEdit, QDateTimeEdit, QComboBox, QSpinBox {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "}"
        
        "QLineEdit:focus, QTextEdit:focus, QDateTimeEdit:focus, QComboBox:focus, QSpinBox:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #ecf0f1;"
        "}"
        
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 25px;"
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
        
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "}"
    );
    
    if (m_okButton) {
        m_okButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
        );
    }
    
    if (m_cancelButton) {
        m_cancelButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #95a5a6;"
            "}"
            "QPushButton:hover {"
            "    background-color: #7f8c8d;"
            "}"
        );
    }
}

