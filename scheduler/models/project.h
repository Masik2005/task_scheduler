#ifndef PROJECT_H
#define PROJECT_H

#include <QString>

// Модель проекта - простая сущность с названием и описанием
// Связь с задачами односторонняя (задача знает о проекте)
class Project
{
public:
    Project(const QString &name, const QString &description = "", int id = -1);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    QString getName() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    
    QString getDescription() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

private:
    int m_id;
    QString m_name;
    QString m_description;
};

#endif // PROJECT_H


