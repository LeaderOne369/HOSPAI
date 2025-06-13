#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QButtonGroup>
#include <QGroupBox>
#include <QToolBar>
#include <QAction>
#include <QColorDialog>
#include <QFontComboBox>
#include <QSpinBox>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QFileDialog>
#include <QMimeData>
#include <QPixmap>
#include <QImageReader>
#include "../../core/DatabaseManager.h"
#include "../../core/AIApiClient.h"
#include "../../core/RichMessageTypes.h"

// AI分诊消息结构体 (保持向后兼容)
struct AIMessage {
    QString content;
    MessageType type;
    QDateTime timestamp;
    QString sessionId;
};

// 分诊建议结构体
struct TriageAdvice {
    QString department;     // 推荐科室
    QString reason;         // 推荐理由
    QString action;         // 建议操作
    bool needAppointment;   // 是否需要预约
    bool needEmergency;     // 是否紧急
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();
    
    void setDatabaseManager(DatabaseManager* dbManager);
    void setUserInfo(const QString& userId, const QString& userName);

signals:
    void requestHumanService(const QString& userId, const QString& userName, const QString& context);

private slots:
    // 消息发送相关
    void onSendMessage();
    void onSendRichMessage();
    void onQuickButtonClicked(QAbstractButton* button);
    void onInputTextChanged();
    
    // AI响应相关
    void onAIResponseReady();
    void onRichAIResponseReady();
    void simulateTyping();
    void onAITriageResponse(const AIDiagnosisResult& result);
    void onAIApiError(const QString& error);
    
    // 交互按钮相关
    void onActionButtonClicked();
    void onDepartmentSelected();
    void onTransferToHuman();  // 转人工服务
    
    // 功能按钮相关
    void onClearChatClicked();
    void onSaveChatClicked();
    void onSettingsClicked();
    
    // 富文本功能槽函数
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
    void onToggleRichMode();

private:
    // UI设置
    void setupUI();
    void setupChatArea();
    void setupQuickButtonsArea();
    void setupInputArea();
    void setupRichTextToolbar();
    void setupToolBar();
    
    // 消息处理
    void addMessage(const AIMessage& message);        // 向后兼容
    void addRichMessage(const RichMessage& message);  // 新的富文本消息
    void displayMessage(const AIMessage& message);    // 向后兼容
    void displayRichMessage(const RichMessage& message); // 新的富文本显示
    void scrollToBottom();
    
    // 富文本处理
    QWidget* createRichMessageBubble(const RichMessage& message);
    QString convertToRichText(const QString& plainText);
    QString extractPlainText(const QString& richText);
    void insertImageIntoEditor(const QString& imagePath);
    void insertFileIntoEditor(const QString& filePath);
    
    // AI分诊逻辑
    TriageAdvice analyzeSymptoms(const QString& userInput);
    QString generateAIResponse(const QString& userInput);
    void processTriageAdvice(const TriageAdvice& advice);
    
    // 快捷按钮管理
    void setupQuickButtons();
    void addQuickButton(const QString& text, const QString& responseTemplate);
    void updateQuickButtons(const QStringList& suggestions);
    
    // 交互组件
    void addActionButtons(const QStringList& actions);
    void addDepartmentSelector(const QStringList& departments);
    void addTransferOption();  // 添加转人工选项
    void clearInteractionComponents();
    
    // 数据库操作
    void initDatabase();
    void saveChatHistory();
    void saveRichChatHistory();
    void loadChatHistory();
    void loadRichChatHistory();
    QString generateSessionId();
    
    // 工具方法
    QString formatTimestamp(const QDateTime& timestamp);
    QString extractKeywords(const QString& text);
    QStringList getSymptomKeywords(const QString& text);

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    
    // 工具栏
    QHBoxLayout* m_toolBarLayout;
    QPushButton* m_btnClearChat;
    QPushButton* m_btnSaveChat;
    QPushButton* m_btnSettings;
    QPushButton* m_btnToggleRichMode;
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
    
    // 聊天显示区域
    QScrollArea* m_chatScrollArea;
    QWidget* m_chatContainer;
    QVBoxLayout* m_chatLayout;
    
    // 快捷按钮区域
    QGroupBox* m_quickButtonsGroup;
    QGridLayout* m_quickButtonsLayout;
    QButtonGroup* m_quickButtonGroup;
    
    // 动态交互区域
    QWidget* m_interactionWidget;
    QVBoxLayout* m_interactionLayout;
    
    // 输入区域
    QWidget* m_inputWidget;
    QHBoxLayout* m_inputLayout;
    QTextEdit* m_messageInput;        // 普通文本输入
    QTextEdit* m_richMessageInput;    // 富文本输入
    QPushButton* m_btnSend;
    QPushButton* m_btnVoice;     // 语音输入（预留）
    QPushButton* m_btnEmoji;     // 表情输入（预留）
    
    // AI处理相关
    QTimer* m_typingTimer;       // 模拟打字效果
    QTimer* m_responseTimer;     // 模拟AI响应延迟
    QTimer* m_richResponseTimer; // 富文本模式AI响应延迟
    QString m_pendingResponse;   // 待发送的AI响应
    QString m_pendingRichResponse; // 待发送的富文本AI响应
    QString m_pendingResponsePlainText; // 富文本模式下用户输入的纯文本
    bool m_isAITyping;          // AI是否正在输入
    AIApiClient* m_aiApiClient;  // AI API客户端
    
    // 数据存储
    QList<AIMessage> m_chatHistory;          // 兼容性聊天记录
    QList<RichMessage> m_richChatHistory;    // 富文本聊天记录
    QString m_currentSessionId;
    QSqlDatabase m_database;
    
    // 分诊相关
    QStringList m_departments;   // 科室列表
    QMap<QString, QStringList> m_symptomKeywords; // 症状关键词映射
    
    // 状态管理
    bool m_isInitialized;
    bool m_isRichMode;           // 是否启用富文本模式
    int m_messageCount;
    QString m_currentContext;    // 当前对话上下文
    QString m_userId;
    QString m_userName;
    DatabaseManager* m_dbManager;
};

#endif // CHATWIDGET_H 