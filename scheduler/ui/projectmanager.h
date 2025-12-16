#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDialog>
#include <QListWidget>

class TaskService;
class Project;
class QLineEdit;
class QPushButton;

class ProjectManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectManagerDialog(TaskService *taskService, QWidget *parent = nullptr);

private slots:
    void onAddProject();
    void onEditProject();
    void onDeleteProject();
    void onProjectSelected(QListWidgetItem *item);

private:
    void setupUI();
    void refreshProjectList();
    void applyStyles();

    TaskService *m_taskService;
    QListWidget *m_projectList;
    QLineEdit *m_nameEdit;
    QLineEdit *m_descriptionEdit;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    Project *m_currentProject;
};

#endif // PROJECTMANAGER_H




