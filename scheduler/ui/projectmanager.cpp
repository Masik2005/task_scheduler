#include "projectmanager.h"
#include "../data/taskservice.h"
#include "../models/project.h"
#include "../models/task.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>

ProjectManagerDialog::ProjectManagerDialog(TaskService *taskManager, QWidget *parent)
    : QDialog(parent), m_taskService(taskManager), m_currentProject(nullptr)
{
    setWindowTitle("Управление проектами");
    setModal(true);
    resize(700, 550);
    setupUI();
    refreshProjectList();
    applyStyles();
}

void ProjectManagerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_projectList = new QListWidget(this);
    connect(m_projectList, &QListWidget::itemClicked, this, &ProjectManagerDialog::onProjectSelected);
    mainLayout->addWidget(m_projectList);

    QFormLayout *formLayout = new QFormLayout();
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Введите название проекта");
    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("Введите описание (необязательно)");
    formLayout->addRow("Имя:", m_nameEdit);
    formLayout->addRow("Описание:", m_descriptionEdit);
    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Добавить", this);
    m_editButton = new QPushButton("Изменить", this);
    m_deleteButton = new QPushButton("Удалить", this);
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    connect(m_addButton, &QPushButton::clicked, this, &ProjectManagerDialog::onAddProject);
    connect(m_editButton, &QPushButton::clicked, this, &ProjectManagerDialog::onEditProject);
    connect(m_deleteButton, &QPushButton::clicked, this, &ProjectManagerDialog::onDeleteProject);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    QPushButton *closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton);
}

void ProjectManagerDialog::refreshProjectList()
{
    m_projectList->clear();
    if (m_taskService) {
        for (Project *project : m_taskService->getAllProjects()) {
            QListWidgetItem *item = new QListWidgetItem(project->getName());
            item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(project)));
            m_projectList->addItem(item);
        }
    }
}

void ProjectManagerDialog::onAddProject()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название проекта");
        return;
    }
    if (m_taskService && m_taskService->findProjectByName(name)) {
        QMessageBox::warning(this, "Ошибка", "Проект с таким названием уже существует");
        return;
    }
    QString desc = m_descriptionEdit->text().trimmed();
    Project *project = new Project(name, desc);
    if (m_taskService) {
        m_taskService->addProject(project);
    }
    refreshProjectList();
    m_nameEdit->clear();
    m_descriptionEdit->clear();
    m_currentProject = nullptr;
    m_editButton->setEnabled(false);
}

void ProjectManagerDialog::onEditProject()
{
    if (!m_currentProject || !m_taskService) return;
    
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название проекта");
        return;
    }
    
    Project *existingProject = m_taskService->findProjectByName(name);
    if (existingProject && existingProject != m_currentProject) {
        QMessageBox::warning(this, "Ошибка", "Проект с таким названием уже существует");
        return;
    }
    
    QString desc = m_descriptionEdit->text().trimmed();
    m_currentProject->setName(name);
    m_currentProject->setDescription(desc);
    
    refreshProjectList();
    m_nameEdit->clear();
    m_descriptionEdit->clear();
    m_currentProject = nullptr;
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
}

void ProjectManagerDialog::onDeleteProject()
{
    QListWidgetItem *item = m_projectList->currentItem();
    if (!item) return;
    Project *project = static_cast<Project*>(item->data(Qt::UserRole).value<void*>());
    if (!project || !m_taskService) return;

    // Проверяем наличие связанных задач через TaskService (актуальные данные)
    QList<Task*> tasks = m_taskService->filterByProject(project);
    if (!tasks.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя удалить проект с привязанными задачами");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение",
                              QString("Удалить проект '%1'?").arg(project->getName())) == QMessageBox::Yes) {
        m_taskService->removeProject(project);
        delete project;
        refreshProjectList();
        m_deleteButton->setEnabled(false);
        m_editButton->setEnabled(false);
        m_nameEdit->clear();
        m_descriptionEdit->clear();
        m_currentProject = nullptr;
    }
}

void ProjectManagerDialog::onProjectSelected(QListWidgetItem *item)
{
    bool hasSelection = item != nullptr;
    m_deleteButton->setEnabled(hasSelection);
    m_editButton->setEnabled(hasSelection);
    
    if (item) {
        m_currentProject = static_cast<Project*>(item->data(Qt::UserRole).value<void*>());
        if (m_currentProject) {
            m_nameEdit->setText(m_currentProject->getName());
            m_descriptionEdit->setText(m_currentProject->getDescription());
        }
    } else {
        m_currentProject = nullptr;
        m_nameEdit->clear();
        m_descriptionEdit->clear();
    }
}

void ProjectManagerDialog::applyStyles()
{
    setStyleSheet(
        "QDialog {"
        "    background-color: #f5f5f5;"
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
        "    background-color: white;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #3498db;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
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
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "}"
    );

    if (m_addButton) {
        m_addButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
        );
    }

    if (m_editButton) {
        m_editButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #f39c12;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e67e22;"
            "}"
        );
    }

    if (m_deleteButton) {
        m_deleteButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #e74c3c;"
            "}"
            "QPushButton:hover {"
            "    background-color: #c0392b;"
            "}"
        );
    }
}



