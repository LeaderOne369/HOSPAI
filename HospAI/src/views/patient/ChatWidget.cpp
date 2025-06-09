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

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolBarLayout(nullptr)
    , m_btnClearChat(nullptr)
    , m_btnSaveChat(nullptr)
    , m_btnSettings(nullptr)
    , m_statusLabel(nullptr)
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
    , m_btnSend(nullptr)
    , m_btnVoice(nullptr)
    , m_btnEmoji(nullptr)
    , m_typingTimer(nullptr)
    , m_responseTimer(nullptr)
    , m_isAITyping(false)
    , m_isInitialized(false)
    , m_messageCount(0)
{
    initDatabase();
    setupUI();
    setupQuickButtons();
    
    m_currentSessionId = generateSessionId();
    m_isInitialized = true;
    
    // 发送欢迎消息
    QTimer::singleShot(500, [this]() {
        ChatMessage welcomeMsg;
        welcomeMsg.content = "您好！我是医院智能分诊助手🏥\n\n我可以帮助您：\n• 🔍 分析症状，推荐合适科室\n• 📅 协助预约挂号流程\n• ❓ 解答就医相关问题\n• 🚨 识别紧急情况\n\n请描述您的症状或点击下方快捷按钮开始咨询～";
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

void ChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    setupToolBar();
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
    // 工具栏
    m_toolBarLayout = new QHBoxLayout;
    
    m_statusLabel = new QLabel("智能分诊助手");
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18px;
            font-weight: bold;
            color: #1D1D1F;
            padding: 5px;
        }
    )");
    
    m_btnClearChat = new QPushButton("🗑️ 清空");
    m_btnSaveChat = new QPushButton("💾 保存");
    m_btnSettings = new QPushButton("⚙️ 设置");
    
    QString toolButtonStyle = R"(
        QPushButton {
            background-color: #F2F2F7;
            border: 1px solid #D1D1D6;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            color: #1D1D1F;
        }
        QPushButton:hover {
            background-color: #E5E5EA;
        }
        QPushButton:pressed {
            background-color: #D1D1D6;
        }
    )";
    
    m_btnClearChat->setStyleSheet(toolButtonStyle);
    m_btnSaveChat->setStyleSheet(toolButtonStyle);
    m_btnSettings->setStyleSheet(toolButtonStyle);
    
    connect(m_btnClearChat, &QPushButton::clicked, this, &ChatWidget::onClearChatClicked);
    connect(m_btnSaveChat, &QPushButton::clicked, this, &ChatWidget::onSaveChatClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &ChatWidget::onSettingsClicked);
    
    m_toolBarLayout->addWidget(m_statusLabel);
    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_btnClearChat);
    m_toolBarLayout->addWidget(m_btnSaveChat);
    m_toolBarLayout->addWidget(m_btnSettings);
    
    m_mainLayout->addLayout(m_toolBarLayout);
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
    connect(m_quickButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, &ChatWidget::onQuickButtonClicked);
    
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
    
    m_messageInput = new QTextEdit;
    m_messageInput->setMaximumHeight(100);
    m_messageInput->setPlaceholderText("请描述您的症状或问题...");
    
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
    m_btnSend->setStyleSheet(sendButtonStyle);
    m_btnVoice->setStyleSheet(iconButtonStyle);
    m_btnEmoji->setStyleSheet(iconButtonStyle);
    
    connect(m_messageInput, &QTextEdit::textChanged, this, &ChatWidget::onInputTextChanged);
    connect(m_btnSend, &QPushButton::clicked, this, &ChatWidget::onSendMessage);
    
    // 支持回车发送
    m_messageInput->installEventFilter(this);
    
    m_inputLayout->addWidget(m_messageInput);
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
        "👁️ 视力问题", "👂 听力问题", "🦷 口腔问题"
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

void ChatWidget::addMessage(const ChatMessage& message)
{
    m_chatHistory.append(message);
    displayMessage(message);
    m_messageCount++;
    
    // 自动保存每10条消息
    if (m_messageCount % 10 == 0) {
        saveChatHistory();
    }
}

void ChatWidget::displayMessage(const ChatMessage& message)
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
        messageLayout->addWidget(bubbleLabel);
        
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

void ChatWidget::onSendMessage()
{
    QString text = m_messageInput->toPlainText().trimmed();
    if (text.isEmpty() || m_isAITyping) {
        return;
    }
    
    // 创建用户消息
    ChatMessage userMsg;
    userMsg.content = text;
    userMsg.type = MessageType::User;
    userMsg.timestamp = QDateTime::currentDateTime();
    userMsg.sessionId = m_currentSessionId;
    
    addMessage(userMsg);
    m_messageInput->clear();
    
    // 显示AI正在输入
    m_isAITyping = true;
    m_statusLabel->setText("智能分诊助手正在分析...");
    m_btnSend->setEnabled(false);
    
    // 生成AI响应
    m_pendingResponse = generateAIResponse(text);
    m_currentContext = text;
    
    // 模拟AI思考时间
    int delay = QRandomGenerator::global()->bounded(1000, 3000);
    m_responseTimer->start(delay);
}

void ChatWidget::onAIResponseReady()
{
    m_isAITyping = false;
    m_statusLabel->setText("智能分诊助手");
    m_btnSend->setEnabled(true);
    
    if (!m_pendingResponse.isEmpty()) {
        ChatMessage aiMsg;
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
    // 简单的关键词匹配响应逻辑
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
        return "根据您的发热症状，我需要了解更多信息：\n\n• 体温多少度？\n• 持续多长时间了？\n• 是否伴随其他症状？\n\n一般情况下：\n🌡️ 38.5°C以下：建议物理降温\n🌡️ 38.5°C以上：建议内科就诊\n🚨 持续高热：建议急诊科";
    }
    
    // 头痛相关
    if (input.contains("头疼") || input.contains("头痛") || input.contains("头晕")) {
        return "关于头痛症状，我来帮您分析：\n\n请问：\n• 疼痛程度如何？\n• 是否伴随恶心呕吐？\n• 最近有没有外伤？\n\n建议科室：\n🧠 神经内科：偏头痛、神经性头痛\n👁️ 眼科：视力相关头痛\n🏥 内科：感冒引起的头痛";
    }
    
    // 咳嗽相关
    if (input.contains("咳嗽") || input.contains("咳痰")) {
        return "咳嗽症状分析：\n\n请描述：\n• 干咳还是有痰？\n• 持续时间？\n• 是否伴随发热？\n\n推荐科室：\n🫁 呼吸内科：持续咳嗽、咳痰\n👶 儿科：小儿咳嗽\n🏥 内科：一般性咳嗽";
    }
    
    // 腹痛相关
    if (input.contains("肚子疼") || input.contains("腹痛") || input.contains("胃痛")) {
        return "腹痛需要进一步确认：\n\n请告诉我：\n• 哪个部位疼痛？\n• 疼痛性质（刺痛/胀痛）？\n• 是否恶心呕吐？\n\n可能科室：\n🏥 消化内科：胃肠道问题\n🏥 普外科：急性腹痛\n👩‍⚕️ 妇科：女性下腹痛";
    }
    
    // 皮肤问题
    if (input.contains("皮疹") || input.contains("瘙痒") || input.contains("过敏")) {
        return "皮肤问题咨询：\n\n请描述：\n• 皮疹的形状和颜色？\n• 瘙痒程度？\n• 最近有无新接触物？\n\n推荐：\n🩺 皮肤科：各种皮肤问题\n⚡ 急诊科：严重过敏反应";
    }
    
    // 预约相关
    if (input.contains("预约") || input.contains("挂号")) {
        return "关于预约挂号：\n\n📱 预约方式：\n• 微信公众号预约\n• 手机APP预约\n• 现场挂号\n• 电话预约：400-123-4567\n\n⏰ 预约时间：\n• 普通门诊：提前3天\n• 专家门诊：提前7天\n\n需要我帮您选择合适的科室吗？";
    }
    
    // 默认响应
    return "我理解您的症状描述。为了给您更准确的建议，请提供更多详细信息：\n\n• 症状持续时间\n• 疼痛或不适程度\n• 是否伴随其他症状\n• 您的年龄范围\n\n这些信息将帮助我为您推荐最合适的科室。";
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
        addActionButtons({"🚨 立即急诊", "📞 拨打120"});
    } else if (advice.needAppointment) {
        // 需要预约，添加预约按钮
        addActionButtons({"📅 预约" + advice.department, "🔍 查看更多科室"});
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
        btn->setStyleSheet(R"(
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
        connect(btn, &QPushButton::clicked, this, &ChatWidget::onActionButtonClicked);
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
void ChatWidget::onQuickButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString buttonText = button->text();
        // 移除emoji，提取关键词
        QString cleanText = buttonText.remove(QRegularExpression("[🤒😷🤕🤧😣🔴👁️👂🦷]")).trimmed();
        
        m_messageInput->setPlainText("我想咨询" + cleanText + "的问题");
        onSendMessage();
    }
}

void ChatWidget::onInputTextChanged()
{
    bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
    m_btnSend->setEnabled(hasText && !m_isAITyping);
}

void ChatWidget::onActionButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString action = button->text();
        
        ChatMessage actionMsg;
        actionMsg.content = "您点击了：" + action + "\n\n相关功能正在开发中，敬请期待！";
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
            ChatMessage welcomeMsg;
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
    QString fileName = QFileDialog::getSaveFileName(this, 
        "保存聊天记录", 
        "聊天记录_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt",
        "文本文件 (*.txt)");
    
    if (!fileName.isEmpty()) {
        saveChatHistory();
        QMessageBox::information(this, "保存成功", "聊天记录已保存到本地数据库，并可导出为文本文件。");
    }
}

void ChatWidget::onSettingsClicked()
{
    QMessageBox::information(this, "设置", "设置功能正在开发中...");
}

// 数据库相关方法的简单实现
void ChatWidget::initDatabase()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "chat_db");
    m_database.setDatabaseName(dbPath + "/chat_history.db");
    
    if (m_database.open()) {
        QSqlQuery query(m_database);
        query.exec("CREATE TABLE IF NOT EXISTS chat_messages ("
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
    query.prepare("INSERT INTO chat_messages (session_id, content, message_type, timestamp) "
                 "VALUES (?, ?, ?, ?)");
    
    for (const ChatMessage& msg : m_chatHistory) {
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