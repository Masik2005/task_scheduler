#ifndef REMINDER_H
#define REMINDER_H

#include <QDateTime>
#include <QTimer>
#include <QObject>

class Task;

// Модель напоминания - использует QTimer для отсчета времени до дедлайна
// Эмитирует сигнал при срабатывании
class Reminder : public QObject
{
    Q_OBJECT

public:
    Reminder(Task *task, int minutesBeforeDeadline, QObject *parent = nullptr);
    ~Reminder();
    
    Task* getTask() const { return m_task; }
    int getMinutesBeforeDeadline() const { return m_minutesBeforeDeadline; }
    QDateTime getReminderTime() const;
    
    void activate();
    void deactivate();

signals:
    void reminderTriggered(Reminder *reminder);

private slots:
    void onTimerTimeout();

private:
    Task *m_task;
    int m_minutesBeforeDeadline;
    QTimer *m_timer;
    QDateTime m_reminderTime;
};

#endif // REMINDER_H





