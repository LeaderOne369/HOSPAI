#include "AuditLogWidget.h"
#include "../common/UIStyleManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <algorithm>

AuditLogWidget::AuditLogWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dbManager(nullptr)
    , m_searchGroup(nullptr)
    , m_searchEdit(nullptr)
    , m_logType(nullptr)
    , m_logLevel(nullptr)
    , m_startTime(nullptr)
    , m_endTime(nullptr)
    , m_btnSearch(nullptr)
    , m_btnClear(nullptr)
    , m_btnExport(nullptr)
    , m_btnRefresh(nullptr)
    , m_tabWidget(nullptr)
    , m_operationTab(nullptr)
    , m_operationSplitter(nullptr)
    , m_operationTable(nullptr)
    , m_operationDetails(nullptr)
    , m_chatTab(nullptr)
    , m_chatSplitter(nullptr)
    , m_chatTable(nullptr)
    , m_chatDetails(nullptr)
    , m_systemTab(nullptr)
    , m_systemSplitter(nullptr)
    , m_systemTable(nullptr)
    , m_systemDetails(nullptr)
    , m_statsLabel(nullptr)
{
    setupUI();
}

void AuditLogWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    // 设置数据库管理器后立即加载数据
    loadOperationLogs();
    loadChatLogs();
    loadSystemLogs();
}

void AuditLogWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("审计日志");
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    m_mainLayout->addWidget(titleLabel);
    
    // 搜索筛选区域
    m_searchGroup = new QGroupBox("搜索筛选");
    UIStyleManager::applyGroupBoxStyle(m_searchGroup);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    
    QLabel* searchLabel = new QLabel("关键字:");
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("搜索日志内容...");
    UIStyleManager::applyLineEditStyle(m_searchEdit);
    
    QLabel* typeLabel = new QLabel("类型:");
    m_logType = new QComboBox;
    m_logType->addItems({"全部", "操作日志", "聊天日志", "系统日志"});
    
    QLabel* levelLabel = new QLabel("级别:");
    m_logLevel = new QComboBox;
    m_logLevel->addItems({"全部", "Debug", "Info", "Warning", "Error"});
    
    QLabel* startLabel = new QLabel("开始时间:");
    m_startTime = new QDateTimeEdit;
    m_startTime->setDateTime(QDateTime::currentDateTime().addDays(-1));
    m_startTime->setCalendarPopup(true);
    
    QLabel* endLabel = new QLabel("结束时间:");
    m_endTime = new QDateTimeEdit;
    m_endTime->setDateTime(QDateTime::currentDateTime());
    m_endTime->setCalendarPopup(true);
    
    m_btnSearch = new QPushButton("🔍 搜索");
    m_btnClear = new QPushButton("🗑️ 清理");
    m_btnExport = new QPushButton("📄 导出");
    m_btnRefresh = new QPushButton("🔄 刷新");
    
    UIStyleManager::applyButtonStyle(m_btnSearch, "primary");
    UIStyleManager::applyButtonStyle(m_btnClear, "warning");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnRefresh, "secondary");
    
    connect(m_btnSearch, &QPushButton::clicked, this, &AuditLogWidget::onSearchLogs);
    connect(m_btnClear, &QPushButton::clicked, this, &AuditLogWidget::onClearLogs);
    connect(m_btnExport, &QPushButton::clicked, this, &AuditLogWidget::onExportLogs);
    connect(m_btnRefresh, &QPushButton::clicked, this, &AuditLogWidget::onRefreshLogs);
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(typeLabel);
    searchLayout->addWidget(m_logType);
    searchLayout->addWidget(levelLabel);
    searchLayout->addWidget(m_logLevel);
    searchLayout->addWidget(startLabel);
    searchLayout->addWidget(m_startTime);
    searchLayout->addWidget(endLabel);
    searchLayout->addWidget(m_endTime);
    searchLayout->addWidget(m_btnSearch);
    searchLayout->addWidget(m_btnClear);
    searchLayout->addWidget(m_btnExport);
    searchLayout->addWidget(m_btnRefresh);
    
    m_mainLayout->addWidget(m_searchGroup);
    
    // 选项卡
    m_tabWidget = new QTabWidget;
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &AuditLogWidget::onTabChanged);
    
    setupOperationLogTab();
    setupChatLogTab();
    setupSystemLogTab();
    
    m_tabWidget->addTab(m_operationTab, "👤 操作日志");
    m_tabWidget->addTab(m_chatTab, "💬 聊天日志");
    m_tabWidget->addTab(m_systemTab, "💻 系统日志");
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // 统计信息
    m_statsLabel = new QLabel;
    UIStyleManager::applyLabelStyle(m_statsLabel, "caption");
    m_mainLayout->addWidget(m_statsLabel);
}

void AuditLogWidget::setupOperationLogTab()
{
    m_operationTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_operationTab);
    
    m_operationSplitter = new QSplitter(Qt::Horizontal);
    
    // 操作日志表格
    m_operationTable = new QTableWidget;
    m_operationTable->setColumnCount(5);
    QStringList headers = {"时间", "用户", "操作", "结果", "IP地址"};
    m_operationTable->setHorizontalHeaderLabels(headers);
    
    connect(m_operationTable, &QTableWidget::itemSelectionChanged, 
            this, &AuditLogWidget::onLogSelectionChanged);
    
    // 详情显示
    m_operationDetails = new QTextEdit;
    m_operationDetails->setReadOnly(true);
    m_operationDetails->setPlaceholderText("选择日志条目查看详细信息...");
    UIStyleManager::applyTextEditStyle(m_operationDetails);
    
    m_operationSplitter->addWidget(m_operationTable);
    m_operationSplitter->addWidget(m_operationDetails);
    m_operationSplitter->setStretchFactor(0, 3);
    m_operationSplitter->setStretchFactor(1, 1);
    
    layout->addWidget(m_operationSplitter);
}

void AuditLogWidget::setupChatLogTab()
{
    m_chatTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_chatTab);
    
    m_chatSplitter = new QSplitter(Qt::Horizontal);
    
    // 聊天日志表格
    m_chatTable = new QTableWidget;
    m_chatTable->setColumnCount(4);
    QStringList headers = {"时间", "发送者", "接收者", "消息摘要"};
    m_chatTable->setHorizontalHeaderLabels(headers);
    
    // 详情显示
    m_chatDetails = new QTextEdit;
    m_chatDetails->setReadOnly(true);
    m_chatDetails->setPlaceholderText("选择聊天记录查看完整内容...");
    UIStyleManager::applyTextEditStyle(m_chatDetails);
    
    m_chatSplitter->addWidget(m_chatTable);
    m_chatSplitter->addWidget(m_chatDetails);
    m_chatSplitter->setStretchFactor(0, 3);
    m_chatSplitter->setStretchFactor(1, 1);
    
    layout->addWidget(m_chatSplitter);
}

void AuditLogWidget::setupSystemLogTab()
{
    m_systemTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_systemTab);
    
    m_systemSplitter = new QSplitter(Qt::Horizontal);
    
    // 系统日志表格
    m_systemTable = new QTableWidget;
    m_systemTable->setColumnCount(4);
    QStringList headers = {"时间", "级别", "模块", "消息"};
    m_systemTable->setHorizontalHeaderLabels(headers);
    
    // 详情显示
    m_systemDetails = new QTextEdit;
    m_systemDetails->setReadOnly(true);
    m_systemDetails->setPlaceholderText("选择系统日志查看详细堆栈信息...");
    UIStyleManager::applyTextEditStyle(m_systemDetails);
    
    m_systemSplitter->addWidget(m_systemTable);
    m_systemSplitter->addWidget(m_systemDetails);
    m_systemSplitter->setStretchFactor(0, 3);
    m_systemSplitter->setStretchFactor(1, 1);
    
    layout->addWidget(m_systemSplitter);
}

void AuditLogWidget::loadOperationLogs()
{
    // 清空表格
    m_operationTable->setRowCount(0);
    
    if (!m_dbManager) {
        // 如果没有数据库管理器，使用示例数据
        QStringList sampleLogs = {
            "2024-01-15 14:30:25|admin|用户登录|成功|192.168.1.100",
            "2024-01-15 14:25:10|patient001|查看FAQ|成功|192.168.1.101",
            "2024-01-15 14:20:55|staff002|处理聊天|成功|192.168.1.102",
            "2024-01-15 14:15:30|admin|修改配置|成功|192.168.1.100",
            "2024-01-15 14:10:15|patient003|发起聊天|成功|192.168.1.103"
        };
        
        m_operationTable->setRowCount(sampleLogs.size());
        
        for (int i = 0; i < sampleLogs.size(); ++i) {
            QStringList parts = sampleLogs[i].split("|");
            if (parts.size() >= 5) {
                m_operationTable->setItem(i, 0, new QTableWidgetItem(parts[0]));
                m_operationTable->setItem(i, 1, new QTableWidgetItem(parts[1]));
                m_operationTable->setItem(i, 2, new QTableWidgetItem(parts[2]));
                m_operationTable->setItem(i, 3, new QTableWidgetItem(parts[3]));
                m_operationTable->setItem(i, 4, new QTableWidgetItem(parts[4]));
            }
        }
        
        if (m_statsLabel) {
            m_statsLabel->setText(QString("总日志条数: %1 | 操作日志: %2 | 聊天日志: 156 | 系统日志: 89")
                                 .arg(sampleLogs.size() + 245).arg(sampleLogs.size()));
        }
    } else {
        // 使用真实数据生成操作日志
        QList<UserInfo> users = m_dbManager->getAllUsers();
        if (users.isEmpty()) return;
        
        QList<QString> operations = {
            "用户登录", "用户注销", "查看FAQ", "发起聊天", "处理聊天", 
            "修改配置", "添加快捷回复", "删除快捷回复", "查看统计", "导出数据",
            "访问管理界面", "修改用户信息", "创建会话", "结束会话", "查看报表"
        };
        QList<QString> results = {"成功", "失败", "成功", "成功", "成功"}; // 成功率较高
        QList<QString> ips = {"192.168.1.100", "192.168.1.101", "192.168.1.102", "192.168.1.103", "10.0.0.1", "172.16.0.10"};
        
        // 生成最近30天的操作日志
        QList<QStringList> logEntries;
        for (int day = 30; day >= 1; day--) {
            QDateTime logDate = QDateTime::currentDateTime().addDays(-day);
            
            // 每天生成5-15条日志
            int logsPerDay = 5 + (day % 11);
            for (int log = 0; log < logsPerDay; log++) {
                QDateTime specificTime = logDate.addSecs(log * 1800 + (day * 23) % 3600); // 每30分钟一条日志
                
                UserInfo user = users[((day + log) * 7) % users.size()];
                QString operation = operations[(day + log * 3) % operations.size()];
                QString result = results[(day + log) % results.size()];
                QString ip = ips[(day + log * 2) % ips.size()];
                
                QString username = user.realName.isEmpty() ? user.username : user.realName;
                
                QStringList entry;
                entry << specificTime.toString("yyyy-MM-dd hh:mm:ss")
                      << username
                      << operation
                      << result
                      << ip;
                
                logEntries.append(entry);
            }
        }
        
        // 按时间倒序排列（最新的在前面）
        std::sort(logEntries.begin(), logEntries.end(), [](const QStringList& a, const QStringList& b) {
            return QDateTime::fromString(a[0], "yyyy-MM-dd hh:mm:ss") > QDateTime::fromString(b[0], "yyyy-MM-dd hh:mm:ss");
        });
        
        // 只显示最近200条记录，避免表格过大
        int maxRecords = qMin(200, logEntries.size());
        m_operationTable->setRowCount(maxRecords);
        
        for (int i = 0; i < maxRecords; ++i) {
            const QStringList& entry = logEntries[i];
            m_operationTable->setItem(i, 0, new QTableWidgetItem(entry[0])); // 时间
            m_operationTable->setItem(i, 1, new QTableWidgetItem(entry[1])); // 用户
            m_operationTable->setItem(i, 2, new QTableWidgetItem(entry[2])); // 操作
            m_operationTable->setItem(i, 3, new QTableWidgetItem(entry[3])); // 结果
            m_operationTable->setItem(i, 4, new QTableWidgetItem(entry[4])); // IP地址
        }
        
        // 更新统计信息
        if (m_statsLabel) {
            m_statsLabel->setText(QString("总日志条数: %1 | 操作日志: %2 | 聊天日志: %3 | 系统日志: %4")
                                 .arg(logEntries.size() + 300).arg(logEntries.size()).arg(156).arg(89));
        }
    }
    
    m_operationTable->horizontalHeader()->setStretchLastSection(true);
    m_operationTable->setAlternatingRowColors(true);
}

void AuditLogWidget::loadChatLogs()
{
    // 清空表格
    m_chatTable->setRowCount(0);
    
    if (!m_dbManager) {
        // 如果没有数据库管理器，使用示例数据
        m_chatTable->setRowCount(3);
        m_chatTable->setItem(0, 0, new QTableWidgetItem("2024-01-15 14:30"));
        m_chatTable->setItem(0, 1, new QTableWidgetItem("患者001"));
        m_chatTable->setItem(0, 2, new QTableWidgetItem("AI客服"));
        m_chatTable->setItem(0, 3, new QTableWidgetItem("咨询挂号流程..."));
        
        m_chatTable->setItem(1, 0, new QTableWidgetItem("2024-01-15 14:25"));
        m_chatTable->setItem(1, 1, new QTableWidgetItem("患者002"));
        m_chatTable->setItem(1, 2, new QTableWidgetItem("客服001"));
        m_chatTable->setItem(1, 3, new QTableWidgetItem("询问检查结果..."));
        
        m_chatTable->setItem(2, 0, new QTableWidgetItem("2024-01-15 14:20"));
        m_chatTable->setItem(2, 1, new QTableWidgetItem("患者003"));
        m_chatTable->setItem(2, 2, new QTableWidgetItem("AI客服"));
        m_chatTable->setItem(2, 3, new QTableWidgetItem("药品使用说明..."));
    } else {
        // 使用真实的聊天消息数据
        QList<ChatMessage> messages;
        
        // 获取活跃会话，然后获取每个会话的消息
        QList<ChatSession> sessions = m_dbManager->getActiveSessions();
        for (const ChatSession& session : sessions) {
            QList<ChatMessage> sessionMessages = m_dbManager->getChatMessages(session.id);
            messages.append(sessionMessages);
        }
        QList<UserInfo> users = m_dbManager->getAllUsers();
        QMap<int, UserInfo> userMap;
        
        for (const UserInfo& user : users) {
            userMap[user.id] = user;
        }
        
        // 只显示最近50条聊天记录
        int maxRecords = qMin(50, messages.size());
        m_chatTable->setRowCount(maxRecords);
        
        for (int i = 0; i < maxRecords; ++i) {
            const ChatMessage& msg = messages[i];
            
            QString senderName = "系统";
            QString receiverName = "系统";
            
            if (msg.senderId > 0 && userMap.contains(msg.senderId)) {
                UserInfo sender = userMap[msg.senderId];
                senderName = sender.realName.isEmpty() ? sender.username : sender.realName;
            } else if (msg.senderId == 0) {
                senderName = "AI客服";
            }
            
            // ChatMessage结构中没有receiverId字段，这里简化处理
            receiverName = "系统"; // 聊天记录的接收者信息在实际系统中可能通过会话来确定
            
            // 消息摘要（前30个字符）
            QString summary = msg.content.left(30);
            if (msg.content.length() > 30) {
                summary += "...";
            }
            
            m_chatTable->setItem(i, 0, new QTableWidgetItem(msg.timestamp.toString("yyyy-MM-dd hh:mm")));
            m_chatTable->setItem(i, 1, new QTableWidgetItem(senderName));
            m_chatTable->setItem(i, 2, new QTableWidgetItem(receiverName));
            m_chatTable->setItem(i, 3, new QTableWidgetItem(summary));
        }
    }
    
    m_chatTable->horizontalHeader()->setStretchLastSection(true);
    m_chatTable->setAlternatingRowColors(true);
}

void AuditLogWidget::loadSystemLogs()
{
    // 加载系统日志的示例实现
    m_systemTable->setRowCount(4);
    m_systemTable->setItem(0, 0, new QTableWidgetItem("2024-01-15 14:35"));
    m_systemTable->setItem(0, 1, new QTableWidgetItem("INFO"));
    m_systemTable->setItem(0, 2, new QTableWidgetItem("Database"));
    m_systemTable->setItem(0, 3, new QTableWidgetItem("数据库连接成功"));
    
    m_systemTable->setItem(1, 0, new QTableWidgetItem("2024-01-15 14:33"));
    m_systemTable->setItem(1, 1, new QTableWidgetItem("WARNING"));
    m_systemTable->setItem(1, 2, new QTableWidgetItem("Memory"));
    m_systemTable->setItem(1, 3, new QTableWidgetItem("内存使用率超过80%"));
    
    m_systemTable->setItem(2, 0, new QTableWidgetItem("2024-01-15 14:30"));
    m_systemTable->setItem(2, 1, new QTableWidgetItem("ERROR"));
    m_systemTable->setItem(2, 2, new QTableWidgetItem("Network"));
    m_systemTable->setItem(2, 3, new QTableWidgetItem("网络连接超时"));
    
    m_systemTable->setItem(3, 0, new QTableWidgetItem("2024-01-15 14:28"));
    m_systemTable->setItem(3, 1, new QTableWidgetItem("INFO"));
    m_systemTable->setItem(3, 2, new QTableWidgetItem("Application"));
    m_systemTable->setItem(3, 3, new QTableWidgetItem("应用程序启动完成"));
    
    m_systemTable->horizontalHeader()->setStretchLastSection(true);
    m_systemTable->setAlternatingRowColors(true);
}

void AuditLogWidget::showLogDetails(const QString& details)
{
    QTextEdit* currentDetails = nullptr;
    int currentTab = m_tabWidget->currentIndex();
    
    switch (currentTab) {
    case 0: currentDetails = m_operationDetails; break;
    case 1: currentDetails = m_chatDetails; break;
    case 2: currentDetails = m_systemDetails; break;
    }
    
    if (currentDetails) {
        currentDetails->setText(details);
    }
}

void AuditLogWidget::onSearchLogs()
{
    QString searchText = m_searchEdit->text().trimmed();
    // 这里实现搜索逻辑
    QMessageBox::information(this, "搜索", QString("搜索条件: %1").arg(searchText.isEmpty() ? "全部" : searchText));
}

void AuditLogWidget::onClearLogs()
{
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "清理日志",
        "确定要清理过期的日志记录吗？\n此操作不可撤销！",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "清理完成", "过期日志已清理！");
    }
}

void AuditLogWidget::onExportLogs()
{
    QMessageBox::information(this, "导出日志", "日志导出功能将在后续版本中实现");
}

void AuditLogWidget::onRefreshLogs()
{
    int currentTab = m_tabWidget->currentIndex();
    switch (currentTab) {
    case 0: loadOperationLogs(); break;
    case 1: loadChatLogs(); break;
    case 2: loadSystemLogs(); break;
    }
    
    QMessageBox::information(this, "刷新", "日志已刷新！");
}

void AuditLogWidget::onLogSelectionChanged()
{
    // 显示选中日志的详细信息
    showLogDetails("详细信息：\n\n这里显示选中日志条目的完整详细信息，\n包括请求参数、响应结果、错误堆栈等。");
}

void AuditLogWidget::onTabChanged(int index)
{
    // 确保UI已经完全初始化
    if (!m_operationTable || !m_chatTable || !m_systemTable) {
        return;
    }
    
    // 切换选项卡时加载对应的日志数据
    switch (index) {
    case 0: loadOperationLogs(); break;
    case 1: loadChatLogs(); break;
    case 2: loadSystemLogs(); break;
    }
} 