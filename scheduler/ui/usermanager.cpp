#include "usermanager.h"
#include "../data/taskservice.h"
#include "../models/user.h"
#include "appstyles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

UserManagerDialog::UserManagerDialog(TaskService *taskManager, QWidget *parent)
    : QDialog(parent), m_taskService(taskManager), m_currentUser(nullptr)
{
    setWindowTitle("Управление пользователями");
    setModal(true);
    resize(700, 500);
    setupUI();
    refreshUserList();
    applyStyles();
}

void UserManagerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_userList = new QListWidget(this);
    connect(m_userList, &QListWidget::itemClicked, this, &UserManagerDialog::onUserSelected);
    mainLayout->addWidget(m_userList);
    
    QFormLayout *formLayout = new QFormLayout();
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Введите имя пользователя");
    formLayout->addRow("Имя:", m_nameEdit);
    mainLayout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Добавить", this);
    m_editButton = new QPushButton("Изменить", this);
    m_deleteButton = new QPushButton("Удалить", this);
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    
    connect(m_addButton, &QPushButton::clicked, this, &UserManagerDialog::onAddUser);
    connect(m_editButton, &QPushButton::clicked, this, &UserManagerDialog::onEditUser);
    connect(m_deleteButton, &QPushButton::clicked, this, &UserManagerDialog::onDeleteUser);
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    QPushButton *closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton);
}

void UserManagerDialog::refreshUserList()
{
    m_userList->clear();
    if (m_taskService) {
        for (User *user : m_taskService->getAllUsers()) {
            QListWidgetItem *item = new QListWidgetItem(user->getName());
            item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(user)));
            m_userList->addItem(item);
        }
    }
}

void UserManagerDialog::onAddUser()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя");
        return;
    }
    
    if (m_taskService && m_taskService->findUserByName(name)) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким именем уже существует");
        return;
    }
    
    if (m_taskService) {
        User *user = new User(name);
        m_taskService->addUser(user);
        refreshUserList();
        m_nameEdit->clear();
        m_currentUser = nullptr;
        m_editButton->setEnabled(false);
    }
}

void UserManagerDialog::onEditUser()
{
    if (!m_currentUser || !m_taskService) return;
    
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя");
        return;
    }
    
    User *existingUser = m_taskService->findUserByName(name);
    if (existingUser && existingUser != m_currentUser) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким именем уже существует");
        return;
    }
    
    m_currentUser->setName(name);
    
    refreshUserList();
    m_nameEdit->clear();
    m_currentUser = nullptr;
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
}

void UserManagerDialog::onDeleteUser()
{
    QListWidgetItem *item = m_userList->currentItem();
    if (!item) return;
    
    User *user = static_cast<User*>(item->data(Qt::UserRole).value<void*>());
    if (!user || !m_taskService) return;
    
    // Проверяем наличие связанных задач через TaskService (актуальные данные)
    QList<Task*> userTasks = m_taskService->filterByUser(user);
    if (!userTasks.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", 
                           "Нельзя удалить пользователя, у которого есть задачи");
        return;
    }
    
    if (QMessageBox::question(this, "Подтверждение",
                              QString("Удалить пользователя '%1'?").arg(user->getName())) == QMessageBox::Yes) {
        m_taskService->removeUser(user);
        delete user;
        refreshUserList();
        m_deleteButton->setEnabled(false);
        m_editButton->setEnabled(false);
        m_nameEdit->clear();
        m_currentUser = nullptr;
    }
}

void UserManagerDialog::onUserSelected(QListWidgetItem *item)
{
    bool hasSelection = item != nullptr;
    m_deleteButton->setEnabled(hasSelection);
    m_editButton->setEnabled(hasSelection);
    
    if (item) {
        m_currentUser = static_cast<User*>(item->data(Qt::UserRole).value<void*>());
        if (m_currentUser) {
            m_nameEdit->setText(m_currentUser->getName());
        }
    } else {
        m_currentUser = nullptr;
        m_nameEdit->clear();
    }
}

void UserManagerDialog::applyStyles()
{
    setStyleSheet(AppStyles::getDialogStyles());
    
    if (m_addButton) {
        m_addButton->setStyleSheet(AppStyles::getButtonStyles("add"));
    }
    
    if (m_editButton) {
        m_editButton->setStyleSheet(AppStyles::getButtonStyles("edit"));
    }

    if (m_deleteButton) {
        m_deleteButton->setStyleSheet(AppStyles::getButtonStyles("delete"));
    }
}

