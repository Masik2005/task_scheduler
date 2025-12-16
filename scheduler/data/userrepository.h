#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "repositories.h"
#include <QObject>

class UserRepository : public QObject, public IUserRepository
{
    Q_OBJECT

public:
    explicit UserRepository(QObject *parent = nullptr);
    
    // IRepository interface
    void add(User *user) override;
    void remove(User *user) override;
    QList<User*> getAll() const override { return m_users; }
    User* findById(int id) const override;
    void clear() override;
    
    // IUserRepository interface
    User* findByName(const QString &name) const override;
    
    int getNextId() { return m_nextUserId++; }
    void setNextId(int id) { m_nextUserId = id; }

private:
    QList<User*> m_users;
    int m_nextUserId;
};

#endif // USERREPOSITORY_H



