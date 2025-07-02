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
    QString userId;      // 用户ID字符串
    QString username;
    QString email;
    QString phone;
    QString role;
    QString realName;
    QString name;        // 显示名称，通常等于realName
    QDateTime createdAt;
    QDateTime lastLogin;
    QDateTime lastLoginTime; // 别名，指向lastLogin
    int status;
    bool isActive;       // 是否活跃，从status转换而来
    QString avatarPath;
};

// 聊天会话信息
struct ChatSession {
    int id;
    int patientId;
    int staffId;
    QString patientName;
    QString staffName;
    QDateTime createdAt;
    QDateTime lastMessageAt;
    int status; // 0-已结束, 1-进行中, 2-等待中
    QString lastMessage;
};

// 聊天消息信息
struct ChatMessage {
    int id;
    int sessionId;
    int senderId;
    QString senderName;
    QString senderRole;
    QString content;
    QDateTime timestamp;
    int messageType; // 0-普通消息, 1-系统消息, 2-图片, 3-文件
    int isRead; // 0-未读, 1-已读
};

// 会话评价信息
struct SessionRating {
    int id;
    int sessionId;
    QString patientId;   // 患者ID字符串
    QString staffId;     // 客服ID字符串
    int rating; // 1-5星评价
    QString comment;
    QDateTime createdAt;
    QDateTime ratingTime; // 别名，指向createdAt
};

// 快捷回复信息
struct QuickReply {
    int id;
    QString title;
    QString content;
    QString category;
    int sortOrder;
    bool isActive;
    QDateTime createdAt;
    QDateTime updatedAt;
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
    
    // 忘记密码相关
    UserInfo getUserByEmail(const QString& email);
    UserInfo getUserByUsername(const QString& username);
    bool resetPassword(const QString& email, const QString& newPassword);
    bool resetPasswordByUsername(const QString& username, const QString& newPassword);
    
    // 聊天会话管理
    int createChatSession(int patientId, int staffId = 0);
    bool updateChatSession(int sessionId, int staffId);
    bool closeChatSession(int sessionId);
    QList<ChatSession> getActiveSessions();
    QList<ChatSession> getPatientSessions(int patientId);
    QList<ChatSession> getStaffSessions(int staffId);
    ChatSession getChatSession(int sessionId);
    
    // 聊天消息管理
    int sendMessage(int sessionId, int senderId, const QString& content, int messageType = 0);
    QList<ChatMessage> getChatMessages(int sessionId, int limit = 50);
    QList<ChatMessage> getUnreadMessages(int userId);
    bool markMessageAsRead(int messageId);
    bool markSessionAsRead(int sessionId, int userId);
    
    // 在线状态管理
    bool updateUserOnlineStatus(int userId, bool isOnline);
    QList<UserInfo> getOnlineStaff();
    
    // 获取用户列表
    QList<UserInfo> getAllUsers();
    QList<UserInfo> getUsersByRole(const QString& role);
    
    // 会话评价管理
    bool addSessionRating(int sessionId, int patientId, int staffId, int rating, const QString& comment = "");
    SessionRating getSessionRating(int sessionId);
    QList<SessionRating> getStaffRatings(int staffId);
    QList<SessionRating> getAllSessionRatings();  // 新增方法
    double getStaffAverageRating(int staffId);
    bool hasSessionRating(int sessionId);
    
    // 快捷回复管理
    QList<QuickReply> getAllQuickReplies();
    QList<QuickReply> getQuickRepliesByCategory(const QString& category);
    QList<QuickReply> getActiveQuickReplies();
    bool addQuickReply(const QString& title, const QString& content, const QString& category, int sortOrder = 0);
    bool updateQuickReply(int id, const QString& title, const QString& content, const QString& category, int sortOrder);
    bool deleteQuickReply(int id);
    bool toggleQuickReplyStatus(int id);

signals:
    // 聊天相关信号
    void newMessageReceived(const ChatMessage& message);
    void sessionCreated(const ChatSession& session);
    void sessionUpdated(const ChatSession& session);
    void userOnlineStatusChanged(int userId, bool isOnline);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    QString getDbPath();
    
    static DatabaseManager* m_instance;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H 