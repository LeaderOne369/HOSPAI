#include "RealChatWidget.h"
#include "SessionRatingDialog.h"
#include "../common/UIStyleManager.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>
#include <QKeyEvent>
#include <QToolBar>
#include <QColorDialog>
#include <QFileDialog>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextCharFormat>
#include <QTextBrowser>
#include <QDebug>
#include <QBuffer>
#include <QPixmap>
#include <QTextImageFormat>
#include <QRegularExpression>

RealChatWidget::RealChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_richTextToolbar(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_messageInput(nullptr)
    , m_richMessageInput(nullptr)
    , m_sendButton(nullptr)
    , m_toggleRichModeButton(nullptr)
    , m_startChatButton(nullptr)
    , m_rateServiceButton(nullptr)
    , m_currentSessionId(-1)
    , m_dbManager(nullptr)
    , m_messageCheckTimer(new QTimer(this))
    , m_typingTimer(new QTimer(this))
    , m_isConnected(false)
    , m_isTyping(false)
    , m_isRichMode(false)
{
    setupUI();
    
    // 延迟初始化数据库连接，确保数据库已经初始化完成
    QTimer::singleShot(100, this, [this]() {
        m_dbManager = DatabaseManager::instance();
        if (m_dbManager) {
            // 连接数据库信号
            connect(m_dbManager, &DatabaseManager::newMessageReceived, 
                    this, &RealChatWidget::onMessageReceived);
            connect(m_dbManager, &DatabaseManager::sessionCreated, 
                    this, &RealChatWidget::onSessionCreated);
            connect(m_dbManager, &DatabaseManager::sessionUpdated, 
                    this, &RealChatWidget::onSessionUpdated);
            
            // 数据库初始化完成后，更新连接状态
            updateConnectionStatus();
            
            qDebug() << "RealChatWidget: 数据库信号连接成功";
        } else {
            qDebug() << "RealChatWidget: 无法获取数据库管理器实例";
        }
    });
    
    // 定时检查新消息
    connect(m_messageCheckTimer, &QTimer::timeout, this, &RealChatWidget::checkForNewMessages);
    m_messageCheckTimer->start(2000); // 每2秒检查一次
    
    // 用户输入计时器
    connect(m_typingTimer, &QTimer::timeout, this, &RealChatWidget::onUserInput);
    m_typingTimer->setSingleShot(true);
}

RealChatWidget::~RealChatWidget()
{
    if (m_currentUser.id > 0 && m_dbManager) {
        m_dbManager->updateUserOnlineStatus(m_currentUser.id, false);
    }
}

void RealChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // 状态栏区域
    QWidget* statusWidget = new QWidget(this);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(10);
    
    m_statusLabel = new QLabel("💬 客服聊天", this);
    UIStyleManager::applyLabelStyle(m_statusLabel, "title");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    
    // 评价服务按钮
    m_rateServiceButton = new QPushButton("⭐ 评价服务", this);
    m_rateServiceButton->setMaximumWidth(100);
    m_rateServiceButton->setVisible(false); // 初始隐藏
    UIStyleManager::applyButtonStyle(m_rateServiceButton, "secondary");
    connect(m_rateServiceButton, &QPushButton::clicked, this, &RealChatWidget::onManualRating);
    
    statusLayout->addStretch();
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_rateServiceButton);
    
    m_mainLayout->addWidget(statusWidget);
    
    setupRichTextToolbar();
    setupMessageArea();
    setupInputArea();
    
    // 暂时不调用updateConnectionStatus，等数据库初始化完成后再调用
}

void RealChatWidget::setupMessageArea()
{
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
    m_messageScrollArea->setMinimumHeight(400);
    
    UIStyleManager::applyScrollAreaStyle(m_messageScrollArea);
    
    m_mainLayout->addWidget(m_messageScrollArea);
}

void RealChatWidget::setupInputArea()
{
    // 开始聊天按钮
    m_startChatButton = new QPushButton("🚀 开始咨询", this);
    UIStyleManager::applyButtonStyle(m_startChatButton, "primary");
    connect(m_startChatButton, &QPushButton::clicked, this, &RealChatWidget::startNewChat);
    m_mainLayout->addWidget(m_startChatButton);
    
    // 输入区域
    QWidget* inputWidget = new QWidget(this);
    QVBoxLayout* inputMainLayout = new QVBoxLayout(inputWidget);
    inputMainLayout->setContentsMargins(0, 0, 0, 0);
    inputMainLayout->setSpacing(5);
    
    // 模式切换按钮区域
    QHBoxLayout* modeLayout = new QHBoxLayout;
    modeLayout->setContentsMargins(0, 0, 0, 0);
    
    m_toggleRichModeButton = new QPushButton("富文本模式", this);
    m_toggleRichModeButton->setCheckable(true);
    m_toggleRichModeButton->setMaximumWidth(100);
    UIStyleManager::applyButtonStyle(m_toggleRichModeButton, "secondary");
    connect(m_toggleRichModeButton, &QPushButton::clicked, this, &RealChatWidget::onToggleRichMode);
    
    modeLayout->addWidget(m_toggleRichModeButton);
    modeLayout->addStretch();
    inputMainLayout->addLayout(modeLayout);
    
    // 输入框区域
    QHBoxLayout* inputLayout = new QHBoxLayout;
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(10);
    
    // 普通消息输入框
    m_messageInput = new QTextEdit(this);
    m_messageInput->setMaximumHeight(80);
    m_messageInput->setPlaceholderText("请输入您的问题...");
    UIStyleManager::applyTextEditStyle(m_messageInput);
    
    // 富文本消息输入框
    m_richMessageInput = new QTextEdit(this);
    m_richMessageInput->setMaximumHeight(120);
    m_richMessageInput->setPlaceholderText("输入富文本消息，支持格式化、图片和文件...");
    m_richMessageInput->setAcceptDrops(true);
    m_richMessageInput->setAcceptRichText(true); // 启用富文本
    m_richMessageInput->setTextInteractionFlags(Qt::TextEditorInteraction);
    UIStyleManager::applyTextEditStyle(m_richMessageInput);
    m_richMessageInput->hide(); // 初始隐藏
    
    // 发送按钮
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_sendButton, "primary");
    m_sendButton->setEnabled(false);
    
    // 连接文本变化事件
    connect(m_messageInput, &QTextEdit::textChanged, [this]() {
        if (!m_isRichMode) {
            bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
            m_sendButton->setEnabled(hasText && m_isConnected);
            
            if (hasText && !m_isTyping) {
                m_isTyping = true;
                m_typingTimer->start(3000);
            }
        }
    });
    
    connect(m_richMessageInput, &QTextEdit::textChanged, [this]() {
        if (m_isRichMode) {
            bool hasText = !m_richMessageInput->toPlainText().trimmed().isEmpty();
            m_sendButton->setEnabled(hasText && m_isConnected);
            
            if (hasText && !m_isTyping) {
                m_isTyping = true;
                m_typingTimer->start(3000);
            }
        }
    });
    
    // 连接发送按钮
    connect(m_sendButton, &QPushButton::clicked, [this]() {
        if (m_isRichMode) {
            onSendRichMessage();
        } else {
            onSendMessage();
        }
    });
    
    // 键盘事件
    m_messageInput->installEventFilter(this);
    m_richMessageInput->installEventFilter(this);
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_richMessageInput);
    inputLayout->addWidget(m_sendButton);
    
    inputMainLayout->addLayout(inputLayout);
    
    inputWidget->setVisible(false); // 初始隐藏
    m_mainLayout->addWidget(inputWidget);
}

void RealChatWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新在线状态
    if (m_dbManager) {
        m_dbManager->updateUserOnlineStatus(user.id, true);
    }
    
    // 加载聊天历史
    loadChatHistory();
}

void RealChatWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    if (m_dbManager) {
        // 连接数据库信号
        connect(m_dbManager, &DatabaseManager::newMessageReceived, 
                this, &RealChatWidget::onMessageReceived);
        connect(m_dbManager, &DatabaseManager::sessionCreated, 
                this, &RealChatWidget::onSessionCreated);
        connect(m_dbManager, &DatabaseManager::sessionUpdated, 
                this, &RealChatWidget::onSessionUpdated);
    }
    
    updateConnectionStatus();
}

void RealChatWidget::startNewChat()
{
    if (m_currentUser.id <= 0) {
        QMessageBox::warning(this, "错误", "请先登录！");
        return;
    }
    
    if (!m_dbManager) {
        QMessageBox::warning(this, "错误", "数据库连接未就绪，请稍后再试！");
        return;
    }
    
    // 创建新的聊天会话
    m_currentSessionId = m_dbManager->createChatSession(m_currentUser.id);
    
    if (m_currentSessionId > 0) {
        m_isConnected = true;
        m_startChatButton->setVisible(false);
        m_mainLayout->itemAt(m_mainLayout->count() - 1)->widget()->setVisible(true);
        
        updateConnectionStatus();
        
        // 添加欢迎消息
        ChatMessage welcomeMsg;
        welcomeMsg.id = 0;
        welcomeMsg.sessionId = m_currentSessionId;
        welcomeMsg.senderId = 0;
        welcomeMsg.senderName = "系统";
        welcomeMsg.senderRole = "system";
        welcomeMsg.content = "您好！请描述您的问题，我们会尽快为您安排客服。";
        welcomeMsg.timestamp = QDateTime::currentDateTime();
        welcomeMsg.messageType = 1;
        welcomeMsg.isRead = 1;
        
        addMessage(welcomeMsg);
    } else {
        QMessageBox::warning(this, "错误", "无法创建聊天会话！");
    }
}

void RealChatWidget::onSendMessage()
{
    QString content = m_messageInput->toPlainText().trimmed();
    if (content.isEmpty() || m_currentSessionId <= 0) {
        return;
    }
    
    if (!m_dbManager) {
        qDebug() << "RealChatWidget::onSendMessage: 数据库管理器为空";
        return;
    }
    
    // 发送消息到数据库
    int messageId = m_dbManager->sendMessage(m_currentSessionId, m_currentUser.id, content);
    
    if (messageId > 0) {
        m_messageInput->clear();
        m_isTyping = false;
        
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

void RealChatWidget::onMessageReceived(const ChatMessage& message)
{
    // 只显示当前会话的消息，且不是自己发送的
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addMessage(message);
        
        // 标记消息为已读
        if (m_dbManager) {
            m_dbManager->markMessageAsRead(message.id);
        }
    }
}

void RealChatWidget::onSessionCreated(const ChatSession& session)
{
    if (session.patientId == m_currentUser.id) {
        updateConnectionStatus();
    }
}

void RealChatWidget::onSessionUpdated(const ChatSession& session)
{
    if (session.id == m_currentSessionId) {
        updateConnectionStatus();
        
        // 如果会话结束，显示评价对话框
        if (session.status == 0 && session.staffId > 0) {
            // 延迟一下显示评价对话框，让用户有时间看到会话结束消息
            QTimer::singleShot(2000, this, [this, session]() {
                showRatingDialog(session);
            });
        }
    }
}

void RealChatWidget::checkForNewMessages()
{
    if (m_currentSessionId <= 0 || !m_dbManager) return;
    
    // 获取未读消息
    QList<ChatMessage> unreadMessages = m_dbManager->getUnreadMessages(m_currentUser.id);
    
    for (const ChatMessage& message : unreadMessages) {
        if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
            addMessage(message);
            m_dbManager->markMessageAsRead(message.id);
        }
    }
    
    // 同时检查会话状态是否有变化
    updateConnectionStatus();
}

void RealChatWidget::onUserInput()
{
    m_isTyping = false;
}

void RealChatWidget::addMessage(const ChatMessage& message)
{
    QWidget* messageBubble = createMessageBubble(message);
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &RealChatWidget::scrollToBottom);
    
    m_lastMessageTime = message.timestamp;
    
    // 如果是系统消息（比如客服接入通知），更新连接状态
    if (message.messageType == 1 && message.content.contains("已接入")) {
        QTimer::singleShot(100, this, &RealChatWidget::updateConnectionStatus);
    }
}

QWidget* RealChatWidget::createMessageBubble(const ChatMessage& message)
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
    
    // 检查是否是富文本消息
    bool isRichText = message.content.contains("[RICH_TEXT]") && message.content.contains("[HTML]");
    
    if (isRichText) {
        // 使用QTextBrowser显示富文本内容
        QTextBrowser* contentBrowser = new QTextBrowser;
        contentBrowser->setMaximumHeight(300);
        contentBrowser->setOpenExternalLinks(false);
        contentBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        contentBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        // 解析富文本内容
        QString htmlContent = parseHTMLFromMessage(message.content);
        if (!htmlContent.isEmpty()) {
            contentBrowser->setHtml(htmlContent);
            qDebug() << "患者端显示富文本消息，HTML长度:" << htmlContent.length();
        } else {
            // 回退到纯文本
            QString plainText = parsePlainTextFromMessage(message.content);
            contentBrowser->setPlainText(plainText.isEmpty() ? message.content : plainText);
        }
        
        frameLayout->addWidget(contentBrowser);
    } else {
        // 普通文本消息
        QLabel* contentLabel = new QLabel(message.content);
        contentLabel->setWordWrap(true);
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        frameLayout->addWidget(contentLabel);
    }
    
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
        
        if (isRichText) {
            QTextBrowser* contentBrowser = qobject_cast<QTextBrowser*>(frameLayout->itemAt(1)->widget());
            if (contentBrowser) {
                contentBrowser->setStyleSheet("color: white; font-size: 14px; background: transparent; border: none;");
            }
        } else {
            QLabel* contentLabel = qobject_cast<QLabel*>(frameLayout->itemAt(1)->widget());
            if (contentLabel) {
                contentLabel->setStyleSheet("color: white; font-size: 14px;");
            }
        }
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
        
        if (isRichText) {
            QTextBrowser* contentBrowser = qobject_cast<QTextBrowser*>(frameLayout->itemAt(1)->widget());
            if (contentBrowser) {
                contentBrowser->setStyleSheet("color: #8E8E93; font-size: 13px; text-align: center; background: transparent; border: none;");
            }
        } else {
            QLabel* contentLabel = qobject_cast<QLabel*>(frameLayout->itemAt(1)->widget());
            if (contentLabel) {
                contentLabel->setStyleSheet("color: #8E8E93; font-size: 13px; text-align: center;");
            }
        }
        
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
        
        if (isRichText) {
            QTextBrowser* contentBrowser = qobject_cast<QTextBrowser*>(frameLayout->itemAt(1)->widget());
            if (contentBrowser) {
                contentBrowser->setStyleSheet("color: #000000; font-size: 14px; background: transparent; border: none;");
            }
        } else {
            QLabel* contentLabel = qobject_cast<QLabel*>(frameLayout->itemAt(1)->widget());
            if (contentLabel) {
                contentLabel->setStyleSheet("color: #000000; font-size: 14px;");
            }
        }
    }
    
    return bubble;
}

void RealChatWidget::scrollToBottom()
{
    QScrollBar* scrollBar = m_messageScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void RealChatWidget::updateConnectionStatus()
{
    if (m_currentSessionId <= 0 || !m_dbManager) {
        m_statusLabel->setText("💬 客服聊天 - 未连接");
        if (m_rateServiceButton) {
            m_rateServiceButton->setVisible(false);
        }
        return;
    }
    
    ChatSession session = m_dbManager->getChatSession(m_currentSessionId);
    
    switch (session.status) {
        case 0:
            m_statusLabel->setText("💬 客服聊天 - 会话已结束");
            // 会话结束时，隐藏评价按钮
            if (m_rateServiceButton) {
                m_rateServiceButton->setVisible(false);
            }
            break;
        case 1:
            if (!session.staffName.isEmpty()) {
                m_statusLabel->setText(QString("💬 与 %1 对话中").arg(session.staffName));
            } else {
                m_statusLabel->setText("💬 与客服对话中");
            }
            // 与客服对话中时显示评价按钮
            if (m_rateServiceButton) {
                m_rateServiceButton->setVisible(true);
            }
            break;
        case 2:
            m_statusLabel->setText("💬 客服聊天 - 等待客服接入...");
            // 等待客服时隐藏评价按钮
            if (m_rateServiceButton) {
                m_rateServiceButton->setVisible(false);
            }
            break;
        default:
            m_statusLabel->setText("💬 客服聊天");
            if (m_rateServiceButton) {
                m_rateServiceButton->setVisible(false);
            }
            break;
    }
}

void RealChatWidget::loadChatHistory()
{
    if (m_currentUser.id <= 0 || !m_dbManager) {
        qDebug() << "RealChatWidget::loadChatHistory: 用户ID无效或数据库管理器为空";
        return;
    }
    
    // 获取用户的最近会话
    QList<ChatSession> sessions = m_dbManager->getPatientSessions(m_currentUser.id);
    
    if (!sessions.isEmpty()) {
        ChatSession lastSession = sessions.first();
        if (lastSession.status > 0) {
            // 如果有进行中的会话，自动加载
            m_currentSessionId = lastSession.id;
            m_isConnected = true;
            m_startChatButton->setVisible(false);
            m_mainLayout->itemAt(m_mainLayout->count() - 1)->widget()->setVisible(true);
            
            // 加载消息历史
            QList<ChatMessage> messages = m_dbManager->getChatMessages(m_currentSessionId);
            for (const ChatMessage& message : messages) {
                addMessage(message);
            }
            
            updateConnectionStatus();
        }
    }
}

QString RealChatWidget::formatTime(const QDateTime& time)
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

bool RealChatWidget::eventFilter(QObject* obj, QEvent* event)
{
    if ((obj == m_messageInput || obj == m_richMessageInput) && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                // 不是 Shift+Enter，发送消息
                if (m_isRichMode) {
                    onSendRichMessage();
                } else {
                    onSendMessage();
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// 富文本功能实现

void RealChatWidget::setupRichTextToolbar()
{
    m_richTextToolbar = new QToolBar("Rich Text", this);
    m_richTextToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_richTextToolbar->setMovable(false);
    m_richTextToolbar->hide(); // 初始隐藏
    
    // 格式化按钮
    m_actionBold = m_richTextToolbar->addAction("B", this, &RealChatWidget::onTextBold);
    m_actionBold->setCheckable(true);
    m_actionBold->setToolTip("加粗");
    m_actionBold->setShortcut(QKeySequence::Bold);
    
    m_actionItalic = m_richTextToolbar->addAction("I", this, &RealChatWidget::onTextItalic);
    m_actionItalic->setCheckable(true);
    m_actionItalic->setToolTip("斜体");
    m_actionItalic->setShortcut(QKeySequence::Italic);
    
    m_actionUnderline = m_richTextToolbar->addAction("U", this, &RealChatWidget::onTextUnderline);
    m_actionUnderline->setCheckable(true);
    m_actionUnderline->setToolTip("下划线");
    m_actionUnderline->setShortcut(QKeySequence::Underline);
    
    m_actionStrikeOut = m_richTextToolbar->addAction("S", this, &RealChatWidget::onTextStrikeOut);
    m_actionStrikeOut->setCheckable(true);
    m_actionStrikeOut->setToolTip("删除线");
    
    m_richTextToolbar->addSeparator();
    
    // 颜色按钮
    m_actionTextColor = m_richTextToolbar->addAction("A", this, &RealChatWidget::onTextColor);
    m_actionTextColor->setToolTip("文字颜色");
    
    m_actionBackgroundColor = m_richTextToolbar->addAction("□", this, &RealChatWidget::onTextBackgroundColor);
    m_actionBackgroundColor->setToolTip("背景颜色");
    
    m_richTextToolbar->addSeparator();
    
    // 字体选择
    m_fontComboBox = new QFontComboBox;
    m_fontComboBox->setMaximumWidth(150);
    connect(m_fontComboBox, &QFontComboBox::textActivated, this, &RealChatWidget::onFontFamilyChanged);
    m_richTextToolbar->addWidget(m_fontComboBox);
    
    // 字体大小
    m_fontSizeSpinBox = new QSpinBox;
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(12);
    m_fontSizeSpinBox->setMaximumWidth(80);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &RealChatWidget::onFontSizeChanged);
    m_richTextToolbar->addWidget(m_fontSizeSpinBox);
    
    m_richTextToolbar->addSeparator();
    
    // 插入功能
    m_actionInsertImage = m_richTextToolbar->addAction("📷", this, &RealChatWidget::onInsertImage);
    m_actionInsertImage->setToolTip("插入图片");
    
    m_actionInsertFile = m_richTextToolbar->addAction("📎", this, &RealChatWidget::onInsertFile);
    m_actionInsertFile->setToolTip("插入文件");
    
    m_mainLayout->addWidget(m_richTextToolbar);
}

void RealChatWidget::onToggleRichMode()
{
    m_isRichMode = m_toggleRichModeButton->isChecked();
    
    if (m_isRichMode) {
        // 切换到富文本模式
        m_messageInput->hide();
        m_richMessageInput->show();
        m_richTextToolbar->show();
        m_toggleRichModeButton->setText("普通模式");
        
        // 同步内容
        QString plainText = m_messageInput->toPlainText();
        if (!plainText.isEmpty()) {
            m_richMessageInput->setPlainText(plainText);
            m_messageInput->clear();
        }
    } else {
        // 切换到普通模式
        m_richMessageInput->hide();
        m_messageInput->show();
        m_richTextToolbar->hide();
        m_toggleRichModeButton->setText("富文本模式");
        
        // 同步内容
        QString plainText = m_richMessageInput->toPlainText();
        if (!plainText.isEmpty()) {
            m_messageInput->setPlainText(plainText);
            m_richMessageInput->clear();
        }
    }
    
    // 更新发送按钮状态
    QTextEdit* currentInput = m_isRichMode ? m_richMessageInput : m_messageInput;
    bool hasText = !currentInput->toPlainText().trimmed().isEmpty();
    m_sendButton->setEnabled(hasText && m_isConnected);
}

void RealChatWidget::onTextBold()
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontWeight(m_actionBold->isChecked() ? QFont::Bold : QFont::Normal);
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onTextItalic()
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontItalic(m_actionItalic->isChecked());
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onTextUnderline()
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontUnderline(m_actionUnderline->isChecked());
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onTextStrikeOut()
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontStrikeOut(m_actionStrikeOut->isChecked());
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onTextColor()
{
    if (!m_richMessageInput) return;
    QColor color = QColorDialog::getColor(Qt::black, this, "选择文字颜色");
    if (color.isValid()) {
        QTextCharFormat format;
        format.setForeground(color);
        m_richMessageInput->textCursor().mergeCharFormat(format);
        m_richMessageInput->setFocus();
    }
}

void RealChatWidget::onTextBackgroundColor()
{
    if (!m_richMessageInput) return;
    QColor color = QColorDialog::getColor(Qt::white, this, "选择背景颜色");
    if (color.isValid()) {
        QTextCharFormat format;
        format.setBackground(color);
        m_richMessageInput->textCursor().mergeCharFormat(format);
        m_richMessageInput->setFocus();
    }
}

void RealChatWidget::onFontFamilyChanged(const QString& fontFamily)
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontFamilies({fontFamily});
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onFontSizeChanged(int size)
{
    if (!m_richMessageInput) return;
    QTextCharFormat format;
    format.setFontPointSize(size);
    m_richMessageInput->textCursor().mergeCharFormat(format);
    m_richMessageInput->setFocus();
}

void RealChatWidget::onInsertImage()
{
    if (!m_richMessageInput) return;
    
    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", 
        "", "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp)");
    
    if (!imagePath.isEmpty()) {
        insertImageIntoEditor(imagePath);
    }
}

void RealChatWidget::onInsertFile()
{
    if (!m_richMessageInput) return;
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", 
        "", "所有文件 (*.*)");
    
    if (!filePath.isEmpty()) {
        insertFileIntoEditor(filePath);
    }
}

void RealChatWidget::insertImageIntoEditor(const QString& imagePath)
{
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) return;
    
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) return;
    
    // 自动调整图片大小
    int maxWidth = 300;
    int maxHeight = 300;
    if (pixmap.width() > maxWidth || pixmap.height() > maxHeight) {
        pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 将图片转换为Base64编码，确保可以跨端传输
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    QString base64Data = imageData.toBase64();
    
    // 创建包含Base64数据的HTML
    QTextCursor cursor = m_richMessageInput->textCursor();
    QString imageHtml = QString("<img src=\"data:image/png;base64,%1\" style=\"max-width:300px; max-height:300px;\" />").arg(base64Data);
    cursor.insertHtml(imageHtml);
    
    // 也保存原始图片用于记录
    // cursor.insertImage(pixmap.toImage(), imagePath);
}

void RealChatWidget::insertFileIntoEditor(const QString& filePath)
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

void RealChatWidget::onSendRichMessage()
{
    if (!m_richMessageInput || m_richMessageInput->toPlainText().trimmed().isEmpty()) {
        return;
    }
    
    if (m_currentSessionId <= 0) {
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
    
    // 保存消息
    saveRichChatHistory(message);
    
    // 显示消息
    addRichMessage(message);
    
    // 清空输入框
    m_richMessageInput->clear();
    m_isTyping = false;
}

void RealChatWidget::onRichMessageReceived(const RichChatMessage& message)
{
    // 只显示当前会话的消息，且不是自己发送的
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addRichMessage(message);
        
        // TODO: 标记富文本消息为已读
    }
}

void RealChatWidget::addRichMessage(const RichChatMessage& message)
{
    QWidget* messageBubble = createRichMessageBubble(message);
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &RealChatWidget::scrollToBottom);
    
    m_lastMessageTime = message.timestamp;
}

QWidget* RealChatWidget::createRichMessageBubble(const RichChatMessage& message)
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

void RealChatWidget::saveRichChatHistory(const RichChatMessage& message)
{
    if (!m_dbManager) return;
    
    // 添加到富文本聊天记录
    m_richChatHistory.append(message);
    
    // 构建包含富文本信息的消息内容
    QString enhancedContent = message.content;
    
    // 如果是富文本或混合内容，在消息前添加特殊标记
    if (message.contentType == RichContentType::RichText || 
        message.contentType == RichContentType::Mixed) {
        // 添加富文本标记和HTML内容，客服端可以识别并正确显示
        enhancedContent = QString("[RICH_TEXT]%1[/RICH_TEXT][HTML]%2[/HTML]")
                         .arg(message.content)
                         .arg(message.htmlContent);
    }
    
    // 转换为普通ChatMessage以保持兼容性
    ChatMessage tempMessage;
    tempMessage.sessionId = message.sessionId;
    tempMessage.senderId = message.senderId;
    tempMessage.senderName = message.senderName;
    tempMessage.senderRole = message.senderRole;
    tempMessage.content = enhancedContent; // 使用增强的内容
    tempMessage.timestamp = message.timestamp;
    tempMessage.messageType = message.messageType;
    tempMessage.isRead = message.isRead;
    
    // 保存消息
    int messageId = m_dbManager->sendMessage(tempMessage.sessionId, tempMessage.senderId, 
                                           tempMessage.content, tempMessage.messageType);
    
    if (messageId > 0) {
        qDebug() << "富文本消息已保存，ID:" << messageId << "内容类型:" << (int)message.contentType;
    }
}

void RealChatWidget::loadRichChatHistory()
{
    // TODO: 从数据库加载富文本记录
    // 当前暂时使用普通消息记录
}

QString RealChatWidget::convertToRichText(const QString& plainText)
{
    // 将纯文本转换为基本的富文本格式
    QString richText = plainText;
    
    // 转换换行符为HTML格式
    richText.replace('\n', "<br>");
    
    return richText;
}

void RealChatWidget::showRatingDialog(const ChatSession& session)
{
    if (!m_dbManager) {
        qDebug() << "RealChatWidget::showRatingDialog: 数据库管理器为空";
        return;
    }
    
    // 检查是否已经评价过
    if (m_dbManager->hasSessionRating(session.id)) {
        qDebug() << "会话已经被评价过，跳过评价:" << session.id;
        return;
    }
    
    // 创建评价对话框
    SessionRatingDialog* ratingDialog = new SessionRatingDialog(
        session.id, 
        session.patientId, 
        session.staffId, 
        session.staffName.isEmpty() ? "客服" : session.staffName,
        this
    );
    
    // 显示评价对话框
    if (ratingDialog->exec() == QDialog::Accepted) {
        qDebug() << "用户提交了评价，评分:" << ratingDialog->getRating();
        
        // 显示感谢消息
        QMessageBox::information(this, "感谢", 
            QString("感谢您为 %1 的服务评价！\n您的反馈对我们很重要。")
            .arg(session.staffName.isEmpty() ? "客服" : session.staffName));
    } else {
        qDebug() << "用户跳过了评价";
    }
    
    ratingDialog->deleteLater();
}

QString RealChatWidget::parseHTMLFromMessage(const QString& content)
{
    // 解析 [HTML]...[/HTML] 标记中的内容
    QRegularExpression htmlRegex("\\[HTML\\](.*?)\\[/HTML\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch htmlMatch = htmlRegex.match(content);
    
    if (htmlMatch.hasMatch()) {
        QString htmlContent = htmlMatch.captured(1);
        
        // 如果是完整的HTML文档，提取body内容
        if (htmlContent.contains("<!DOCTYPE") && htmlContent.contains("<body")) {
            QRegularExpression bodyRegex("<body[^>]*>(.*?)</body>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch bodyMatch = bodyRegex.match(htmlContent);
            if (bodyMatch.hasMatch()) {
                htmlContent = bodyMatch.captured(1);
            }
        }
        
        qDebug() << "患者端解析HTML内容，长度:" << htmlContent.length();
        return htmlContent;
    }
    
    return QString();
}

QString RealChatWidget::parsePlainTextFromMessage(const QString& content)
{
    // 解析 [RICH_TEXT]...[/RICH_TEXT] 标记中的内容
    QRegularExpression richTextRegex("\\[RICH_TEXT\\](.*?)\\[/RICH_TEXT\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch richTextMatch = richTextRegex.match(content);
    
    if (richTextMatch.hasMatch()) {
        return richTextMatch.captured(1);
    }
    
    return QString();
}

void RealChatWidget::onManualRating()
{
    if (m_currentSessionId <= 0 || !m_dbManager) {
        QMessageBox::warning(this, "提示", "当前没有活跃的会话可以评价。");
        return;
    }
    
    // 获取当前会话信息
    ChatSession session = m_dbManager->getChatSession(m_currentSessionId);
    
    if (session.id <= 0) {
        QMessageBox::warning(this, "错误", "无法获取当前会话信息。");
        return;
    }
    
    // 检查会话是否有客服参与
    if (session.staffId <= 0) {
        QMessageBox::information(this, "提示", "会话还未分配客服，暂时无法评价。");
        return;
    }
    
    // 检查是否已经评价过
    if (m_dbManager->hasSessionRating(session.id)) {
        int ret = QMessageBox::question(this, "提示", 
                                       "您已经对此次会话进行过评价了。\n确定要重新评价吗？", 
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::No) {
            return;
        }
    }
    
    // 创建并显示评价对话框
    SessionRatingDialog* ratingDialog = new SessionRatingDialog(
        session.id, 
        session.patientId, 
        session.staffId, 
        session.staffName.isEmpty() ? "客服" : session.staffName,
        this
    );
    
    // 设置对话框标题，表明这是手动评价
    ratingDialog->setWindowTitle("评价客服服务");
    
    if (ratingDialog->exec() == QDialog::Accepted) {
        // 评价提交成功
        QMessageBox::information(this, "感谢", 
            QString("感谢您为 %1 的服务评价！\n您的反馈对我们改进服务非常重要。")
            .arg(session.staffName.isEmpty() ? "客服" : session.staffName));
    }
    
    ratingDialog->deleteLater();
}

// MOC generated automatically 