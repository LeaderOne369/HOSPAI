#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDateTime>
#include <QCryptographicHash>

struct UserInfo {
    int id;
    QString username;
    QString email;
    QString phone;
    QString role;
    QString realName;
    QDateTime createdAt;
    QDateTime lastLogin;
    int status;
    QString avatarPath;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    
    // 数据库初始化
    bool initDatabase();
    void closeDatabase();
    
    // 用户管理
    bool registerUser(const QString& username, const QString& password, 
                     const QString& email, const QString& phone, 
                     const QString& role, const QString& realName = "");
    bool loginUser(const QString& username, const QString& password, UserInfo& userInfo);
    bool updateLastLogin(int userId);
    bool isUsernameExists(const QString& username);
    bool isEmailExists(const QString& email);
    
    // 密码处理
    QString hashPassword(const QString& password);
    bool verifyPassword(const QString& password, const QString& hash);
    
    // 用户信息
    UserInfo getUserInfo(int userId);
    bool updateUserInfo(const UserInfo& userInfo);
    bool changePassword(int userId, const QString& oldPassword, const QString& newPassword);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    QString getDbPath();
    
    static DatabaseManager* m_instance;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H 