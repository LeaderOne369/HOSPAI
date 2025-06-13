#ifndef REALCHATWIDGET_H
#define REALCHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDateTime>
#include <QToolBar>
#include <QAction>
#include <QColorDialog>
#include <QFontComboBox>
#include <QSpinBox>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QFileDialog>
#include <QTextBrowser>
#include "../../core/DatabaseManager.h"
#include "../../core/RichMessageTypes.h"

class RealChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealChatWidget(QWidget *parent = nullptr);
    ~RealChatWidget();
    
    void setCurrentUser(const UserInfo& user);
    void setDatabaseManager(DatabaseManager* dbManager);
    void startNewChat();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSendMessage();
    void onSendRichMessage();
    void onMessageReceived(const ChatMessage& message);
    void onRichMessageReceived(const RichChatMessage& message);
    void onSessionCreated(const ChatSession& session);
    void onSessionUpdated(const ChatSession& session);
    void checkForNewMessages();
    void onUserInput();
    
    // 富文本功能槽函数
    void onToggleRichMode();
    void onTextBold();
    void onTextItalic();
    void onTextUnderline();
    void onTextStrikeOut();
    void onTextColor();
    void onTextBackgroundColor();
    void onFontFamilyChanged(const QString& fontFamily);
    void onFontSizeChanged(int size);
    void onInsertImage();
    void onInsertFile();

private:
    void setupUI();
    void setupMessageArea();
    void setupRichTextToolbar();
    void setupInputArea();
    void addMessage(const ChatMessage& message);
    void addRichMessage(const RichChatMessage& message);
    void scrollToBottom();
    void updateConnectionStatus();
    void loadChatHistory();
    void loadRichChatHistory();
    QString formatTime(const QDateTime& time);
    QWidget* createMessageBubble(const ChatMessage& message);
    QWidget* createRichMessageBubble(const RichChatMessage& message);
    
    // 富文本处理方法
    void insertImageIntoEditor(const QString& imagePath);
    void insertFileIntoEditor(const QString& filePath);
    void saveRichChatHistory(const RichChatMessage& message);
    QString convertToRichText(const QString& plainText);
    
    // 消息解析方法
    QString parseHTMLFromMessage(const QString& content);
    QString parsePlainTextFromMessage(const QString& content);
    
    // 会话评价
    void showRatingDialog(const ChatSession& session);
    void onManualRating(); // 手动评价槽函数

    // UI 组件
    QVBoxLayout* m_mainLayout;
    QLabel* m_statusLabel;
    
    // 富文本工具栏
    QToolBar* m_richTextToolbar;
    QAction* m_actionBold;
    QAction* m_actionItalic;
    QAction* m_actionUnderline;
    QAction* m_actionStrikeOut;
    QAction* m_actionTextColor;
    QAction* m_actionBackgroundColor;
    QAction* m_actionInsertImage;
    QAction* m_actionInsertFile;
    QFontComboBox* m_fontComboBox;
    QSpinBox* m_fontSizeSpinBox;
    
    QScrollArea* m_messageScrollArea;
    QWidget* m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QTextEdit* m_messageInput;        // 普通文本输入
    QTextEdit* m_richMessageInput;    // 富文本输入
    QPushButton* m_sendButton;
    QPushButton* m_toggleRichModeButton;
    QPushButton* m_startChatButton;
    QPushButton* m_rateServiceButton;  // 评价服务按钮
    
    // 数据
    UserInfo m_currentUser;
    int m_currentSessionId;
    DatabaseManager* m_dbManager;
    QTimer* m_messageCheckTimer;
    QTimer* m_typingTimer;
    
    // 状态
    bool m_isConnected;
    bool m_isTyping;
    bool m_isRichMode;               // 是否启用富文本模式
    QDateTime m_lastMessageTime;
    
    // 富文本聊天记录
    QList<RichChatMessage> m_richChatHistory;
};

#endif // REALCHATWIDGET_H 