#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QDialog>
#include <QListWidget>

class TaskService;
class User;
class QLineEdit;
class QPushButton;
class QListWidget;

// Диалог управления пользователями
// Использует TaskService для проверки связанных задач при удалении
class UserManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagerDialog(TaskService *taskService, QWidget *parent = nullptr);

private slots:
    void onAddUser();
    void onEditUser();
    void onDeleteUser();
    void onUserSelected(QListWidgetItem *item);

private:
    void setupUI();
    void refreshUserList();
    void applyStyles();
    
    TaskService *m_taskService;
    QListWidget *m_userList;
    QLineEdit *m_nameEdit;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    User *m_currentUser;
};

#endif // USERMANAGER_H

