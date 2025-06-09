#include "DatabaseManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlRecord>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager;
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::initDatabase()
{
    // 获取数据库路径
    QString dbPath = getDbPath();
    
    // 创建数据库连接
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qDebug() << "数据库打开失败:" << m_database.lastError().text();
        return false;
    }
    
    // 创建表
    if (!createTables()) {
        qDebug() << "创建数据表失败";
        return false;
    }
    
    qDebug() << "数据库初始化成功:" << dbPath;
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

QString DatabaseManager::getDbPath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dataPath + "/hospai.db";
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    
    // 创建用户表
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(64) NOT NULL,
            email VARCHAR(100) UNIQUE,
            phone VARCHAR(20),
            role VARCHAR(20) NOT NULL,
            real_name VARCHAR(50),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME,
            status INTEGER DEFAULT 1,
            avatar_path VARCHAR(255)
        )
    )";
    
    if (!query.exec(createUsersTable)) {
        qDebug() << "创建用户表失败:" << query.lastError().text();
        return false;
    }
    
    // 创建默认管理员账户（如果不存在）
    if (!isUsernameExists("admin")) {
        registerUser("admin", "admin123", "admin@hospai.com", "", "管理员", "系统管理员");
    }
    
    return true;
}

bool DatabaseManager::registerUser(const QString& username, const QString& password, 
                                 const QString& email, const QString& phone, 
                                 const QString& role, const QString& realName)
{
    // 检查用户名是否已存在
    if (isUsernameExists(username)) {
        return false;
    }
    
    // 检查邮箱是否已存在
    if (!email.isEmpty() && isEmailExists(email)) {
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO users (username, password_hash, email, phone, role, real_name)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    query.addBindValue(email);
    query.addBindValue(phone);
    query.addBindValue(role);
    query.addBindValue(realName);
    
    if (!query.exec()) {
        qDebug() << "用户注册失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::loginUser(const QString& username, const QString& password, UserInfo& userInfo)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name, created_at, last_login, status, avatar_path, password_hash
        FROM users 
        WHERE username = ? AND status = 1
    )");
    
    query.addBindValue(username);
    
    if (!query.exec()) {
        qDebug() << "登录查询失败:" << query.lastError().text();
        return false;
    }
    
    if (query.next()) {
        QString storedHash = query.value("password_hash").toString();
        
        // 验证密码
        if (!verifyPassword(password, storedHash)) {
            return false;
        }
        
        // 填充用户信息
        userInfo.id = query.value("id").toInt();
        userInfo.username = query.value("username").toString();
        userInfo.email = query.value("email").toString();
        userInfo.phone = query.value("phone").toString();
        userInfo.role = query.value("role").toString();
        userInfo.realName = query.value("real_name").toString();
        userInfo.createdAt = query.value("created_at").toDateTime();
        userInfo.lastLogin = query.value("last_login").toDateTime();
        userInfo.status = query.value("status").toInt();
        userInfo.avatarPath = query.value("avatar_path").toString();
        
        // 更新最后登录时间
        updateLastLogin(userInfo.id);
        
        return true;
    }
    
    return false;
}

bool DatabaseManager::updateLastLogin(int userId)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(userId);
    
    return query.exec();
}

bool DatabaseManager::isUsernameExists(const QString& username)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

bool DatabaseManager::isEmailExists(const QString& email)
{
    if (email.isEmpty()) return false;
    
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
    query.addBindValue(email);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

QString DatabaseManager::hashPassword(const QString& password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool DatabaseManager::verifyPassword(const QString& password, const QString& hash)
{
    return hashPassword(password) == hash;
}

UserInfo DatabaseManager::getUserInfo(int userId)
{
    UserInfo userInfo;
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name, created_at, last_login, status, avatar_path
        FROM users 
        WHERE id = ?
    )");
    
    query.addBindValue(userId);
    
    if (query.exec() && query.next()) {
        userInfo.id = query.value("id").toInt();
        userInfo.username = query.value("username").toString();
        userInfo.email = query.value("email").toString();
        userInfo.phone = query.value("phone").toString();
        userInfo.role = query.value("role").toString();
        userInfo.realName = query.value("real_name").toString();
        userInfo.createdAt = query.value("created_at").toDateTime();
        userInfo.lastLogin = query.value("last_login").toDateTime();
        userInfo.status = query.value("status").toInt();
        userInfo.avatarPath = query.value("avatar_path").toString();
    }
    
    return userInfo;
}

bool DatabaseManager::updateUserInfo(const UserInfo& userInfo)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE users 
        SET email = ?, phone = ?, real_name = ?, avatar_path = ?
        WHERE id = ?
    )");
    
    query.addBindValue(userInfo.email);
    query.addBindValue(userInfo.phone);
    query.addBindValue(userInfo.realName);
    query.addBindValue(userInfo.avatarPath);
    query.addBindValue(userInfo.id);
    
    return query.exec();
}

bool DatabaseManager::changePassword(int userId, const QString& oldPassword, const QString& newPassword)
{
    // 先验证旧密码
    QSqlQuery query(m_database);
    query.prepare("SELECT password_hash FROM users WHERE id = ?");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    QString storedHash = query.value(0).toString();
    if (!verifyPassword(oldPassword, storedHash)) {
        return false;
    }
    
    // 更新新密码
    query.prepare("UPDATE users SET password_hash = ? WHERE id = ?");
    query.addBindValue(hashPassword(newPassword));
    query.addBindValue(userId);
    
    return query.exec();
} 