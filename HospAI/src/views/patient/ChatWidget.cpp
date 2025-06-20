#include "ChatWidget.h"
#include <QGroupBox>
#include <QScrollBar>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QUuid>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QDebug>
#include <QToolBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QColorDialog>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QRandomGenerator>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolBarLayout(nullptr)
    , m_btnClearChat(nullptr)
    , m_btnSaveChat(nullptr)
    , m_btnSettings(nullptr)
    , m_btnToggleRichMode(nullptr)
    , m_statusLabel(nullptr)
    , m_richTextToolbar(nullptr)
    , m_chatScrollArea(nullptr)
    , m_chatContainer(nullptr)
    , m_chatLayout(nullptr)
    , m_quickButtonsGroup(nullptr)
    , m_quickButtonsLayout(nullptr)
    , m_quickButtonGroup(nullptr)
    , m_interactionWidget(nullptr)
    , m_interactionLayout(nullptr)
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_messageInput(nullptr)
    , m_richMessageInput(nullptr)
    , m_btnSend(nullptr)
    , m_btnVoice(nullptr)
    , m_btnEmoji(nullptr)
    , m_typingTimer(nullptr)
    , m_responseTimer(nullptr)
    , m_isAITyping(false)
    , m_isInitialized(false)
    , m_isRichMode(false)
    , m_messageCount(0)
    , m_dbManager(nullptr)
    , m_aiApiClient(new AIApiClient(this))
{
    initDatabase();
    setupUI();
    setupQuickButtons();
    
    m_currentSessionId = generateSessionId();
    m_isInitialized = true;
    
    // 连接AI API客户端信号
    connect(m_aiApiClient, &AIApiClient::triageResponseReceived,
            this, &ChatWidget::onAITriageResponse);
    connect(m_aiApiClient, &AIApiClient::apiError,
            this, &ChatWidget::onAIApiError);
    connect(m_aiApiClient, &AIApiClient::requestStarted, [this]() {
        m_isAITyping = true;
        m_statusLabel->setText("智能分诊助手正在分析中...");
        m_btnSend->setEnabled(false);
    });
    connect(m_aiApiClient, &AIApiClient::requestFinished, [this]() {
        m_isAITyping = false;
        m_statusLabel->setText("智能分诊助手");
        m_btnSend->setEnabled(true);
    });
    
    // 发送欢迎消息
    QTimer::singleShot(500, [this]() {
        AIMessage welcomeMsg;
        welcomeMsg.content = "👋 您好！我是AI智能导诊助手，可以帮助您：\n\n"
                            "🏥 科室推荐和医生介绍\n"
                            "📅 预约挂号服务\n" 
                            "💡 健康咨询和症状分析\n"
                            "🆘 紧急情况处理指导\n\n"
                            "请描述您的症状或选择下方快捷咨询选项：";
        welcomeMsg.type = MessageType::Robot;
        welcomeMsg.timestamp = QDateTime::currentDateTime();
        welcomeMsg.sessionId = m_currentSessionId;
        addMessage(welcomeMsg);
    });
}

ChatWidget::~ChatWidget()
{
    if (m_database.isOpen()) {
        saveChatHistory();
        m_database.close();
    }
}

void ChatWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
}

void ChatWidget::setUserInfo(const QString& userId, const QString& userName)
{
    m_userId = userId;
    m_userName = userName;
    qDebug() << "ChatWidget 设置用户信息 - ID:" << userId << "名称:" << userName;
}

void ChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    setupToolBar();
    setupRichTextToolbar();
    setupChatArea();
    setupQuickButtonsArea();
    setupInputArea();
    
    // 应用整体样式
    setStyleSheet(R"(
        ChatWidget {
            background-color: #F8F9FA;
        }
    )");
}

void ChatWidget::setupToolBar()
{
    m_toolBarLayout = new QHBoxLayout;
    m_toolBarLayout->setSpacing(10);
    
    m_btnClearChat = new QPushButton("清空对话");
    m_btnSaveChat = new QPushButton("保存对话");
    m_btnSettings = new QPushButton("设置");
    m_btnToggleRichMode = new QPushButton("富文本模式");
    m_btnToggleRichMode->setCheckable(true);
    
    m_statusLabel = new QLabel("AI智能导诊");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    
    QString buttonStyle = R"(
        QPushButton {
            background-color: white;
            border: 1px solid #D1D1D6;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            color: #1D1D1F;
        }
        QPushButton:hover {
            background-color: #F2F2F7;
            border-color: #007AFF;
        }
        QPushButton:checked {
            background-color: #007AFF;
            color: white;
        }
    )";
    
    m_btnClearChat->setStyleSheet(buttonStyle);
    m_btnSaveChat->setStyleSheet(buttonStyle);
    m_btnSettings->setStyleSheet(buttonStyle);
    m_btnToggleRichMode->setStyleSheet(buttonStyle);
    
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            font-size: 16px;
            font-weight: bold;
            color: #1D1D1F;
            padding: 8px;
        }
    )");
    
    connect(m_btnClearChat, &QPushButton::clicked, this, &ChatWidget::onClearChatClicked);
    connect(m_btnSaveChat, &QPushButton::clicked, this, &ChatWidget::onSaveChatClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &ChatWidget::onSettingsClicked);
    connect(m_btnToggleRichMode, &QPushButton::clicked, this, &ChatWidget::onToggleRichMode);
    
    m_toolBarLayout->addWidget(m_btnClearChat);
    m_toolBarLayout->addWidget(m_btnSaveChat);
    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_statusLabel);
    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_btnToggleRichMode);
    m_toolBarLayout->addWidget(m_btnSettings);
    
    m_mainLayout->addLayout(m_toolBarLayout);
}

void ChatWidget::setupRichTextToolbar()
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
            this, &ChatWidget::onFontSizeChanged);
    
    // 格式化按钮
    m_actionBold = m_richTextToolbar->addAction("B");
    m_actionBold->setCheckable(true);
    m_actionBold->setToolTip("粗体");
    connect(m_actionBold, &QAction::triggered, this, &ChatWidget::onTextBold);
    
    m_actionItalic = m_richTextToolbar->addAction("I");
    m_actionItalic->setCheckable(true);
    m_actionItalic->setToolTip("斜体");
    connect(m_actionItalic, &QAction::triggered, this, &ChatWidget::onTextItalic);
    
    m_actionUnderline = m_richTextToolbar->addAction("U");
    m_actionUnderline->setCheckable(true);
    m_actionUnderline->setToolTip("下划线");
    connect(m_actionUnderline, &QAction::triggered, this, &ChatWidget::onTextUnderline);
    
    m_actionStrikeOut = m_richTextToolbar->addAction("S");
    m_actionStrikeOut->setCheckable(true);
    m_actionStrikeOut->setToolTip("删除线");
    connect(m_actionStrikeOut, &QAction::triggered, this, &ChatWidget::onTextStrikeOut);
    
    m_richTextToolbar->addSeparator();
    
    // 颜色按钮
    m_actionTextColor = m_richTextToolbar->addAction("🎨");
    m_actionTextColor->setToolTip("文字颜色");
    connect(m_actionTextColor, &QAction::triggered, this, &ChatWidget::onTextColor);
    
    m_actionBackgroundColor = m_richTextToolbar->addAction("🖍️");
    m_actionBackgroundColor->setToolTip("背景颜色");
    connect(m_actionBackgroundColor, &QAction::triggered, this, &ChatWidget::onTextBackgroundColor);
    
    m_richTextToolbar->addSeparator();
    
    // 插入功能
    m_actionInsertImage = m_richTextToolbar->addAction("🖼️");
    m_actionInsertImage->setToolTip("插入图片");
    connect(m_actionInsertImage, &QAction::triggered, this, &ChatWidget::onInsertImage);
    
    m_actionInsertFile = m_richTextToolbar->addAction("📎");
    m_actionInsertFile->setToolTip("插入文件");
    connect(m_actionInsertFile, &QAction::triggered, this, &ChatWidget::onInsertFile);
    
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
    m_mainLayout->addWidget(m_richTextToolbar);
}

void ChatWidget::setupChatArea()
{
    // 聊天显示区域
    m_chatScrollArea = new QScrollArea;
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_chatContainer = new QWidget;
    m_chatLayout = new QVBoxLayout(m_chatContainer);
    m_chatLayout->setContentsMargins(10, 10, 10, 10);
    m_chatLayout->setSpacing(15);
    m_chatLayout->addStretch(); // 消息从底部开始
    
    m_chatScrollArea->setWidget(m_chatContainer);
    m_chatScrollArea->setMinimumHeight(400);
    
    m_chatScrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: white;
            border: 1px solid #E5E5EA;
            border-radius: 12px;
        }
        QScrollArea QWidget {
            background-color: white;
        }
        QScrollBar:vertical {
            background-color: #F2F2F7;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: #C7C7CC;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #A8A8AF;
        }
    )");
    
    m_mainLayout->addWidget(m_chatScrollArea);
}

void ChatWidget::setupQuickButtonsArea()
{
    // 快捷按钮区域
    m_quickButtonsGroup = new QGroupBox("快捷咨询");
    m_quickButtonsGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #1D1D1F;
            border: 1px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
        }
    )");
    
    m_quickButtonsLayout = new QGridLayout(m_quickButtonsGroup);
    m_quickButtonsLayout->setSpacing(8);
    m_quickButtonGroup = new QButtonGroup(this);
    connect(m_quickButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(onQuickButtonClicked(QAbstractButton*)));
    
    m_mainLayout->addWidget(m_quickButtonsGroup);
    
    // 动态交互区域
    m_interactionWidget = new QWidget;
    m_interactionLayout = new QVBoxLayout(m_interactionWidget);
    m_interactionLayout->setContentsMargins(0, 0, 0, 0);
    m_interactionWidget->hide(); // 初始隐藏
    
    m_mainLayout->addWidget(m_interactionWidget);
}

void ChatWidget::setupInputArea()
{
    // 输入区域
    m_inputWidget = new QWidget;
    m_inputLayout = new QHBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(10);
    
    // 普通文本输入
    m_messageInput = new QTextEdit;
    m_messageInput->setMaximumHeight(100);
    m_messageInput->setPlaceholderText("请描述您的症状或问题...");
    
    // 富文本输入
    m_richMessageInput = new QTextEdit;
    m_richMessageInput->setMaximumHeight(150);
    m_richMessageInput->setPlaceholderText("输入富文本内容，支持格式化、图片和文件...");
    m_richMessageInput->setAcceptDrops(true);
    m_richMessageInput->hide(); // 初始隐藏
    
    m_btnSend = new QPushButton("发送");
    m_btnVoice = new QPushButton("🎤");
    m_btnEmoji = new QPushButton("😊");
    
    m_btnSend->setFixedSize(80, 36);
    m_btnVoice->setFixedSize(36, 36);
    m_btnEmoji->setFixedSize(36, 36);
    
    QString inputStyle = R"(
        QTextEdit {
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #007AFF;
        }
    )";
    
    QString sendButtonStyle = R"(
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056CC;
        }
        QPushButton:pressed {
            background-color: #004499;
        }
        QPushButton:disabled {
            background-color: #C7C7CC;
        }
    )";
    
    QString iconButtonStyle = R"(
        QPushButton {
            background-color: #F2F2F7;
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #E5E5EA;
        }
    )";
    
    m_messageInput->setStyleSheet(inputStyle);
    m_richMessageInput->setStyleSheet(inputStyle);
    m_btnSend->setStyleSheet(sendButtonStyle);
    m_btnVoice->setStyleSheet(iconButtonStyle);
    m_btnEmoji->setStyleSheet(iconButtonStyle);
    
    connect(m_messageInput, &QTextEdit::textChanged, this, &ChatWidget::onInputTextChanged);
    connect(m_richMessageInput, &QTextEdit::textChanged, this, &ChatWidget::onInputTextChanged);
    connect(m_btnSend, &QPushButton::clicked, this, [this]() {
        if (m_isRichMode) {
            onSendRichMessage();
        } else {
            onSendMessage();
        }
    });
    
    m_inputLayout->addWidget(m_messageInput);
    m_inputLayout->addWidget(m_richMessageInput);
    m_inputLayout->addWidget(m_btnVoice);
    m_inputLayout->addWidget(m_btnEmoji);
    m_inputLayout->addWidget(m_btnSend);
    
    m_mainLayout->addWidget(m_inputWidget);
    
    // 初始化定时器
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatWidget::simulateTyping);
    
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    connect(m_responseTimer, &QTimer::timeout, this, &ChatWidget::onAIResponseReady);
    
    m_richResponseTimer = new QTimer(this);
    m_richResponseTimer->setSingleShot(true);
    connect(m_richResponseTimer, &QTimer::timeout, this, &ChatWidget::onRichAIResponseReady);
}

void ChatWidget::setupQuickButtons()
{
    // 初始化科室列表
    m_departments = {
        "内科", "外科", "妇产科", "儿科", "急诊科",
        "皮肤科", "眼科", "耳鼻喉科", "口腔科", "中医科"
    };
    
    // 初始化症状关键词映射
    m_symptomKeywords["发热"] = {"发烧", "发热", "体温", "高烧", "低烧"};
    m_symptomKeywords["头痛"] = {"头疼", "头痛", "偏头痛", "头晕"};
    m_symptomKeywords["咳嗽"] = {"咳嗽", "咳痰", "干咳", "有痰"};
    m_symptomKeywords["腹痛"] = {"肚子疼", "腹痛", "胃痛", "腹部"};
    m_symptomKeywords["皮肤"] = {"皮疹", "瘙痒", "红肿", "过敏"};
    
    // 添加快捷按钮
    QStringList quickQuestions = {
        "🤒 发热咨询", "😷 感冒症状", "🤕 头痛头晕",
        "🤧 咳嗽咳痰", "😣 腹痛腹泻", "🔴 皮肤问题",
        "👁️ 视力问题", "👂 听力问题", "👤 转人工客服"
    };
    
    int row = 0, col = 0;
    for (const QString& question : quickQuestions) {
        addQuickButton(question, "");
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

void ChatWidget::addQuickButton(const QString& text, const QString& responseTemplate)
{
    QPushButton* button = new QPushButton(text);
    button->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 13px;
            color: #1D1D1F;
            text-align: left;
        }
        QPushButton:hover {
            background-color: #F2F8FF;
            border-color: #007AFF;
        }
        QPushButton:pressed {
            background-color: #E3F2FD;
        }
    )");
    
    button->setProperty("responseTemplate", responseTemplate);
    m_quickButtonGroup->addButton(button);
    
    // 计算按钮位置
    int buttonCount = m_quickButtonGroup->buttons().size() - 1;
    int row = buttonCount / 3;
    int col = buttonCount % 3;
    m_quickButtonsLayout->addWidget(button, row, col);
}

void ChatWidget::onQuickButtonClicked(QAbstractButton* button)
{
    QPushButton* pushButton = qobject_cast<QPushButton*>(button);
    if (pushButton) {
        QString buttonText = pushButton->text();
        qDebug() << "快捷按钮被点击:" << buttonText;
        
        // 检查是否是转人工服务按钮
        if (buttonText.contains("转人工客服")) {
            qDebug() << "检测到转人工客服按钮点击！";
            onTransferToHuman();
            return;
        }
        
        // 移除emoji，提取关键词
        QString cleanText = buttonText.remove(QRegularExpression("[🤒😷🤕🤧😣🔴👁️👂🦷]")).trimmed();
        
        m_messageInput->setPlainText("我想咨询" + cleanText + "的问题");
        onSendMessage();
    }
}

void ChatWidget::onTransferToHuman()
{
    // 调试信息
    qDebug() << "转人工按钮被点击！用户ID:" << m_userId << "用户名:" << m_userName;
    
    // 添加转人工提示消息
    AIMessage systemMsg;
    systemMsg.content = "正在为您转接人工客服，请稍候...";
    systemMsg.type = MessageType::System;
    systemMsg.timestamp = QDateTime::currentDateTime();
    systemMsg.sessionId = m_currentSessionId;
    addMessage(systemMsg);
    
    // 发出转人工信号，包含当前对话上下文
    QString context = "";
    for (const AIMessage& msg : m_chatHistory) {
        if (msg.type == MessageType::User) {
            context += "患者：" + msg.content + "\n";
        } else if (msg.type == MessageType::Robot) {
            context += "AI助手：" + msg.content + "\n";
        }
    }
    
    qDebug() << "发射转人工信号，上下文长度:" << context.length();
    emit requestHumanService(m_userId, m_userName, context);
    
    // 添加转人工说明
    addTransferOption();
}

void ChatWidget::addTransferOption()
{
    clearInteractionComponents();
    
    QWidget* transferWidget = new QWidget;
    QVBoxLayout* transferLayout = new QVBoxLayout(transferWidget);
    transferLayout->setSpacing(10);
    
    QLabel* infoLabel = new QLabel("已为您转接人工客服服务：");
    infoLabel->setStyleSheet(R"(
        QLabel {
            font-size: 14px;
            color: #1D1D1F;
            font-weight: bold;
            padding: 5px;
        }
    )");
    
    QLabel* detailLabel = new QLabel("• 请切换到\"客服咨询\"选项卡继续对话\n• 您的对话记录已同步给客服\n• 如需重新使用AI分诊，请点击下方按钮");
    detailLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: #666666;
            padding: 5px 10px;
            line-height: 1.4;
        }
    )");
    
    QPushButton* backToAIButton = new QPushButton("🤖 返回AI分诊");
    backToAIButton->setStyleSheet(R"(
        QPushButton {
            background-color: #34C759;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #30A855;
        }
    )");
    
    connect(backToAIButton, &QPushButton::clicked, [this]() {
        clearInteractionComponents();
        AIMessage backMsg;
        backMsg.content = "欢迎回到AI智能分诊！有什么可以帮助您的吗？";
        backMsg.type = MessageType::Robot;
        backMsg.timestamp = QDateTime::currentDateTime();
        backMsg.sessionId = m_currentSessionId;
        addMessage(backMsg);
    });
    
    transferLayout->addWidget(infoLabel);
    transferLayout->addWidget(detailLabel);
    transferLayout->addWidget(backToAIButton);
    transferLayout->addStretch();
    
    m_interactionLayout->addWidget(transferWidget);
    m_interactionWidget->show();
}

void ChatWidget::onSendMessage()
{
    QString text = m_messageInput->toPlainText().trimmed();
    if (text.isEmpty() || m_isAITyping) {
        return;
    }
    
    // 创建用户消息
    AIMessage userMsg;
    userMsg.content = text;
    userMsg.type = MessageType::User;
    userMsg.timestamp = QDateTime::currentDateTime();
    userMsg.sessionId = m_currentSessionId;
    
    addMessage(userMsg);
    m_messageInput->clear();
    m_currentContext = text;
    
    // 检查是否直接触发转人工关键词
    QString lowerText = text.toLower();
    if (lowerText.contains("转人工") || lowerText.contains("换人工") || lowerText.contains("要人工") || 
        lowerText.contains("人工客服") || lowerText.contains("真人客服") || lowerText.contains("联系客服")) {
        QTimer::singleShot(500, this, &ChatWidget::onTransferToHuman);
        return;
    }
    
    // 构建对话历史
    QString conversationHistory;
    int recentMsgCount = qMin(5, m_chatHistory.size()); // 只取最近5条消息作为上下文
    for (int i = m_chatHistory.size() - recentMsgCount; i < m_chatHistory.size(); i++) {
        const AIMessage& msg = m_chatHistory[i];
        if (msg.type == MessageType::User) {
            conversationHistory += "患者：" + msg.content + "\n";
        } else if (msg.type == MessageType::Robot) {
            conversationHistory += "AI助手：" + msg.content + "\n";
        }
    }
    
    // 使用真实的AI API进行分诊
    m_aiApiClient->sendTriageRequest(text, conversationHistory);
}

void ChatWidget::onAIResponseReady()
{
    m_isAITyping = false;
    m_statusLabel->setText("智能分诊助手");
    m_btnSend->setEnabled(true);
    
    if (!m_pendingResponse.isEmpty()) {
        AIMessage aiMsg;
        aiMsg.content = m_pendingResponse;
        aiMsg.type = MessageType::Robot;
        aiMsg.timestamp = QDateTime::currentDateTime();
        aiMsg.sessionId = m_currentSessionId;
        
        addMessage(aiMsg);
        m_pendingResponse.clear();
        
        // 分析是否需要添加交互组件
        TriageAdvice advice = analyzeSymptoms(m_currentContext);
        if (!advice.department.isEmpty()) {
            processTriageAdvice(advice);
        }
    }
}

QString ChatWidget::generateAIResponse(const QString& userInput)
{
    // AI分诊逻辑
    QString input = userInput.toLower();
    
    // 检查紧急情况
    QStringList emergencyKeywords = {"胸痛", "呼吸困难", "昏迷", "大出血", "中毒", "外伤"};
    for (const QString& keyword : emergencyKeywords) {
        if (input.contains(keyword)) {
            return "⚠️ 根据您描述的症状，建议您立即前往急诊科就诊！\n\n这种情况可能比较紧急，请不要延误。\n\n急诊科位置：医院1楼\n急诊电话：120";
        }
    }
    
    // 发热相关
    if (input.contains("发烧") || input.contains("发热") || input.contains("体温")) {
        return "根据您的发热症状，我需要了解更多信息：\n\n• 体温多少度？\n• 持续多长时间了？\n• 是否伴随其他症状？\n\n一般情况下：\n🌡️ 38.5°C以下：建议物理降温\n🌡️ 38.5°C以上：建议内科就诊\n🚨 持续高热：建议急诊科\n\n如需更详细的诊断，建议点击下方转人工客服。";
    }
    
    // 头痛相关
    if (input.contains("头疼") || input.contains("头痛") || input.contains("头晕")) {
        return "关于头痛症状，我来帮您分析：\n\n请问：\n• 疼痛程度如何？\n• 是否伴随恶心呕吐？\n• 最近有没有外伤？\n\n建议科室：\n🧠 神经内科：偏头痛、神经性头痛\n👁️ 眼科：视力相关头痛\n🏥 内科：感冒引起的头痛\n\n如需专业医生诊断，可转接人工客服。";
    }
    
    // 咳嗽相关
    if (input.contains("咳嗽") || input.contains("咳痰")) {
        return "咳嗽症状分析：\n\n请描述：\n• 干咳还是有痰？\n• 持续时间？\n• 是否伴随发热？\n\n推荐科室：\n🫁 呼吸内科：持续咳嗽、咳痰\n👶 儿科：小儿咳嗽\n🏥 内科：一般性咳嗽\n\n需要详细诊断建议转人工客服。";
    }
    
    // 转人工相关 - 直接触发转人工
    if (input.contains("转人工") || input.contains("换人工") || input.contains("要人工") || 
        input.contains("人工客服") || input.contains("真人客服") || input.contains("联系客服")) {
        // 直接触发转人工
        QTimer::singleShot(500, this, &ChatWidget::onTransferToHuman);
        return "好的，正在为您转接人工客服，请稍候...";
    }
    
    // 一般人工咨询提示
    if (input.contains("人工") || input.contains("客服") || input.contains("医生")) {
        return "我可以为您转接人工客服：\n\n🏥 人工客服可以提供：\n• 专业医疗咨询\n• 详细症状分析\n• 预约挂号协助\n• 医院相关服务\n\n💬 输入\"转人工\"可直接转接\n📱 或点击下方\"转人工客服\"按钮";
    }
    
    // 预约相关
    if (input.contains("预约") || input.contains("挂号")) {
        return "关于预约挂号：\n\n📱 预约方式：\n• 微信公众号预约\n• 手机APP预约\n• 现场挂号\n• 电话预约：400-123-4567\n\n⏰ 预约时间：\n• 普通门诊：提前3天\n• 专家门诊：提前7天\n\n需要预约协助？建议转接人工客服。";
    }
    
    // 默认响应
    return "我理解您的症状描述。为了给您更准确的建议，请提供更多详细信息：\n\n• 症状持续时间\n• 疼痛或不适程度\n• 是否伴随其他症状\n• 您的年龄范围\n\n如需专业医生诊断，建议转人工客服获得更详细的医疗建议。";
}

// 实现其他必要的方法
void ChatWidget::addMessage(const AIMessage& message)
{
    m_chatHistory.append(message);
    displayMessage(message);
    m_messageCount++;
    
    // 自动保存每10条消息
    if (m_messageCount % 10 == 0) {
        saveChatHistory();
    }
}

void ChatWidget::displayMessage(const AIMessage& message)
{
    QWidget* messageWidget = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息气泡
    QLabel* bubbleLabel = new QLabel;
    bubbleLabel->setText(message.content);
    bubbleLabel->setWordWrap(true);
    bubbleLabel->setTextFormat(Qt::RichText);
    bubbleLabel->setOpenExternalLinks(true);
    
    // 时间戳
    QLabel* timeLabel = new QLabel(formatTimestamp(message.timestamp));
    timeLabel->setStyleSheet("color: #8E8E93; font-size: 11px;");
    
    QString bubbleStyle;
    if (message.type == MessageType::User) {
        // 用户消息 - 右对齐，蓝色
        bubbleStyle = R"(
            QLabel {
                background-color: #007AFF;
                color: white;
                border-radius: 12px;
                padding: 12px 16px;
                margin-left: 60px;
                font-size: 14px;
                line-height: 1.4;
            }
        )";
        messageLayout->addStretch();
        
        QVBoxLayout* rightLayout = new QVBoxLayout;
        rightLayout->addWidget(bubbleLabel);
        rightLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignRight);
        
        messageLayout->addLayout(rightLayout);
        
    } else {
        // 机器人消息 - 左对齐，灰色
        bubbleStyle = R"(
            QLabel {
                background-color: #F2F2F7;
                color: #1D1D1F;
                border-radius: 12px;
                padding: 12px 16px;
                margin-right: 60px;
                font-size: 14px;
                line-height: 1.4;
            }
        )";
        
        // 添加机器人头像
        QLabel* avatarLabel = new QLabel("🤖");
        avatarLabel->setFixedSize(32, 32);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setStyleSheet(R"(
            QLabel {
                background-color: #34C759;
                border-radius: 16px;
                font-size: 16px;
            }
        )");
        
        QVBoxLayout* leftLayout = new QVBoxLayout;
        leftLayout->addWidget(bubbleLabel);
        leftLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignLeft);
        
        messageLayout->addWidget(avatarLabel);
        messageLayout->addLayout(leftLayout);
        messageLayout->addStretch();
    }
    
    bubbleLabel->setStyleSheet(bubbleStyle);
    
    // 移除最后的弹性空间，添加新消息，再添加弹性空间
    m_chatLayout->removeItem(m_chatLayout->itemAt(m_chatLayout->count() - 1));
    m_chatLayout->addWidget(messageWidget);
    m_chatLayout->addStretch();
    
    // 滚动到底部
    QTimer::singleShot(100, this, &ChatWidget::scrollToBottom);
}

void ChatWidget::scrollToBottom()
{
    QScrollBar* scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

TriageAdvice ChatWidget::analyzeSymptoms(const QString& userInput)
{
    TriageAdvice advice;
    QString input = userInput.toLower();
    
    // 简单的症状分析逻辑
    if (input.contains("发烧") || input.contains("发热")) {
        advice.department = "内科";
        advice.reason = "发热症状通常需要内科医生评估";
        advice.needAppointment = true;
        advice.needEmergency = input.contains("高烧") || input.contains("39");
    }
    else if (input.contains("咳嗽") || input.contains("呼吸")) {
        advice.department = "呼吸内科";
        advice.reason = "呼吸道症状建议看呼吸内科";
        advice.needAppointment = true;
    }
    else if (input.contains("头痛") || input.contains("头晕")) {
        advice.department = "神经内科";
        advice.reason = "头部不适建议神经内科检查";
        advice.needAppointment = true;
    }
    
    return advice;
}

void ChatWidget::processTriageAdvice(const TriageAdvice& advice)
{
    if (advice.needEmergency) {
        // 紧急情况，添加紧急就诊按钮
        addActionButtons({"🚨 立即急诊", "📞 拨打120", "👤 转人工客服"});
    } else if (advice.needAppointment) {
        // 需要预约，添加预约按钮
        addActionButtons({"📅 预约" + advice.department, "🔍 查看更多科室", "👤 转人工客服"});
    }
}

void ChatWidget::addActionButtons(const QStringList& actions)
{
    clearInteractionComponents();
    
    QWidget* actionWidget = new QWidget;
    QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setSpacing(10);
    
    for (const QString& action : actions) {
        QPushButton* btn = new QPushButton(action);
        QString buttonStyle;
        
        if (action.contains("转人工客服")) {
            buttonStyle = R"(
                QPushButton {
                    background-color: #FF9500;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #E6860E;
                }
            )";
            connect(btn, &QPushButton::clicked, this, &ChatWidget::onTransferToHuman);
        } else {
            buttonStyle = R"(
                QPushButton {
                    background-color: #34C759;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #30A855;
                }
            )";
            connect(btn, &QPushButton::clicked, this, &ChatWidget::onActionButtonClicked);
        }
        
        btn->setStyleSheet(buttonStyle);
        actionLayout->addWidget(btn);
    }
    
    actionLayout->addStretch();
    m_interactionLayout->addWidget(actionWidget);
    m_interactionWidget->show();
}

void ChatWidget::clearInteractionComponents()
{
    while (m_interactionLayout->count() > 0) {
        QLayoutItem* item = m_interactionLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_interactionWidget->hide();
}

// 其他槽函数的简单实现
void ChatWidget::onInputTextChanged()
{
    bool hasText;
    if (m_isRichMode) {
        hasText = !m_richMessageInput->toPlainText().trimmed().isEmpty();
    } else {
        hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
    }
    m_btnSend->setEnabled(hasText && !m_isAITyping);
}

void ChatWidget::onActionButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString action = button->text();
        
        AIMessage actionMsg;
        actionMsg.content = "您选择了：" + action + "\n\n相关功能正在开发中。如需立即协助，建议转人工客服。";
        actionMsg.type = MessageType::System;
        actionMsg.timestamp = QDateTime::currentDateTime();
        actionMsg.sessionId = m_currentSessionId;
        
        addMessage(actionMsg);
        clearInteractionComponents();
    }
}

void ChatWidget::onClearChatClicked()
{
    int ret = QMessageBox::question(this, "确认清空", "确定要清空聊天记录吗？\n此操作不可恢复。");
    if (ret == QMessageBox::Yes) {
        // 清空显示
        while (m_chatLayout->count() > 1) {
            QLayoutItem* item = m_chatLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        
        // 清空历史记录
        m_chatHistory.clear();
        m_messageCount = 0;
        
        // 重新发送欢迎消息
        QTimer::singleShot(300, [this]() {
            AIMessage welcomeMsg;
            welcomeMsg.content = "聊天记录已清空。我是医院智能分诊助手，请问有什么可以帮助您的吗？";
            welcomeMsg.type = MessageType::Robot;
            welcomeMsg.timestamp = QDateTime::currentDateTime();
            welcomeMsg.sessionId = m_currentSessionId;
            addMessage(welcomeMsg);
        });
    }
}

void ChatWidget::onSaveChatClicked()
{
    QMessageBox::information(this, "保存成功", "聊天记录已保存。");
}

void ChatWidget::onSettingsClicked()
{
    SettingsDialog dialog("patient", this);
    
    // 连接设置变更信号
    connect(&dialog, &SettingsDialog::settingsChanged, this, [this]() {
        // 可以在这里处理设置变更后的逻辑，比如更新界面主题、字体等
        qDebug() << "患者端设置已更新";
        // 可以重新加载设置并应用到界面
        loadUserSettings();
    });
    
    connect(&dialog, &SettingsDialog::fontChanged, this, [this]() {
        // 处理字体变更
        updateChatFont();
    });
    
    connect(&dialog, &SettingsDialog::themeChanged, this, [this]() {
        // 处理主题变更
        updateTheme();
    });
    
    dialog.exec();
}

// 数据库相关方法的简单实现
void ChatWidget::initDatabase()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "ai_chat_db");
    m_database.setDatabaseName(dbPath + "/ai_chat_history.db");
    
    if (m_database.open()) {
        QSqlQuery query(m_database);
        query.exec("CREATE TABLE IF NOT EXISTS ai_chat_messages ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                  "session_id TEXT,"
                  "content TEXT,"
                  "message_type INTEGER,"
                  "timestamp TEXT)");
    }
}

void ChatWidget::saveChatHistory()
{
    if (!m_database.isOpen()) return;
    
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO ai_chat_messages (session_id, content, message_type, timestamp) "
                 "VALUES (?, ?, ?, ?)");
    
    for (const AIMessage& msg : m_chatHistory) {
        query.bindValue(0, msg.sessionId);
        query.bindValue(1, msg.content);
        query.bindValue(2, static_cast<int>(msg.type));
        query.bindValue(3, msg.timestamp.toString(Qt::ISODate));
        query.exec();
    }
}

QString ChatWidget::generateSessionId()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString ChatWidget::formatTimestamp(const QDateTime& timestamp)
{
    return timestamp.toString("hh:mm");
}

// 其他未实现的方法
void ChatWidget::simulateTyping() { }
void ChatWidget::onDepartmentSelected() { }
void ChatWidget::loadChatHistory() { }
QString ChatWidget::extractKeywords(const QString& text) { return text; }
QStringList ChatWidget::getSymptomKeywords(const QString& text) { return QStringList(); }
void ChatWidget::updateQuickButtons(const QStringList& suggestions) { }
void ChatWidget::addDepartmentSelector(const QStringList& departments) { }

void ChatWidget::onAITriageResponse(const AIDiagnosisResult& result)
{
    // 创建AI回复消息
    AIMessage aiMsg;
    aiMsg.content = result.aiResponse;
    aiMsg.type = MessageType::Robot;
    aiMsg.timestamp = QDateTime::currentDateTime();
    aiMsg.sessionId = m_currentSessionId;
    
    addMessage(aiMsg);
    
    // 根据诊断结果添加交互组件
    QStringList actionButtons;
    
    if (result.emergencyLevel == "critical") {
        actionButtons << "🚨 立即急诊" << "📞 拨打120" << "👤 转人工客服";
    } else if (result.emergencyLevel == "high") {
        actionButtons << "🏥 尽快就医" << "📞 预约挂号" << "👤 转人工客服";
    } else if (!result.recommendedDepartment.isEmpty()) {
        actionButtons << QString("📅 预约%1").arg(result.recommendedDepartment) 
                      << "🔍 查看更多科室" << "👤 转人工客服";
    } else {
        actionButtons << "🔍 症状分析" << "📅 预约挂号" << "👤 转人工客服";
    }
    
    if (!actionButtons.isEmpty()) {
        addActionButtons(actionButtons);
    }
    
    qDebug() << "AI分诊结果 - 科室:" << result.recommendedDepartment 
             << "紧急程度:" << result.emergencyLevel
             << "需要人工:" << result.needsHumanConsult;
}

void ChatWidget::onAIApiError(const QString& error)
{
    qDebug() << "AI API错误:" << error;
    
    // 显示错误消息并回退到本地逻辑
    AIMessage errorMsg;
    errorMsg.content = "抱歉，AI服务暂时不可用，为您提供基础分诊建议：\n\n" + generateAIResponse(m_currentContext);
    errorMsg.type = MessageType::Robot;
    errorMsg.timestamp = QDateTime::currentDateTime();
    errorMsg.sessionId = m_currentSessionId;
    
    addMessage(errorMsg);
    
    // 添加基础交互按钮
    addActionButtons({"🔍 症状自查", "📅 预约挂号", "👤 转人工客服"});
}

void ChatWidget::onToggleRichMode()
{
    m_isRichMode = m_btnToggleRichMode->isChecked();
    
    if (m_isRichMode) {
        m_richTextToolbar->show();
        m_messageInput->hide();
        m_richMessageInput->show();
        m_btnToggleRichMode->setText("普通模式");
    } else {
        m_richTextToolbar->hide();
        m_messageInput->show();
        m_richMessageInput->hide();
        m_btnToggleRichMode->setText("富文本模式");
    }
    
    // 切换模式后更新发送按钮状态
    onInputTextChanged();
}

void ChatWidget::onTextBold()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontWeight(m_actionBold->isChecked() ? QFont::Bold : QFont::Normal);
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onTextItalic()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontItalic(m_actionItalic->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onTextUnderline()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontUnderline(m_actionUnderline->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onTextStrikeOut()
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontStrikeOut(m_actionStrikeOut->isChecked());
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onTextColor()
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

void ChatWidget::onTextBackgroundColor()
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

void ChatWidget::onFontFamilyChanged(const QString& fontFamily)
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontFamilies({fontFamily});
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onFontSizeChanged(int size)
{
    if (!m_richMessageInput) return;
    
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(size);
    cursor.mergeCharFormat(format);
    m_richMessageInput->mergeCurrentCharFormat(format);
}

void ChatWidget::onInsertImage()
{
    if (!m_richMessageInput) return;
    
    QString imagePath = QFileDialog::getOpenFileName(this, 
        "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp)");
    
    if (!imagePath.isEmpty()) {
        insertImageIntoEditor(imagePath);
    }
}

void ChatWidget::onInsertFile()
{
    if (!m_richMessageInput) return;
    
    QString filePath = QFileDialog::getOpenFileName(this, 
        "选择文件", "", "所有文件 (*)");
    
    if (!filePath.isEmpty()) {
        insertFileIntoEditor(filePath);
    }
}

void ChatWidget::insertImageIntoEditor(const QString& imagePath)
{
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) return;
    
    // 读取图片并调整大小
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) return;
    
    // 限制图片大小
    int maxWidth = 300;
    int maxHeight = 200;
    if (pixmap.width() > maxWidth || pixmap.height() > maxHeight) {
        pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 创建图片名称
    QString imageName = QString("image_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    // 添加到文档
    QTextDocument* doc = m_richMessageInput->document();
    doc->addResource(QTextDocument::ImageResource, QUrl(imageName), pixmap);
    
    // 插入图片
    QTextCursor cursor = m_richMessageInput->textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setName(imageName);
    imageFormat.setWidth(pixmap.width());
    imageFormat.setHeight(pixmap.height());
    cursor.insertImage(imageFormat);
    
    m_richMessageInput->setTextCursor(cursor);
}

void ChatWidget::insertFileIntoEditor(const QString& filePath)
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

void ChatWidget::onSendRichMessage()
{
    if (!m_richMessageInput || m_richMessageInput->toPlainText().trimmed().isEmpty()) {
        return;
    }
    
    QString plainText = m_richMessageInput->toPlainText().trimmed();
    QString htmlContent = m_richMessageInput->toHtml();
    
    // 创建富文本消息
    RichMessage userMsg;
    userMsg.content = plainText;
    userMsg.htmlContent = htmlContent;
    userMsg.type = MessageType::User;
    userMsg.contentType = RichContentType::RichText;
    userMsg.timestamp = QDateTime::currentDateTime();
    userMsg.sessionId = m_currentSessionId;
    
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
                    userMsg.attachments.append(imageFormat.name());
                    userMsg.contentType = RichContentType::Mixed;
                }
            }
        }
        block = block.next();
    }
    
    addRichMessage(userMsg);
    m_richMessageInput->clear();
    
    // 模拟AI响应 - 在富文本模式下
    m_isAITyping = true;
    m_statusLabel->setText("AI正在思考中...");
    m_btnSend->setEnabled(false);
    
    QString aiResponse = generateAIResponse(plainText);
    m_pendingRichResponse = aiResponse;
    m_pendingResponsePlainText = plainText;  // 保存用户输入用于分析
    m_richResponseTimer->start(1500 + QRandomGenerator::global()->bounded(1000));
}

void ChatWidget::addRichMessage(const RichMessage& message)
{
    m_richChatHistory.append(message);
    displayRichMessage(message);
    m_messageCount++;
    
    // 自动保存每10条消息
    if (m_messageCount % 10 == 0) {
        saveRichChatHistory();
    }
}

void ChatWidget::displayRichMessage(const RichMessage& message)
{
    QWidget* messageWidget = createRichMessageBubble(message);
    
    // 移除最后的弹性空间，添加新消息，再添加弹性空间
    m_chatLayout->removeItem(m_chatLayout->itemAt(m_chatLayout->count() - 1));
    m_chatLayout->addWidget(messageWidget);
    m_chatLayout->addStretch();
    
    // 滚动到底部
    QTimer::singleShot(100, this, &ChatWidget::scrollToBottom);
}

QWidget* ChatWidget::createRichMessageBubble(const RichMessage& message)
{
    QWidget* messageWidget = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息内容显示
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
    
    // 时间戳
    QLabel* timeLabel = new QLabel(formatTimestamp(message.timestamp));
    timeLabel->setStyleSheet("color: #8E8E93; font-size: 11px;");
    
    QString bubbleStyle;
    if (message.type == MessageType::User) {
        // 用户消息 - 右对齐，蓝色
        bubbleStyle = R"(
            QTextBrowser {
                background-color: #007AFF;
                color: white;
                border-radius: 12px;
                padding: 12px 16px;
                margin-left: 60px;
                font-size: 14px;
                line-height: 1.4;
                border: none;
            }
        )";
        messageLayout->addStretch();
        
        QVBoxLayout* rightLayout = new QVBoxLayout;
        rightLayout->addWidget(contentBrowser);
        rightLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignRight);
        
        messageLayout->addLayout(rightLayout);
        
    } else {
        // 机器人消息 - 左对齐，灰色
        bubbleStyle = R"(
            QTextBrowser {
                background-color: #F2F2F7;
                color: #1D1D1F;
                border-radius: 12px;
                padding: 12px 16px;
                margin-right: 60px;
                font-size: 14px;
                line-height: 1.4;
                border: none;
            }
        )";
        
        // 添加机器人头像
        QLabel* avatarLabel = new QLabel("🤖");
        avatarLabel->setFixedSize(32, 32);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setStyleSheet(R"(
            QLabel {
                background-color: #34C759;
                border-radius: 16px;
                font-size: 16px;
            }
        )");
        
        QVBoxLayout* leftLayout = new QVBoxLayout;
        leftLayout->addWidget(contentBrowser);
        leftLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignLeft);
        
        messageLayout->addWidget(avatarLabel);
        messageLayout->addLayout(leftLayout);
        messageLayout->addStretch();
    }
    
    contentBrowser->setStyleSheet(bubbleStyle);
    
    return messageWidget;
}

void ChatWidget::saveRichChatHistory()
{
    if (!m_dbManager) return;
    
    // 实现富文本聊天记录保存
    // 这里需要扩展数据库以支持富文本内容
    // 暂时使用JSON格式存储富文本数据
    for (const RichMessage& msg : m_richChatHistory) {
        // TODO: 实现富文本数据库存储
    }
}

void ChatWidget::loadRichChatHistory()
{
    if (!m_dbManager) return;
    
    // 实现富文本聊天记录加载
    // TODO: 从数据库加载富文本记录
}

void ChatWidget::onRichAIResponseReady()
{
    m_isAITyping = false;
    m_statusLabel->setText("智能分诊助手");
    m_btnSend->setEnabled(true);
    
    if (!m_pendingRichResponse.isEmpty()) {
        // 创建富文本AI响应消息
        RichMessage aiMsg;
        aiMsg.content = m_pendingRichResponse;
        aiMsg.htmlContent = convertToRichText(m_pendingRichResponse);
        aiMsg.type = MessageType::Robot;
        aiMsg.contentType = RichContentType::Text;  // AI响应暂时为纯文本
        aiMsg.timestamp = QDateTime::currentDateTime();
        aiMsg.sessionId = m_currentSessionId;
        
        addRichMessage(aiMsg);
        m_pendingRichResponse.clear();
        
        // 在富文本模式下也分析是否需要添加交互组件
        TriageAdvice advice = analyzeSymptoms(m_pendingResponsePlainText);
        if (!advice.department.isEmpty()) {
            processTriageAdvice(advice);
        }
        
        m_pendingResponsePlainText.clear();
    }
}

QString ChatWidget::convertToRichText(const QString& plainText)
{
    // 将纯文本转换为基本的富文本格式
    QString richText = plainText;
    
    // 转换换行符为HTML格式
    richText.replace('\n', "<br>");
    
    // 识别并格式化一些关键词
    richText.replace(QRegularExpression("(⚠️[^\\n]*紧急[^\\n]*)"), "<span style='color: #FF3B30; font-weight: bold;'>\\1</span>");
    richText.replace(QRegularExpression("(🌡️[^\\n]*)"), "<span style='color: #FF9500;'>\\1</span>");
    richText.replace(QRegularExpression("(🧠[^\\n]*)"), "<span style='color: #5856D6;'>\\1</span>");
    richText.replace(QRegularExpression("(🫁[^\\n]*)"), "<span style='color: #34C759;'>\\1</span>");
    richText.replace(QRegularExpression("(🏥[^\\n]*)"), "<span style='color: #007AFF;'>\\1</span>");
    richText.replace(QRegularExpression("(📱[^\\n]*)"), "<span style='color: #5AC8FA;'>\\1</span>");
    
    // 格式化科室名称
    richText.replace(QRegularExpression("([内外妇儿急皮眼耳口中][科医]*)："), "<span style='font-weight: bold; color: #007AFF;'>\\1：</span>");
    
    return richText;
}

// 设置相关方法实现
void ChatWidget::loadUserSettings()
{
    QSettings settings("HospAI", "Settings");
    
    // 加载字体设置
    QString fontStr = settings.value("chat/font", "Microsoft YaHei,12,-1,5,50,0,0,0,0,0").toString();
    QFont chatFont;
    chatFont.fromString(fontStr);
    
    // 应用字体到聊天区域
    if (m_chatContainer) {
        m_chatContainer->setFont(chatFont);
    }
    
    // 加载其他设置
    bool showTimestamp = settings.value("chat/showTimestamp", true).toBool();
    // 可以根据需要处理时间戳显示设置
    
    qDebug() << "用户设置已加载";
}

void ChatWidget::updateChatFont()
{
    QSettings settings("HospAI", "Settings");
    QString fontStr = settings.value("chat/font", "Microsoft YaHei,12,-1,5,50,0,0,0,0,0").toString();
    QFont chatFont;
    chatFont.fromString(fontStr);
    
    // 更新聊天区域字体
    if (m_chatContainer) {
        m_chatContainer->setFont(chatFont);
        
        // 更新所有现有消息的字体
        QList<QWidget*> messageWidgets = m_chatContainer->findChildren<QWidget*>();
        for (QWidget* widget : messageWidgets) {
            widget->setFont(chatFont);
        }
    }
    
    // 更新输入框字体
    if (m_messageInput) {
        m_messageInput->setFont(chatFont);
    }
    if (m_richMessageInput) {
        m_richMessageInput->setFont(chatFont);
    }
    
    qDebug() << "聊天字体已更新";
}

void ChatWidget::updateTheme()
{
    QSettings settings("HospAI", "Settings");
    QString theme = settings.value("general/theme", "浅色主题").toString();
    
    QString styleSheet;
    if (theme == "深色主题") {
        styleSheet = R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QTextEdit {
                background-color: #3c3c3c;
                border: 1px solid #555;
                border-radius: 8px;
                color: #ffffff;
            }
            QPushButton {
                background-color: #0078d4;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #106ebe;
            }
        )";
    } else if (theme == "护眼模式") {
        styleSheet = R"(
            QWidget {
                background-color: #f5f5dc;
                color: #2f4f4f;
            }
            QTextEdit {
                background-color: #fafafa;
                border: 1px solid #d3d3d3;
                border-radius: 8px;
                color: #2f4f4f;
            }
            QPushButton {
                background-color: #8fbc8f;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #7ba07b;
            }
        )";
    } else {
        // 浅色主题（默认）
        styleSheet = R"(
            QWidget {
                background-color: #ffffff;
                color: #333333;
            }
            QTextEdit {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
                color: #333333;
            }
            QPushButton {
                background-color: #0078d4;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #106ebe;
            }
        )";
    }
    
    setStyleSheet(styleSheet);
    qDebug() << "主题已更新为:" << theme;
} 