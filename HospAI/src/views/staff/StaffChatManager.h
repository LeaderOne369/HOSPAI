#ifndef STAFFCHATMANAGER_H
#define STAFFCHATMANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QGroupBox>
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

class StaffChatManager : public QWidget
{
    Q_OBJECT

public:
    explicit StaffChatManager(QWidget *parent = nullptr);
    ~StaffChatManager();
    
    void setCurrentUser(const UserInfo& user);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSessionSelectionChanged();
    void onSendMessage();
    void onSendRichMessage();
    void onAcceptSession(int sessionId);
    void onCloseSession(int sessionId);
    void onMessageReceived(const ChatMessage& message);
    void onRichMessageReceived(const RichChatMessage& message);
    void onSessionCreated(const ChatSession& session);
    void onSessionUpdated(const ChatSession& session);
    void checkForNewSessions();
    void checkForNewMessages();
    void refreshSessionList();
    
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
    
    // 会话统计和评价管理
    void showMyRatings();
    void updateSessionStats();
    
    // 快捷回复功能
    void onQuickReplyClicked(const QString& replyText);

private:
    void setupUI();
    void setupSessionList();
    void setupChatArea();
    void setupCustomerInfoPanel();
    void setupQuickReplyPanel();
    void setupRichTextToolbar();
    void setupWaitingList();
    void loadSessionList();
    void loadChatHistory(int sessionId);
    void addMessage(const ChatMessage& message);
    void addRichMessage(const RichChatMessage& message);
    void scrollToBottom();
    QWidget* createMessageBubble(const ChatMessage& message);
    QWidget* createRichMessageBubble(const RichChatMessage& message);
    QListWidgetItem* createSessionItem(const ChatSession& session);
    QString formatTime(const QDateTime& time);
    void updateSessionItemStyle(QListWidgetItem* item, const ChatSession& session);
    
    // 富文本处理方法
    void insertImageIntoEditor(const QString& imagePath);
    void insertFileIntoEditor(const QString& filePath);
    void saveRichChatHistory(const RichChatMessage& message);
    void parseRichTextMessage(const ChatMessage& chatMessage, RichChatMessage& richMessage);
    
    // 客户信息和快捷回复方法
    void updateCustomerInfo(int sessionId);
    void setupQuickReplyButtons();

    // UI 组件
    QHBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 左侧会话列表
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    QGroupBox* m_activeSessionsGroup;
    QListWidget* m_activeSessionsList;
    QGroupBox* m_waitingSessionsGroup;
    QListWidget* m_waitingSessionsList;
    QLabel* m_statsLabel;
    QLabel* m_ratingLabel;
    QPushButton* m_viewRatingsButton;
    
    // 右侧聊天区域
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    QLabel* m_chatTitleLabel;
    
    // 客户信息面板
    QGroupBox* m_customerInfoGroup;
    QLabel* m_customerNameLabel;
    QLabel* m_customerPhoneLabel;
    QLabel* m_customerStatusLabel;
    QLabel* m_sessionTimeLabel;
    
    // 快捷回复面板
    QGroupBox* m_quickReplyGroup;
    QWidget* m_quickReplyWidget;
    QHBoxLayout* m_quickReplyLayout;
    
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
    
    // 输入区域
    QHBoxLayout* m_inputLayout;
    QTextEdit* m_messageInput;        // 普通文本输入
    QTextEdit* m_richMessageInput;    // 富文本输入
    QPushButton* m_sendButton;
    QPushButton* m_toggleRichModeButton;
    QPushButton* m_closeSessionButton;
    
    // 数据
    UserInfo m_currentUser;
    int m_currentSessionId;
    DatabaseManager* m_dbManager;
    QTimer* m_sessionCheckTimer;
    QTimer* m_messageCheckTimer;
    
    // 状态
    bool m_isRichMode;               // 是否启用富文本模式
    
    // 会话映射
    QMap<int, ChatSession> m_sessions;
    QMap<QListWidgetItem*, int> m_itemToSessionId;
};

#endif // STAFFCHATMANAGER_H 