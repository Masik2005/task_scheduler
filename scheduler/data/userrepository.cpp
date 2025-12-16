#include "userrepository.h"
#include "../models/user.h"

UserRepository::UserRepository(QObject *parent)
    : QObject(parent), m_nextUserId(1)
{
}

void UserRepository::add(User *user)
{
    if (user && !m_users.contains(user)) {
        if (user->getId() < 0) {
            user->setId(m_nextUserId++);
        }
        m_users.append(user);
    }
}

void UserRepository::remove(User *user)
{
    m_users.removeAll(user);
}

User* UserRepository::findById(int id) const
{
    for (User *user : m_users) {
        if (user->getId() == id) {
            return user;
        }
    }
    return nullptr;
}

void UserRepository::clear()
{
    m_users.clear();
    m_nextUserId = 1;
}

User* UserRepository::findByName(const QString &name) const
{
    for (User *user : m_users) {
        if (user->getName() == name) {
            return user;
        }
    }
    return nullptr;
}



