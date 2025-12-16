#ifndef PROJECTREPOSITORY_H
#define PROJECTREPOSITORY_H

#include "repositories.h"
#include <QObject>

class ProjectRepository : public QObject, public IProjectRepository
{
    Q_OBJECT

public:
    explicit ProjectRepository(QObject *parent = nullptr);
    
    // IRepository interface
    void add(Project *project) override;
    void remove(Project *project) override;
    QList<Project*> getAll() const override { return m_projects; }
    Project* findById(int id) const override;
    void clear() override;
    
    // IProjectRepository interface
    Project* findByName(const QString &name) const override;
    
    int getNextId() { return m_nextProjectId++; }
    void setNextId(int id) { m_nextProjectId = id; }

private:
    QList<Project*> m_projects;
    int m_nextProjectId;
};

#endif // PROJECTREPOSITORY_H



