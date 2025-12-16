#ifndef TASKEDITOR_H
#define TASKEDITOR_H

#include <QDialog>
#include "../models/task.h"

class TaskService;
class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QComboBox;
class QSpinBox;
class QPushButton;

// Диалог создания/редактирования задачи
// Поддерживает оба режима в одном классе
class TaskEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskEditorDialog(TaskService *taskService, QWidget *parent = nullptr);
    void setTask(Task *task);
    Task* getCreatedTask() const { return m_task; }

private slots:
    void onAccept();
    void onCancel();

private:
    void setupUI();
    void loadTaskData();
    void applyStyles();
    
    TaskService *m_taskService;
    Task *m_task;
    bool m_isEditMode;
    
    QLineEdit *m_titleEdit;
    QTextEdit *m_descriptionEdit;
    QDateTimeEdit *m_deadlineEdit;
    QComboBox *m_priorityCombo;
    QComboBox *m_projectCombo;
    QComboBox *m_userCombo;
    QSpinBox *m_reminderMinutes;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // TASKEDITOR_H

