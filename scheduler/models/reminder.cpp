#include "reminder.h"
#include "task.h"

Reminder::Reminder(Task *task, int minutesBeforeDeadline, QObject *parent)
    : QObject(parent), m_task(task), m_minutesBeforeDeadline(minutesBeforeDeadline)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Reminder::onTimerTimeout);
    
    if (m_task) {
        m_reminderTime = m_task->getDeadline().addSecs(-minutesBeforeDeadline * 60);
    }
}

Reminder::~Reminder()
{
    if (m_timer) {
        m_timer->stop();
    }
}

QDateTime Reminder::getReminderTime() const
{
    if (m_task) {
        return m_task->getDeadline().addSecs(-m_minutesBeforeDeadline * 60);
    }
    return QDateTime();
}

// Активирует напоминание - вычисляет время до срабатывания и запускает таймер
// Если время уже прошло - эмитирует сигнал сразу
void Reminder::activate()
{
    if (!m_task || m_task->isCompleted() || !m_timer) {
        return;
    }
    
    QDateTime deadline = m_task->getDeadline();
    if (!deadline.isValid()) {
        return;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    
    // Если дедлайн уже прошел - показываем напоминание сразу
    if (deadline <= now) {
        QTimer::singleShot(0, this, [this]() {
            emit reminderTriggered(this);
        });
        return;
    }
    
    m_reminderTime = getReminderTime();
    if (!m_reminderTime.isValid()) {
        return;
    }
    
    // Если время напоминания уже прошло - показываем сразу
    if (m_reminderTime <= now) {
        QTimer::singleShot(0, this, [this]() {
            emit reminderTriggered(this);
        });
        return;
    }
    
    // Запускаем таймер на вычисленное время (ограничение QTimer::start)
    int msecs = now.msecsTo(m_reminderTime);
    if (msecs > 0 && msecs < 2147483647) {
        m_timer->start(msecs);
    }
}

void Reminder::deactivate()
{
    if (m_timer) {
        m_timer->stop();
    }
}

void Reminder::onTimerTimeout()
{
    if (m_task && !m_task->isCompleted()) {
        emit reminderTriggered(this);
    }
}

