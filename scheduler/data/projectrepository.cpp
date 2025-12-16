#include "projectrepository.h"
#include "../models/project.h"

ProjectRepository::ProjectRepository(QObject *parent)
    : QObject(parent), m_nextProjectId(1)
{
}

void ProjectRepository::add(Project *project)
{
    if (project && !m_projects.contains(project)) {
        if (project->getId() < 0) {
            project->setId(m_nextProjectId++);
        }
        m_projects.append(project);
    }
}

void ProjectRepository::remove(Project *project)
{
    m_projects.removeAll(project);
}

Project* ProjectRepository::findById(int id) const
{
    for (Project *project : m_projects) {
        if (project->getId() == id) {
            return project;
        }
    }
    return nullptr;
}

void ProjectRepository::clear()
{
    m_projects.clear();
    m_nextProjectId = 1;
}

Project* ProjectRepository::findByName(const QString &name) const
{
    for (Project *project : m_projects) {
        if (project->getName() == name) {
            return project;
        }
    }
    return nullptr;
}



