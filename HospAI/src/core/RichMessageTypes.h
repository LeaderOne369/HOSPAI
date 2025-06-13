#ifndef RICHMESSAGETYPES_H
#define RICHMESSAGETYPES_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QMap>
#include <QVariant>

// 富文本消息内容类型
enum class RichContentType {
    Text,           // 普通文本
    RichText,       // 富文本
    Image,          // 图片
    File,           // 文件
    Mixed           // 混合内容
};

// 消息类型枚举 (如果在其他地方没有定义)
enum class MessageType {
    User,           // 用户消息
    Robot,          // 机器人消息
    System,         // 系统消息
    QuickReply      // 快捷回复
};

// 富文本消息结构体（患者端使用）
struct RichMessage {
    QString content;        // 文本内容
    QString htmlContent;    // HTML格式内容
    MessageType type;
    RichContentType contentType;
    QDateTime timestamp;
    QString sessionId;
    QStringList attachments;  // 附件路径列表
    QMap<QString, QVariant> metadata; // 元数据（如字体、颜色等）
};

// 富文本聊天消息结构体（客服端使用）
struct RichChatMessage {
    int id;
    int sessionId;
    int senderId;
    QString senderName;
    QString senderRole;
    QString content;        // 纯文本内容
    QString htmlContent;    // HTML格式内容
    RichContentType contentType;
    QDateTime timestamp;
    int messageType;
    int isRead;
    QStringList attachments;  // 附件路径列表
    QMap<QString, QVariant> metadata; // 元数据
};

#endif // RICHMESSAGETYPES_H 