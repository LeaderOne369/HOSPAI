#include "StaffChatManager.h"
#include "../common/UIStyleManager.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>
#include <QKeyEvent>
#include <QSplitter>
#include <QToolBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QColorDialog>
#include <QFileDialog>
#include <QTextImageFormat>
#include <QPixmap>
#include <QFileInfo>
#include <QTextBlock>
#include <QTextFragment>
#include <QRegularExpression>
#include <QBuffer>
#include <QGridLayout>

StaffChatManager::StaffChatManager(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_leftPanel(nullptr)
    , m_rightPanel(nullptr)
    , m_leftLayout(nullptr)
    , m_rightLayout(nullptr)
    , m_inputLayout(nullptr)
    , m_activeSessionsGroup(nullptr)
    , m_activeSessionsList(nullptr)
    , m_waitingSessionsGroup(nullptr)
    , m_waitingSessionsList(nullptr)
    , m_statsLabel(nullptr)
    , m_ratingLabel(nullptr)
    , m_viewRatingsButton(nullptr)
    , m_chatTitleLabel(nullptr)
    , m_customerInfoGroup(nullptr)
    , m_customerNameLabel(nullptr)
    , m_customerPhoneLabel(nullptr)
    , m_customerStatusLabel(nullptr)
    , m_sessionTimeLabel(nullptr)
    , m_quickReplyGroup(nullptr)
    , m_quickReplyWidget(nullptr)
    , m_quickReplyLayout(nullptr)
    , m_richTextToolbar(nullptr)
    , m_actionBold(nullptr)
    , m_actionItalic(nullptr)
    , m_actionUnderline(nullptr)
    , m_actionStrikeOut(nullptr)
    , m_actionTextColor(nullptr)
    , m_actionBackgroundColor(nullptr)
    , m_actionInsertImage(nullptr)
    , m_actionInsertFile(nullptr)
    , m_fontComboBox(nullptr)
    , m_fontSizeSpinBox(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_messageInput(nullptr)
    , m_richMessageInput(nullptr)
    , m_sendButton(nullptr)
    , m_toggleRichModeButton(nullptr)
    , m_closeSessionButton(nullptr)
    , m_currentSessionId(-1)
    , m_dbManager(nullptr)
    , m_sessionCheckTimer(nullptr)
    , m_messageCheckTimer(nullptr)
    , m_isRichMode(false)
{
    // 初始化核心组件
    m_dbManager = DatabaseManager::instance();
    m_sessionCheckTimer = new QTimer(this);
    m_messageCheckTimer = new QTimer(this);
    
    // 初始化UI
    setupUI();
    
    // 连接数据库信号
    if (m_dbManager) {
        connect(m_dbManager, &DatabaseManager::newMessageReceived, 
                this, &StaffChatManager::onMessageReceived);
        connect(m_dbManager, &DatabaseManager::sessionCreated, 
                this, &StaffChatManager::onSessionCreated);
        connect(m_dbManager, &DatabaseManager::sessionUpdated, 
                this, &StaffChatManager::onSessionUpdated);
    }
    
    // 定时检查新会话和消息
    if (m_sessionCheckTimer && m_messageCheckTimer) {
        connect(m_sessionCheckTimer, &QTimer::timeout, this, &StaffChatManager::checkForNewSessions);
        connect(m_messageCheckTimer, &QTimer::timeout, this, &StaffChatManager::checkForNewMessages);
        
        m_sessionCheckTimer->start(3000); // 每3秒检查新会话
        m_messageCheckTimer->start(2000); // 每2秒检查新消息
    }
}

StaffChatManager::~StaffChatManager()
{
    if (m_currentUser.id > 0) {
        m_dbManager->updateUserOnlineStatus(m_currentUser.id, false);
    }
}

void StaffChatManager::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    setupSessionList();
    setupChatArea();
    
    m_splitter->addWidget(m_leftPanel);
    m_splitter->addWidget(m_rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);
    
    m_mainLayout->addWidget(m_splitter);
}

void StaffChatManager::setupSessionList()
{
    m_leftPanel = new QWidget;
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(10);
    
    // 统计信息
    m_statsLabel = new QLabel("客服工作台", this);
    UIStyleManager::applyLabelStyle(m_statsLabel, "title");
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_leftLayout->addWidget(m_statsLabel);
    
    // 等待接入的会话
    m_waitingSessionsGroup = new QGroupBox("等待接入", this);
    UIStyleManager::applyGroupBoxStyle(m_waitingSessionsGroup);
    QVBoxLayout* waitingLayout = new QVBoxLayout(m_waitingSessionsGroup);
    
    m_waitingSessionsList = new QListWidget(this);
    m_waitingSessionsList->setMaximumHeight(150);
    waitingLayout->addWidget(m_waitingSessionsList);
    
    connect(m_waitingSessionsList, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
        int sessionId = m_itemToSessionId.value(item, -1);
        if (sessionId > 0) {
            onAcceptSession(sessionId);
        }
    });
    
    m_leftLayout->addWidget(m_waitingSessionsGroup);
    
    // 进行中的会话
    m_activeSessionsGroup = new QGroupBox("进行中的对话", this);
    UIStyleManager::applyGroupBoxStyle(m_activeSessionsGroup);
    QVBoxLayout* activeLayout = new QVBoxLayout(m_activeSessionsGroup);
    
    m_activeSessionsList = new QListWidget(this);
    activeLayout->addWidget(m_activeSessionsList);
    
    connect(m_activeSessionsList, &QListWidget::itemClicked, 
            this, &StaffChatManager::onSessionSelectionChanged);
    
    m_leftLayout->addWidget(m_activeSessionsGroup);
    
    // 个人统计和评价区域
    QWidget* statsWidget = new QWidget(this);
    QVBoxLayout* statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->setContentsMargins(5, 5, 5, 5);
    statsLayout->setSpacing(5);
    
    m_ratingLabel = new QLabel("服务评价: 加载中...", this);
    m_ratingLabel->setStyleSheet("font-size: 12px; color: #5D6D7E;");
    m_ratingLabel->setAlignment(Qt::AlignCenter);
    statsLayout->addWidget(m_ratingLabel);
    
    m_viewRatingsButton = new QPushButton("查看评价详情", this);
    m_viewRatingsButton->setMaximumHeight(30);
    UIStyleManager::applyButtonStyle(m_viewRatingsButton, "secondary");
    connect(m_viewRatingsButton, &QPushButton::clicked, this, &StaffChatManager::showMyRatings);
    statsLayout->addWidget(m_viewRatingsButton);
    
    m_leftLayout->addWidget(statsWidget);
}

void StaffChatManager::setupChatArea()
{
    m_rightPanel = new QWidget;
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightLayout->setSpacing(10);
    
    // 聊天标题
    m_chatTitleLabel = new QLabel("请选择一个对话", this);
    UIStyleManager::applyLabelStyle(m_chatTitleLabel, "subtitle");
    m_chatTitleLabel->setAlignment(Qt::AlignCenter);
    m_rightLayout->addWidget(m_chatTitleLabel);
    
    // 客户信息面板
    setupCustomerInfoPanel();
    
    // 快捷回复面板  
    setupQuickReplyPanel();
    
    // 富文本工具栏
    setupRichTextToolbar();
    
    // 消息显示区域
    m_messageScrollArea = new QScrollArea(this);
    m_messageScrollArea->setWidgetResizable(true);
    m_messageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_messageContainer = new QWidget;
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(10, 10, 10, 10);
    m_messageLayout->setSpacing(10);
    m_messageLayout->addStretch();
    
    m_messageScrollArea->setWidget(m_messageContainer);
    UIStyleManager::applyScrollAreaStyle(m_messageScrollArea);
    
    m_rightLayout->addWidget(m_messageScrollArea);
    
    // 输入区域
    QWidget* inputWidget = new QWidget(this);
    m_inputLayout = new QHBoxLayout(inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(10);
    
    // 普通消息输入框
    m_messageInput = new QTextEdit(this);
    m_messageInput->setMaximumHeight(80);
    m_messageInput->setPlaceholderText("输入回复消息...");
    UIStyleManager::applyTextEditStyle(m_messageInput);
    m_messageInput->setEnabled(false);
    
    // 富文本消息输入框
    m_richMessageInput = new QTextEdit(this);
    m_richMessageInput->setMaximumHeight(120);
    m_richMessageInput->setPlaceholderText("输入富文本回复，支持格式化、图片和文件...");
    m_richMessageInput->setAcceptDrops(true);
    m_richMessageInput->setAcceptRichText(true); // 启用富文本
    m_richMessageInput->setTextInteractionFlags(Qt::TextEditorInteraction);
    UIStyleManager::applyTextEditStyle(m_richMessageInput);
    m_richMessageInput->setEnabled(false);
    m_richMessageInput->hide(); // 初始隐藏
    
    // 按钮区域
    QWidget* buttonWidget = new QWidget;
    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(5);
    
    m_toggleRichModeButton = new QPushButton("富文本模式", this);
    m_toggleRichModeButton->setCheckable(true);
    m_toggleRichModeButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_toggleRichModeButton, "secondary");
    m_toggleRichModeButton->setEnabled(false);
    
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_sendButton, "primary");
    m_sendButton->setEnabled(false);
    
    m_closeSessionButton = new QPushButton("结束对话", this);
    m_closeSessionButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_closeSessionButton, "error");
    m_closeSessionButton->setEnabled(false);
    
    buttonLayout->addWidget(m_toggleRichModeButton);
    buttonLayout->addWidget(m_sendButton);
    buttonLayout->addWidget(m_closeSessionButton);
    buttonLayout->addStretch();
    
    connect(m_messageInput, &QTextEdit::textChanged, [this]() {
        bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_currentSessionId > 0);
    });
    
    connect(m_richMessageInput, &QTextEdit::textChanged, [this]() {
        bool hasText = !m_richMessageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_currentSessionId > 0);
    });
    
    connect(m_toggleRichModeButton, &QPushButton::clicked, this, &StaffChatManager::onToggleRichMode);
    connect(m_sendButton, &QPushButton::clicked, this, [this]() {
        if (m_isRichMode) {
            onSendRichMessage();
        } else {
            onSendMessage();
        }
    });
    connect(m_closeSessionButton, &QPushButton::clicked, [this]() {
        if (m_currentSessionId > 0) {
            onCloseSession(m_currentSessionId);
        }
    });
    
    // 键盘事件
    m_messageInput->installEventFilter(this);
    m_richMessageInput->installEventFilter(this);
    
    m_inputLayout->addWidget(m_messageInput);
    m_inputLayout->addWidget(m_richMessageInput);
    m_inputLayout->addWidget(buttonWidget);
    
    m_rightLayout->addWidget(inputWidget);
}

void StaffChatManager::setupRichTextToolbar()
{
    m_richTextToolbar = new QToolBar("富文本工具栏", this);
    m_richTextToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_richTextToolbar->setIconSize(QSize(20, 20));
    
    // 字体选择
    m_fontComboBox = new QFontComboBox;
    m_fontComboBox->setMaximumWidth(150);
    connect(m_fontComboBox, &QFontComboBox::currentFontChanged,
            this, [this](const QFont& font) { onFontFamilyChanged(font.family()); });
    
    // 字体大小
    m_fontSizeSpinBox = new QSpinBox;
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(12);
    m_fontSizeSpinBox->setMaximumWidth(60);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &StaffChatManager::onFontSizeChanged);
    
    // 格式化按钮
    m_actionBold = m_richTextToolbar->addAction("B");
    m_actionBold->setCheckable(true);
    m_actionBold->setToolTip("粗体");
    connect(m_actionBold, &QAction::triggered, this, &StaffChatManager::onTextBold);
    
    m_actionItalic = m_richTextToolbar->addAction("I");
    m_actionItalic->setCheckable(true);
    m_actionItalic->setToolTip("斜体");
    connect(m_actionItalic, &QAction::triggered, this, &StaffChatManager::onTextItalic);
    
    m_actionUnderline = m_richTextToolbar->addAction("U");
    m_actionUnderline->setCheckable(true);
    m_actionUnderline->setToolTip("下划线");
    connect(m_actionUnderline, &QAction::triggered, this, &StaffChatManager::onTextUnderline);
    
    m_actionStrikeOut = m_richTextToolbar->addAction("S");
    m_actionStrikeOut->setCheckable(true);
    m_actionStrikeOut->setToolTip("删除线");
    connect(m_actionStrikeOut, &QAction::triggered, this, &StaffChatManager::onTextStrikeOut);
    
    m_richTextToolbar->addSeparator();
    
    // 颜色按钮
    m_actionTextColor = m_richTextToolbar->addAction("🎨");
    m_actionTextColor->setToolTip("文字颜色");
    connect(m_actionTextColor, &QAction::triggered, this, &StaffChatManager::onTextColor);
    
    m_actionBackgroundColor = m_richTextToolbar->addAction("🖍️");
    m_actionBackgroundColor->setToolTip("背景颜色");
    connect(m_actionBackgroundColor, &QAction::triggered, this, &StaffChatManager::onTextBackgroundColor);
    
    m_richTextToolbar->addSeparator();
    
    // 插入功能
    m_actionInsertImage = m_richTextToolbar->addAction("🖼️");
    m_actionInsertImage->setToolTip("插入图片");
    connect(m_actionInsertImage, &QAction::triggered, this, &StaffChatManager::onInsertImage);
    
    m_actionInsertFile = m_richTextToolbar->addAction("📎");
    m_actionInsertFile->setToolTip("插入文件");
    connect(m_actionInsertFile, &QAction::triggered, this, &StaffChatManager::onInsertFile);
    
    // 添加组件到工具栏
    m_richTextToolbar->insertWidget(m_actionBold, new QLabel("字体: "));
    m_richTextToolbar->insertWidget(m_actionBold, m_fontComboBox);
    m_richTextToolbar->insertWidget(m_actionBold, new QLabel(" 大小: "));
    m_richTextToolbar->insertWidget(m_actionBold, m_fontSizeSpinBox);
    m_richTextToolbar->insertSeparator(m_actionBold);
    
    // 设置工具栏样式
    m_richTextToolbar->setStyleSheet(R"(
        QToolBar {
            background-color: #F2F2F7;
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            padding: 5px;
            spacing: 5px;
        }
        QToolButton {
            background-color: white;
            border: 1px solid #D1D1D6;
            border-radius: 4px;
            padding: 4px 8px;
            font-weight: bold;
            min-width: 24px;
            min-height: 24px;
        }
        QToolButton:hover {
            background-color: #E5E5EA;
        }
        QToolButton:checked {
            background-color: #007AFF;
            color: white;
        }
        QFontComboBox, QSpinBox {
            border: 1px solid #D1D1D6;
            border-radius: 4px;
            padding: 2px 4px;
            background-color: white;
        }
    )");
    
    m_richTextToolbar->hide(); // 初始隐藏，只在富文本模式下显示
    m_rightLayout->addWidget(m_richTextToolbar);
}

void StaffChatManager::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新在线状态
    if (m_dbManager) {
        m_dbManager->updateUserOnlineStatus(user.id, true);
    }
    
    // 更新统计标签
    if (m_statsLabel) {
        m_statsLabel->setText(QString("客服工作台 - %1").arg(user.realName.isEmpty() ? user.username : user.realName));
    }
    
    // 加载会话列表
    loadSessionList();
    
    // 更新统计信息
    updateSessionStats();
}

void StaffChatManager::loadSessionList()
{
    if (m_currentUser.id <= 0 || !m_dbManager || !m_activeSessionsList || !m_waitingSessionsList) return;
    
    // 清空列表
    m_activeSessionsList->clear();
    m_waitingSessionsList->clear();
    m_itemToSessionId.clear();
    m_sessions.clear();
    
    // 获取活跃会话
    QList<ChatSession> activeSessions = m_dbManager->getActiveSessions();
    
    for (const ChatSession& session : activeSessions) {
        m_sessions[session.id] = session;
        
        QListWidgetItem* item = createSessionItem(session);
        
        if (session.status == 2) {
            // 等待中的会话
            m_waitingSessionsList->addItem(item);
        } else if (session.status == 1 && session.staffId == m_currentUser.id) {
            // 自己负责的进行中会话
            m_activeSessionsList->addItem(item);
        }
        
        m_itemToSessionId[item] = session.id;
    }
    
    // 更新统计信息
    if (m_waitingSessionsGroup && m_activeSessionsGroup) {
        int waitingCount = m_waitingSessionsList->count();
        int activeCount = m_activeSessionsList->count();
        m_waitingSessionsGroup->setTitle(QString("等待接入 (%1)").arg(waitingCount));
        m_activeSessionsGroup->setTitle(QString("进行中的对话 (%1)").arg(activeCount));
    }
}

QListWidgetItem* StaffChatManager::createSessionItem(const ChatSession& session)
{
    QListWidgetItem* item = new QListWidgetItem;
    
    QString text = QString("%1\n最后消息: %2")
                   .arg(session.patientName)
                   .arg(formatTime(session.lastMessageAt));
    
    item->setText(text);
    updateSessionItemStyle(item, session);
    
    return item;
}

void StaffChatManager::updateSessionItemStyle(QListWidgetItem* item, const ChatSession& session)
{
    if (session.status == 2) {
        // 等待中 - 橙色
        item->setBackground(QBrush(QColor("#FFF3CD")));
        item->setForeground(QBrush(QColor("#856404")));
    } else if (session.status == 1) {
        // 进行中 - 绿色
        item->setBackground(QBrush(QColor("#D4EDDA")));
        item->setForeground(QBrush(QColor("#155724")));
    }
}

void StaffChatManager::onSessionSelectionChanged()
{
    QListWidgetItem* currentItem = m_activeSessionsList->currentItem();
    if (!currentItem) return;
    
    int sessionId = m_itemToSessionId.value(currentItem, -1);
    if (sessionId <= 0) return;
    
    m_currentSessionId = sessionId;
    ChatSession session = m_sessions.value(sessionId);
    
    // 更新聊天标题
    m_chatTitleLabel->setText(QString("与 %1 的对话").arg(session.patientName));
    
    // 启用输入组件
    m_messageInput->setEnabled(true);
    m_richMessageInput->setEnabled(true);
    m_toggleRichModeButton->setEnabled(true);
    m_closeSessionButton->setEnabled(true);
    
    // 根据当前模式启用相应的输入框
    if (m_isRichMode) {
        m_messageInput->setEnabled(false);
        m_richMessageInput->setEnabled(true);
    } else {
        m_messageInput->setEnabled(true);
        m_richMessageInput->setEnabled(false);
    }
    
    // 清空消息区域
    QLayoutItem* child;
    while ((child = m_messageLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    m_messageLayout->addStretch();
    
    // 加载聊天历史
    loadChatHistory(sessionId);
    
    // 更新客户信息面板
    updateCustomerInfo(sessionId);
    
    // 显示快捷回复面板
    if (m_quickReplyGroup) {
        m_quickReplyGroup->setVisible(true);
    }
    
    // 标记消息为已读
    m_dbManager->markSessionAsRead(sessionId, m_currentUser.id);
}

void StaffChatManager::loadChatHistory(int sessionId)
{
    QList<ChatMessage> messages = m_dbManager->getChatMessages(sessionId);
    
    for (const ChatMessage& message : messages) {
        addMessage(message);
    }
}

void StaffChatManager::onAcceptSession(int sessionId)
{
    if (m_dbManager->updateChatSession(sessionId, m_currentUser.id)) {
        // 刷新会话列表
        refreshSessionList();
        
        // 自动选择这个会话
        for (int i = 0; i < m_activeSessionsList->count(); ++i) {
            QListWidgetItem* item = m_activeSessionsList->item(i);
            if (m_itemToSessionId.value(item) == sessionId) {
                m_activeSessionsList->setCurrentItem(item);
                onSessionSelectionChanged();
                break;
            }
        }
    }
}

void StaffChatManager::onCloseSession(int sessionId)
{
    int ret = QMessageBox::question(this, "确认", "确定要结束这个对话吗？", 
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager->closeChatSession(sessionId)) {
            // 清空当前选择
            m_currentSessionId = -1;
            m_chatTitleLabel->setText("请选择一个对话");
            m_messageInput->setEnabled(false);
            m_richMessageInput->setEnabled(false);
            m_toggleRichModeButton->setEnabled(false);
            m_closeSessionButton->setEnabled(false);
            
            // 隐藏客户信息面板和快捷回复面板
            if (m_customerInfoGroup) {
                m_customerInfoGroup->setVisible(false);
            }
            if (m_quickReplyGroup) {
                m_quickReplyGroup->setVisible(false);
            }
            
            // 清空消息区域
            QLayoutItem* child;
            while ((child = m_messageLayout->takeAt(0)) != nullptr) {
                if (child->widget()) {
                    delete child->widget();
                }
                delete child;
            }
            m_messageLayout->addStretch();
            
            // 刷新会话列表
            refreshSessionList();
        }
    }
}

void StaffChatManager::onSendMessage()
{
    QString content = m_messageInput->toPlainText().trimmed();
    if (content.isEmpty() || m_currentSessionId <= 0) {
        return;
    }
    
    // 发送消息到数据库
    int messageId = m_dbManager->sendMessage(m_currentSessionId, m_currentUser.id, content);
    
    if (messageId > 0) {
        m_messageInput->clear();
        
        // 立即显示自己的消息
        ChatMessage message;
        message.id = messageId;
        message.sessionId = m_currentSessionId;
        message.senderId = m_currentUser.id;
        message.senderName = m_currentUser.realName.isEmpty() ? m_currentUser.username : m_currentUser.realName;
        message.senderRole = m_currentUser.role;
        message.content = content;
        message.timestamp = QDateTime::currentDateTime();
        message.messageType = 0;
        message.isRead = 1;
        
        addMessage(message);
    }
}

void StaffChatManager::onMessageReceived(const ChatMessage& message)
{
    // 如果是当前会话的消息且不是自己发送的，显示消息
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addMessage(message);
        m_dbManager->markMessageAsRead(message.id);
    }
    
    // 刷新会话列表以更新最后消息时间
    refreshSessionList();
}

void StaffChatManager::onSessionCreated(const ChatSession& session)
{
    // 新会话创建，刷新列表
    refreshSessionList();
}

void StaffChatManager::onSessionUpdated(const ChatSession& session)
{
    // 会话更新，刷新列表
    refreshSessionList();
}

void StaffChatManager::checkForNewSessions()
{
    refreshSessionList();
}

void StaffChatManager::checkForNewMessages()
{
    if (m_currentSessionId <= 0) return;
    
    // 获取未读消息
    QList<ChatMessage> unreadMessages = m_dbManager->getUnreadMessages(m_currentUser.id);
    
    for (const ChatMessage& message : unreadMessages) {
        if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
            addMessage(message);
            m_dbManager->markMessageAsRead(message.id);
        }
    }
}

void StaffChatManager::refreshSessionList()
{
    // 保存当前选择
    int currentSelectedId = m_currentSessionId;
    
    // 重新加载
    loadSessionList();
    
    // 恢复选择
    if (currentSelectedId > 0) {
        for (int i = 0; i < m_activeSessionsList->count(); ++i) {
            QListWidgetItem* item = m_activeSessionsList->item(i);
            if (m_itemToSessionId.value(item) == currentSelectedId) {
                m_activeSessionsList->setCurrentItem(item);
                break;
            }
        }
    }
}

void StaffChatManager::addMessage(const ChatMessage& message)
{
    QWidget* messageBubble;
    
    // 检查是否是富文本消息
    if (message.content.contains("[RICH_TEXT]") && message.content.contains("[HTML]")) {
        // 解析富文本消息
        RichChatMessage richMessage;
        parseRichTextMessage(message, richMessage);
        messageBubble = createRichMessageBubble(richMessage);
    } else {
        // 普通消息
        messageBubble = createMessageBubble(message);
    }
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &StaffChatManager::scrollToBottom);
}

QWidget* StaffChatManager::createMessageBubble(const ChatMessage& message)
{
    QWidget* bubble = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(bubble);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息内容
    QFrame* messageFrame = new QFrame;
    QVBoxLayout* frameLayout = new QVBoxLayout(messageFrame);
    frameLayout->setContentsMargins(12, 8, 12, 8);
    frameLayout->setSpacing(4);
    
    // 发送者名称和时间
    QLabel* headerLabel = new QLabel;
    QString headerText = QString("%1  %2").arg(message.senderName).arg(formatTime(message.timestamp));
    headerLabel->setText(headerText);
    headerLabel->setStyleSheet("font-size: 11px; color: #8E8E93;");
    frameLayout->addWidget(headerLabel);
    
    // 消息内容
    QLabel* contentLabel = new QLabel(message.content);
    contentLabel->setWordWrap(true);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    frameLayout->addWidget(contentLabel);
    
    // 根据发送者设置样式和对齐
    if (message.senderId == m_currentUser.id) {
        // 自己的消息 - 右对齐，蓝色
        layout->addStretch();
        layout->addWidget(messageFrame);
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #007AFF;
                border-radius: 18px;
                max-width: 300px;
            }
        )");
        contentLabel->setStyleSheet("color: white; font-size: 14px;");
        headerLabel->setStyleSheet("font-size: 11px; color: rgba(255,255,255,0.8);");
        
    } else if (message.messageType == 1) {
        // 系统消息 - 居中，灰色
        layout->addStretch();
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #F2F2F7;
                border-radius: 18px;
                max-width: 250px;
            }
        )");
        contentLabel->setStyleSheet("color: #8E8E93; font-size: 13px; text-align: center;");
        
    } else {
        // 对方的消息 - 左对齐，灰色
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #E5E5EA;
                border-radius: 18px;
                max-width: 300px;
            }
        )");
        contentLabel->setStyleSheet("color: #000000; font-size: 14px;");
    }
    
    return bubble;
}

void StaffChatManager::scrollToBottom()
{
    QScrollBar* scrollBar = m_messageScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

QString StaffChatManager::formatTime(const QDateTime& time)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = time.secsTo(now);
    
    if (secs < 60) {
        return "刚刚";
    } else if (secs < 3600) {
        return QString("%1分钟前").arg(secs / 60);
    } else if (time.date() == now.date()) {
        return time.toString("hh:mm");
    } else {
        return time.toString("MM-dd hh:mm");
    }
}

bool StaffChatManager::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                // 不是 Shift+Enter，发送消息
                onSendMessage();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// 富文本功能实现
void StaffChatManager::onToggleRichMode()
{
    m_isRichMode = m_toggleRichModeButton->isChecked();
    
    if (m_isRichMode) {
        m_richTextToolbar->show();
        m_messageInput->hide();
        m_richMessageInput->show();
        m_toggleRichModeButton->setText("普通模式");
        
        // 启用富文本输入
        m_richMessageInput->setEnabled(m_currentSessionId > 0);
        
        // 更新发送按钮状态 - 基于富文本输入框内容
        bool hasText = !m_richMessageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_currentSessionId > 0);
    } else {
        m_richTextToolbar->hide();
        m_messageInput->show();
        m_richMessageInput->hide();
        m_toggleRichModeButton->setText("富文本模式");
        
        // 启用普通输入
        m_messageInput->setEnabled(m_currentSessionId > 0);
        
        // 更新发送按钮状态 - 基于普通输入框内容
        bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_currentSessionId > 0);
    }
}

void StaffChatManager::onTextBold()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontWeight(m_actionBold->isChecked() ? QFont::Bold : QFont::Normal);
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onTextItalic()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontItalic(m_actionItalic->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onTextUnderline()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontUnderline(m_actionUnderline->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onTextStrikeOut()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontStrikeOut(m_actionStrikeOut->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onTextColor()
{
    if (!m_richMessageInput) return;
    
    QColor color = QColorDialog::getColor(Qt::black, this, "选择文字颜色");
    if (color.isValid()) {
        QTextCursor cursor = m_richMessageInput->textCursor();
        QTextCharFormat format = cursor.charFormat();
        format.setForeground(color);
        cursor.mergeCharFormat(format);
        m_richMessageInput->mergeCurrentCharFormat(format);
    }
}

void StaffChatManager::onTextBackgroundColor()
{
    if (!m_richMessageInput) return;
    
    QColor color = QColorDialog::getColor(Qt::white, this, "选择背景颜色");
    if (color.isValid()) {
        QTextCursor cursor = m_richMessageInput->textCursor();
        QTextCharFormat format = cursor.charFormat();
        format.setBackground(color);
        cursor.mergeCharFormat(format);
        m_richMessageInput->mergeCurrentCharFormat(format);
    }
}

void StaffChatManager::onFontFamilyChanged(const QString& fontFamily)
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontFamilies({fontFamily});
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onFontSizeChanged(int size)
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(size);
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void StaffChatManager::onInsertImage()
{
    if (!m_richMessageInput) return;
    
    QString imagePath = QFileDialog::getOpenFileName(this, 
        "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp)");
    
    if (!imagePath.isEmpty()) {
        insertImageIntoEditor(imagePath);
    }
}

void StaffChatManager::onInsertFile()
{
    if (!m_richMessageInput) return;
    
    QString filePath = QFileDialog::getOpenFileName(this, 
        "选择文件", "", "所有文件 (*)");
    
    if (!filePath.isEmpty()) {
        insertFileIntoEditor(filePath);
    }
}

void StaffChatManager::insertImageIntoEditor(const QString& imagePath)
{
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) return;
    
    // 读取图片并调整大小
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) return;
    
    // 限制图片大小为300x300
    int maxSize = 300;
    if (pixmap.width() > maxSize || pixmap.height() > maxSize) {
        pixmap = pixmap.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 将图片转换为Base64编码
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    QByteArray imageData = buffer.data();
    QString base64Data = imageData.toBase64();
    
    // 创建带有Base64数据的HTML图片标签
    QString imageHtml = QString("<img src=\"data:image/png;base64,%1\" width=\"%2\" height=\"%3\" />")
                       .arg(base64Data)
                       .arg(pixmap.width())
                       .arg(pixmap.height());
    
    // 插入图片HTML
    QTextCursor cursor = m_richMessageInput->textCursor();
    cursor.insertHtml(imageHtml);
    m_richMessageInput->setTextCursor(cursor);
    
    qDebug() << "客服端插入图片，Base64长度:" << base64Data.length() << "尺寸:" << pixmap.size();
}

void StaffChatManager::insertFileIntoEditor(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) return;
    
    QString fileName = fileInfo.fileName();
    QString fileSize = QString::number(fileInfo.size() / 1024.0, 'f', 1) + " KB";
    
    // 插入文件链接
    QTextCursor cursor = m_richMessageInput->textCursor();
    QString fileHtml = QString("<p>📎 <a href=\"file:///%1\">%2</a> (%3)</p>")
                      .arg(filePath)
                      .arg(fileName)
                      .arg(fileSize);
    
    cursor.insertHtml(fileHtml);
    m_richMessageInput->setTextCursor(cursor);
}

void StaffChatManager::onSendRichMessage()
{
    if (!m_richMessageInput || m_richMessageInput->toPlainText().trimmed().isEmpty()) {
        return;
    }
    
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个对话");
        return;
    }
    
    QString plainText = m_richMessageInput->toPlainText().trimmed();
    QString htmlContent = m_richMessageInput->toHtml();
    
    // 创建富文本消息
    RichChatMessage message;
    message.sessionId = m_currentSessionId;
    message.senderId = m_currentUser.id;
    message.senderName = m_currentUser.realName.isEmpty() ? m_currentUser.username : m_currentUser.realName;
    message.senderRole = m_currentUser.role;
    message.content = plainText;
    message.htmlContent = htmlContent;
    message.contentType = RichContentType::RichText;
    message.timestamp = QDateTime::currentDateTime();
    message.messageType = 0;
    message.isRead = 1;
    
    // 检查是否包含图片或文件
    QTextDocument* doc = m_richMessageInput->document();
    QTextBlock block = doc->firstBlock();
    while (block.isValid()) {
        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                QTextCharFormat format = fragment.charFormat();
                if (format.isImageFormat()) {
                    QTextImageFormat imageFormat = format.toImageFormat();
                    message.attachments.append(imageFormat.name());
                    message.contentType = RichContentType::Mixed;
                }
            }
        }
        block = block.next();
    }
    
    // 保存到数据库
    saveRichChatHistory(message);
    
    // 显示消息
    addRichMessage(message);
    
    // 清空输入框
    m_richMessageInput->clear();
}

void StaffChatManager::addRichMessage(const RichChatMessage& message)
{
    QWidget* messageBubble = createRichMessageBubble(message);
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &StaffChatManager::scrollToBottom);
}

QWidget* StaffChatManager::createRichMessageBubble(const RichChatMessage& message)
{
    QWidget* bubble = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(bubble);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息内容
    QFrame* messageFrame = new QFrame;
    QVBoxLayout* frameLayout = new QVBoxLayout(messageFrame);
    frameLayout->setContentsMargins(12, 8, 12, 8);
    frameLayout->setSpacing(4);
    
    // 发送者名称和时间
    QLabel* headerLabel = new QLabel;
    QString headerText = QString("%1  %2").arg(message.senderName).arg(formatTime(message.timestamp));
    headerLabel->setText(headerText);
    headerLabel->setStyleSheet("font-size: 11px; color: #8E8E93;");
    frameLayout->addWidget(headerLabel);
    
    // 消息内容
    QTextBrowser* contentBrowser = new QTextBrowser;
    contentBrowser->setMaximumHeight(300);
    contentBrowser->setOpenExternalLinks(false);
    contentBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    contentBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 设置内容
    if (message.contentType == RichContentType::RichText || message.contentType == RichContentType::Mixed) {
        contentBrowser->setHtml(message.htmlContent);
    } else {
        contentBrowser->setPlainText(message.content);
    }
    
    frameLayout->addWidget(contentBrowser);
    
    // 根据发送者设置样式和对齐
    if (message.senderId == m_currentUser.id) {
        // 自己的消息 - 右对齐，蓝色
        layout->addStretch();
        layout->addWidget(messageFrame);
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #007AFF;
                border-radius: 18px;
                max-width: 400px;
            }
        )");
        contentBrowser->setStyleSheet("color: white; font-size: 14px; background: transparent; border: none;");
        headerLabel->setStyleSheet("font-size: 11px; color: rgba(255,255,255,0.8);");
        
    } else if (message.messageType == 1) {
        // 系统消息 - 居中，灰色
        layout->addStretch();
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #F2F2F7;
                border-radius: 18px;
                max-width: 300px;
            }
        )");
        contentBrowser->setStyleSheet("color: #8E8E93; font-size: 13px; text-align: center; background: transparent; border: none;");
        
    } else {
        // 对方的消息 - 左对齐，灰色
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #E5E5EA;
                border-radius: 18px;
                max-width: 400px;
            }
        )");
        contentBrowser->setStyleSheet("color: #000000; font-size: 14px; background: transparent; border: none;");
    }
    
    return bubble;
}

void StaffChatManager::saveRichChatHistory(const RichChatMessage& message)
{
    if (!m_dbManager) return;
    
    // 这里需要扩展数据库以支持富文本内容
    // 暂时使用JSON格式存储富文本数据
    // TODO: 实现富文本数据库存储
    
    // 临时转换为普通ChatMessage以保持兼容性
    ChatMessage tempMessage;
    tempMessage.sessionId = message.sessionId;
    tempMessage.senderId = message.senderId;
    tempMessage.senderName = message.senderName;
    tempMessage.senderRole = message.senderRole;
    tempMessage.content = message.content; // 使用纯文本内容
    tempMessage.timestamp = message.timestamp;
    tempMessage.messageType = message.messageType;
    tempMessage.isRead = message.isRead;
    
    // 暂时保存为普通消息
    int messageId = m_dbManager->sendMessage(tempMessage.sessionId, tempMessage.senderId, 
                                           tempMessage.content, tempMessage.messageType);
    
    if (messageId > 0) {
        // 这里可以在将来添加富文本扩展信息的保存
        qDebug() << "富文本消息已保存为普通消息，ID:" << messageId;
    }
}

void StaffChatManager::onRichMessageReceived(const RichChatMessage& message)
{
    // 如果是当前会话的消息且不是自己发送的，显示消息
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addRichMessage(message);
        // TODO: 标记富文本消息为已读
    }
    
    // 刷新会话列表以更新最后消息时间
    refreshSessionList();
}

void StaffChatManager::parseRichTextMessage(const ChatMessage& chatMessage, RichChatMessage& richMessage)
{
    // 初始化富文本消息结构
    richMessage.sessionId = chatMessage.sessionId;
    richMessage.senderId = chatMessage.senderId;
    richMessage.senderName = chatMessage.senderName;
    richMessage.senderRole = chatMessage.senderRole;
    richMessage.timestamp = chatMessage.timestamp;
    richMessage.messageType = chatMessage.messageType;
    richMessage.isRead = chatMessage.isRead;
    
    QString fullContent = chatMessage.content;
    
    // 解析富文本标记 - 使用DotMatchesEverything选项处理多行HTML
    QRegularExpression richTextRegex("\\[RICH_TEXT\\](.*?)\\[/RICH_TEXT\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch richTextMatch = richTextRegex.match(fullContent);
    
    QRegularExpression htmlRegex("\\[HTML\\](.*?)\\[/HTML\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch htmlMatch = htmlRegex.match(fullContent);
    
    if (richTextMatch.hasMatch() && htmlMatch.hasMatch()) {
        // 这是一个富文本消息
        richMessage.content = richTextMatch.captured(1);
        richMessage.htmlContent = htmlMatch.captured(1);
        
        // 检查内容类型
        if (richMessage.htmlContent.contains("<img")) {
            richMessage.contentType = RichContentType::Mixed;
        } else {
            richMessage.contentType = RichContentType::RichText;
        }
        
        qDebug() << "解析到富文本消息:" << richMessage.content;
        qDebug() << "HTML内容长度:" << richMessage.htmlContent.length();
        qDebug() << "HTML前100字符:" << richMessage.htmlContent.left(100);
        
        // 如果是完整的HTML文档，提取body内容
        if (richMessage.htmlContent.contains("<!DOCTYPE") && richMessage.htmlContent.contains("<body")) {
            QRegularExpression bodyRegex("<body[^>]*>(.*?)</body>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch bodyMatch = bodyRegex.match(richMessage.htmlContent);
            if (bodyMatch.hasMatch()) {
                richMessage.htmlContent = bodyMatch.captured(1);
                qDebug() << "提取body内容，长度:" << richMessage.htmlContent.length();
            }
        }
        
    } else {
        // 回退到普通文本
        richMessage.content = fullContent;
        richMessage.htmlContent = fullContent;
        richMessage.contentType = RichContentType::Text;
        qDebug() << "未找到富文本标记，使用普通文本模式";
    }
}

// ========== 会话统计和评价管理 ==========

void StaffChatManager::updateSessionStats()
{
    if (m_currentUser.id <= 0 || !m_ratingLabel) return;
    
    // 获取平均评分
    double avgRating = m_dbManager->getStaffAverageRating(m_currentUser.id);
    
    // 获取评价数量
    QList<SessionRating> ratings = m_dbManager->getStaffRatings(m_currentUser.id);
    int ratingCount = ratings.size();
    
    if (ratingCount > 0) {
        m_ratingLabel->setText(QString("服务评价: ⭐%.1f (%2条评价)").arg(avgRating).arg(ratingCount));
    } else {
        m_ratingLabel->setText("服务评价: 暂无评价");
    }
}

void StaffChatManager::showMyRatings()
{
    if (m_currentUser.id <= 0) return;
    
    QList<SessionRating> ratings = m_dbManager->getStaffRatings(m_currentUser.id);
    
    if (ratings.isEmpty()) {
        QMessageBox::information(this, "评价详情", "您暂时还没有收到任何评价。");
        return;
    }
    
    // 创建评价详情对话框
    QDialog* ratingsDialog = new QDialog(this);
    ratingsDialog->setWindowTitle("我的服务评价");
    ratingsDialog->setModal(true);
    ratingsDialog->resize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(ratingsDialog);
    
    // 标题和统计
    double avgRating = m_dbManager->getStaffAverageRating(m_currentUser.id);
    QLabel* titleLabel = new QLabel(QString("📊 服务评价统计 - 平均评分: ⭐%.1f").arg(avgRating));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // 评价列表
    QScrollArea* scrollArea = new QScrollArea;
    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    
    for (const SessionRating& rating : ratings) {
        QFrame* ratingFrame = new QFrame;
        ratingFrame->setFrameStyle(QFrame::Box);
        ratingFrame->setStyleSheet(R"(
            QFrame {
                border: 1px solid #E8F4FD;
                border-radius: 8px;
                background-color: #F8FBFF;
                padding: 8px;
                margin: 2px;
            }
        )");
        
        QVBoxLayout* frameLayout = new QVBoxLayout(ratingFrame);
        frameLayout->setSpacing(5);
        
        // 评分显示
        QString stars = "";
        for (int i = 0; i < rating.rating; ++i) {
            stars += "⭐";
        }
        for (int i = rating.rating; i < 5; ++i) {
            stars += "☆";
        }
        
        QLabel* starLabel = new QLabel(QString("%1 (%2分)").arg(stars).arg(rating.rating));
        starLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #E67E22;");
        frameLayout->addWidget(starLabel);
        
        // 评价时间
        QLabel* timeLabel = new QLabel(rating.createdAt.toString("yyyy-MM-dd hh:mm:ss"));
        timeLabel->setStyleSheet("font-size: 11px; color: #7F8C8D;");
        frameLayout->addWidget(timeLabel);
        
        // 评价内容
        if (!rating.comment.isEmpty()) {
            QLabel* commentLabel = new QLabel(rating.comment);
            commentLabel->setWordWrap(true);
            commentLabel->setStyleSheet("font-size: 12px; color: #34495E; padding: 5px;");
            frameLayout->addWidget(commentLabel);
        } else {
            QLabel* noCommentLabel = new QLabel("（未留言）");
            noCommentLabel->setStyleSheet("font-size: 12px; color: #95A5A6; font-style: italic;");
            frameLayout->addWidget(noCommentLabel);
        }
        
        scrollLayout->addWidget(ratingFrame);
    }
    
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    // 关闭按钮
    QPushButton* closeButton = new QPushButton("关闭");
    UIStyleManager::applyButtonStyle(closeButton, "primary");
    connect(closeButton, &QPushButton::clicked, ratingsDialog, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
    
    ratingsDialog->exec();
    ratingsDialog->deleteLater();
}

void StaffChatManager::setupCustomerInfoPanel()
{
    m_customerInfoGroup = new QGroupBox("客户信息", this);
    m_customerInfoGroup->setVisible(false); // 初始隐藏
    UIStyleManager::applyGroupBoxStyle(m_customerInfoGroup);
    m_customerInfoGroup->setMaximumHeight(120);
    
    QGridLayout* infoLayout = new QGridLayout(m_customerInfoGroup);
    infoLayout->setContentsMargins(10, 15, 10, 10);
    infoLayout->setSpacing(8);
    
    // 客户姓名
    QLabel* nameTitle = new QLabel("姓名:", this);
    nameTitle->setStyleSheet("font-weight: bold; color: #2C3E50;");
    m_customerNameLabel = new QLabel("--", this);
    m_customerNameLabel->setStyleSheet("color: #34495E;");
    
    // 联系电话
    QLabel* phoneTitle = new QLabel("电话:", this);
    phoneTitle->setStyleSheet("font-weight: bold; color: #2C3E50;");
    m_customerPhoneLabel = new QLabel("--", this);
    m_customerPhoneLabel->setStyleSheet("color: #34495E;");
    
    // 在线状态
    QLabel* statusTitle = new QLabel("状态:", this);
    statusTitle->setStyleSheet("font-weight: bold; color: #2C3E50;");
    m_customerStatusLabel = new QLabel("--", this);
    m_customerStatusLabel->setStyleSheet("color: #27AE60;");
    
    // 会话时长
    QLabel* timeTitle = new QLabel("时长:", this);
    timeTitle->setStyleSheet("font-weight: bold; color: #2C3E50;");
    m_sessionTimeLabel = new QLabel("--", this);
    m_sessionTimeLabel->setStyleSheet("color: #8E44AD;");
    
    // 布局
    infoLayout->addWidget(nameTitle, 0, 0);
    infoLayout->addWidget(m_customerNameLabel, 0, 1);
    infoLayout->addWidget(phoneTitle, 0, 2);
    infoLayout->addWidget(m_customerPhoneLabel, 0, 3);
    infoLayout->addWidget(statusTitle, 1, 0);
    infoLayout->addWidget(m_customerStatusLabel, 1, 1);
    infoLayout->addWidget(timeTitle, 1, 2);
    infoLayout->addWidget(m_sessionTimeLabel, 1, 3);
    
    // 设置列拉伸
    infoLayout->setColumnStretch(1, 1);
    infoLayout->setColumnStretch(3, 1);
    
    m_rightLayout->addWidget(m_customerInfoGroup);
}

void StaffChatManager::setupQuickReplyPanel()
{
    m_quickReplyGroup = new QGroupBox("快捷回复", this);
    m_quickReplyGroup->setVisible(false); // 初始隐藏
    UIStyleManager::applyGroupBoxStyle(m_quickReplyGroup);
    m_quickReplyGroup->setMaximumHeight(80);
    
    QVBoxLayout* quickReplyMainLayout = new QVBoxLayout(m_quickReplyGroup);
    quickReplyMainLayout->setContentsMargins(10, 15, 10, 10);
    quickReplyMainLayout->setSpacing(5);
    
    m_quickReplyWidget = new QWidget(this);
    m_quickReplyLayout = new QHBoxLayout(m_quickReplyWidget);
    m_quickReplyLayout->setContentsMargins(0, 0, 0, 0);
    m_quickReplyLayout->setSpacing(8);
    
    setupQuickReplyButtons();
    
    quickReplyMainLayout->addWidget(m_quickReplyWidget);
    m_rightLayout->addWidget(m_quickReplyGroup);
}

void StaffChatManager::setupQuickReplyButtons()
{
    // 清除现有按钮
    QLayoutItem* child;
    while ((child = m_quickReplyLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    // 预设快捷回复内容
    QStringList quickReplies = {
        "您好，我是客服，有什么可以帮助您的吗？",
        "请稍等，我来为您查询一下",
        "感谢您的耐心等待",
        "您的问题我已经记录，会尽快处理",
        "如还有其他问题，随时联系我们",
        "祝您身体健康！"
    };
    
    for (const QString& reply : quickReplies) {
        QPushButton* button = new QPushButton(this);
        
        // 截取前10个字符作为按钮文本
        QString buttonText = reply.left(10);
        if (reply.length() > 10) {
            buttonText += "...";
        }
        button->setText(buttonText);
        button->setToolTip(reply); // 完整内容作为提示
        button->setMaximumWidth(120);
        button->setMaximumHeight(30);
        
        // 设置按钮样式
        button->setStyleSheet(R"(
            QPushButton {
                background-color: #E8F4FD;
                border: 1px solid #3498DB;
                border-radius: 15px;
                padding: 4px 8px;
                font-size: 11px;
                color: #2980B9;
            }
            QPushButton:hover {
                background-color: #D5DBDB;
                border-color: #2980B9;
            }
            QPushButton:pressed {
                background-color: #BDC3C7;
            }
        )");
        
        // 连接点击事件
        connect(button, &QPushButton::clicked, this, [this, reply]() {
            onQuickReplyClicked(reply);
        });
        
        m_quickReplyLayout->addWidget(button);
    }
    
    m_quickReplyLayout->addStretch();
}

void StaffChatManager::onQuickReplyClicked(const QString& replyText)
{
    if (m_currentSessionId <= 0) {
        return;
    }
    
    // 将快捷回复内容插入到当前的输入框中
    if (m_isRichMode && m_richMessageInput) {
        // 富文本模式
        QTextCursor cursor = m_richMessageInput->textCursor();
        cursor.insertText(replyText);
        m_richMessageInput->setFocus();
    } else if (m_messageInput) {
        // 普通文本模式
        QString currentText = m_messageInput->toPlainText();
        if (!currentText.isEmpty()) {
            currentText += "\n" + replyText;
        } else {
            currentText = replyText;
        }
        m_messageInput->setPlainText(currentText);
        m_messageInput->setFocus();
        
        // 将光标移到末尾
        QTextCursor cursor = m_messageInput->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_messageInput->setTextCursor(cursor);
    }
}

void StaffChatManager::updateCustomerInfo(int sessionId)
{
    if (!m_dbManager || sessionId <= 0) {
        // 隐藏客户信息面板
        if (m_customerInfoGroup) {
            m_customerInfoGroup->setVisible(false);
        }
        return;
    }
    
    // 获取会话信息
    ChatSession session = m_dbManager->getChatSession(sessionId);
    if (session.id <= 0) {
        if (m_customerInfoGroup) {
            m_customerInfoGroup->setVisible(false);
        }
        return;
    }
    
    // 获取患者详细信息
    UserInfo patientInfo = m_dbManager->getUserInfo(session.patientId);
    
    // 更新客户姓名
    if (m_customerNameLabel) {
        QString customerName = patientInfo.realName.isEmpty() ? patientInfo.username : patientInfo.realName;
        m_customerNameLabel->setText(customerName);
    }
    
    // 更新联系电话
    if (m_customerPhoneLabel) {
        QString phone = patientInfo.phone.isEmpty() ? "未提供" : patientInfo.phone;
        m_customerPhoneLabel->setText(phone);
    }
    
    // 更新在线状态
    if (m_customerStatusLabel) {
        // 暂时显示固定状态，因为UserInfo中没有isOnline字段
        QString status = "✅ 会话中";
        m_customerStatusLabel->setText(status);
        m_customerStatusLabel->setStyleSheet("color: #27AE60;");
    }
    
    // 计算会话时长
    if (m_sessionTimeLabel) {
        QDateTime now = QDateTime::currentDateTime();
        qint64 duration = session.createdAt.secsTo(now);
        
        QString timeText;
        if (duration < 60) {
            timeText = QString("%1秒").arg(duration);
        } else if (duration < 3600) {
            timeText = QString("%1分钟").arg(duration / 60);
        } else {
            int hours = duration / 3600;
            int minutes = (duration % 3600) / 60;
            timeText = QString("%1小时%2分钟").arg(hours).arg(minutes);
        }
        m_sessionTimeLabel->setText(timeText);
    }
    
    // 显示客户信息面板
    if (m_customerInfoGroup) {
        m_customerInfoGroup->setVisible(true);
    }
}

// MOC generated automatically 