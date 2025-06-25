#include "SystemStatsWidget.h"
#include "../common/UIStyleManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QTimer>
#include <QHeaderView>

SystemStatsWidget::SystemStatsWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void SystemStatsWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    // 设置数据库管理器后立即更新统计数据
    updateOverviewStats();
    updateUserStats();
}

void SystemStatsWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("系统统计分析", this);
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    m_mainLayout->addWidget(titleLabel);
    
    // 日期范围选择
    m_dateGroup = new QGroupBox("统计时间范围", this);
    UIStyleManager::applyGroupBoxStyle(m_dateGroup);
    QHBoxLayout* dateLayout = new QHBoxLayout(m_dateGroup);
    
    dateLayout->addWidget(new QLabel("开始日期:", this));
    m_startDate = new QDateEdit(QDate::currentDate().addDays(-30), this);
    m_startDate->setCalendarPopup(true);
    dateLayout->addWidget(m_startDate);
    
    dateLayout->addWidget(new QLabel("结束日期:", this));
    m_endDate = new QDateEdit(QDate::currentDate(), this);
    m_endDate->setCalendarPopup(true);
    dateLayout->addWidget(m_endDate);
    
    m_btnRefresh = new QPushButton("🔄 刷新", this);
    UIStyleManager::applyButtonStyle(m_btnRefresh, "primary");
    m_btnExport = new QPushButton("📊 导出", this);
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    
    dateLayout->addWidget(m_btnRefresh);
    dateLayout->addWidget(m_btnExport);
    dateLayout->addStretch();
    
    m_mainLayout->addWidget(m_dateGroup);
    
    // 连接信号
    connect(m_startDate, &QDateEdit::dateChanged, this, &SystemStatsWidget::onDateRangeChanged);
    connect(m_endDate, &QDateEdit::dateChanged, this, &SystemStatsWidget::onDateRangeChanged);
    connect(m_btnRefresh, &QPushButton::clicked, this, &SystemStatsWidget::onRefreshStats);
    connect(m_btnExport, &QPushButton::clicked, this, &SystemStatsWidget::onExportReport);
    
    // 选项卡
    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &SystemStatsWidget::onTabChanged);
    
    setupOverviewTab();
    setupUserStatsTab();
    setupSystemStatsTab();
    setupReportsTab();
    
    m_tabWidget->addTab(m_overviewTab, "📊 概览");
    m_tabWidget->addTab(m_userStatsTab, "👥 用户统计");
    m_tabWidget->addTab(m_systemStatsTab, "🔧 系统性能");
    m_tabWidget->addTab(m_reportsTab, "📋 报表");
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // 定时更新
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &SystemStatsWidget::updateOverviewStats);
    updateTimer->start(5000); // 每5秒更新一次
}

void SystemStatsWidget::setupOverviewTab()
{
    m_overviewTab = new QWidget;
    m_overviewLayout = new QGridLayout(m_overviewTab);
    m_overviewLayout->setSpacing(15);
    
    // 系统概览统计卡片
    QGroupBox* usersGroup = new QGroupBox("用户统计", this);
    UIStyleManager::applyGroupBoxStyle(usersGroup);
    QVBoxLayout* usersLayout = new QVBoxLayout(usersGroup);
    
    m_totalUsers = new QLabel("总用户数: <b>0</b>", this);
    m_activeUsers = new QLabel("活跃用户: <b>0</b>", this);
    m_totalChats = new QLabel("总对话数: <b>0</b>", this);
    
    UIStyleManager::applyLabelStyle(m_totalUsers, "normal");
    UIStyleManager::applyLabelStyle(m_activeUsers, "success");
    UIStyleManager::applyLabelStyle(m_totalChats, "normal");
    
    usersLayout->addWidget(m_totalUsers);
    usersLayout->addWidget(m_activeUsers);
    usersLayout->addWidget(m_totalChats);
    
    // 性能统计卡片
    QGroupBox* perfGroup = new QGroupBox("系统性能", this);
    UIStyleManager::applyGroupBoxStyle(perfGroup);
    QVBoxLayout* perfLayout = new QVBoxLayout(perfGroup);
    
    m_avgResponseTime = new QLabel("平均响应时间: <b>1.2秒</b>", this);
    UIStyleManager::applyLabelStyle(m_avgResponseTime, "normal");
    perfLayout->addWidget(m_avgResponseTime);
    
    QLabel* sysLoadLabel = new QLabel("系统负载:", this);
    m_systemLoad = new QProgressBar(this);
    m_systemLoad->setRange(0, 100);
    m_systemLoad->setValue(35);
    m_systemLoad->setFormat("%p%");
    
    QLabel* memUsageLabel = new QLabel("内存使用:", this);
    m_memoryUsage = new QProgressBar(this);
    m_memoryUsage->setRange(0, 100);
    m_memoryUsage->setValue(67);
    m_memoryUsage->setFormat("%p%");
    
    perfLayout->addWidget(sysLoadLabel);
    perfLayout->addWidget(m_systemLoad);
    perfLayout->addWidget(memUsageLabel);
    perfLayout->addWidget(m_memoryUsage);
    
    m_overviewLayout->addWidget(usersGroup, 0, 0);
    m_overviewLayout->addWidget(perfGroup, 0, 1);
    
    // 今日活动摘要
    QGroupBox* activityGroup = new QGroupBox("今日活动摘要", this);
    UIStyleManager::applyGroupBoxStyle(activityGroup);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityGroup);
    
    QLabel* todayLogins = new QLabel("• 今日登录: 234 次", this);
    QLabel* todayChats = new QLabel("• 新建对话: 89 个", this);
    QLabel* todayIssues = new QLabel("• 解决问题: 156 个", this);
    QLabel* todayRating = new QLabel("• 平均评分: ⭐⭐⭐⭐⭐ (4.6/5.0)", this);
    
    activityLayout->addWidget(todayLogins);
    activityLayout->addWidget(todayChats);
    activityLayout->addWidget(todayIssues);
    activityLayout->addWidget(todayRating);
    
    m_overviewLayout->addWidget(activityGroup, 1, 0, 1, 2);
}

void SystemStatsWidget::setupUserStatsTab()
{
    m_userStatsTab = new QWidget;
    m_userStatsLayout = new QHBoxLayout(m_userStatsTab);
    
    // 用户角色分布图表（简化版）
    m_userPieChart = new QLabel(this);
    m_userPieChart->setText(
        "📊 用户角色分布\n\n"
        "👥 患者: 879 人 (70.5%)\n"
        "🛎️ 客服: 314 人 (25.2%)\n"
        "👑 管理员: 54 人 (4.3%)\n\n"
        "总计: 1,247 人"
    );
    m_userPieChart->setStyleSheet(
        "QLabel {"
        "    background-color: #f8f9fa;"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "    font-size: 14px;"
        "    line-height: 1.5;"
        "}"
    );
    m_userPieChart->setAlignment(Qt::AlignCenter);
    m_userPieChart->setMinimumHeight(300);
    
    // 用户详细统计表
    m_userStatsTable = new QTableWidget;
    m_userStatsTable->setColumnCount(4);
    QStringList headers = {"角色", "总数", "活跃", "占比"};
    m_userStatsTable->setHorizontalHeaderLabels(headers);
    
    // 初始为空，将通过updateUserStats()方法填充真实数据
    
    m_userStatsTable->horizontalHeader()->setStretchLastSection(true);
    m_userStatsTable->setAlternatingRowColors(true);
    
    m_userStatsLayout->addWidget(m_userPieChart);
    m_userStatsLayout->addWidget(m_userStatsTable);
}

void SystemStatsWidget::setupSystemStatsTab()
{
    m_systemStatsTab = new QWidget;
    m_systemStatsLayout = new QVBoxLayout(m_systemStatsTab);
    
    // 性能趋势图（简化版）
    m_performanceChart = new QLabel(this);
    m_performanceChart->setText(
        "📈 24小时系统性能趋势\n\n"
        "🔥 CPU使用率: 平均 35%，峰值 58%\n"
        "💾 内存使用率: 平均 65%，峰值 82%\n"
        "🌐 网络流量: 上传 2.3GB，下载 5.7GB\n"
        "💿 磁盘I/O: 读取 1.2GB/s，写入 0.8GB/s\n\n"
        "✅ 系统运行正常，性能良好"
    );
    m_performanceChart->setStyleSheet(
        "QLabel {"
        "    background-color: #f8f9fa;"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "}"
    );
    m_performanceChart->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_performanceChart->setMinimumHeight(200);
    
    // 资源详情
    m_resourceGroup = new QGroupBox("系统资源详情", this);
    UIStyleManager::applyGroupBoxStyle(m_resourceGroup);
    QGridLayout* resourceLayout = new QGridLayout(m_resourceGroup);
    
    QLabel* osLabel = new QLabel("操作系统:", this);
    QLabel* osValue = new QLabel("Linux Ubuntu 24.04", this);
    QLabel* qtLabel = new QLabel("Qt版本:", this);
    QLabel* qtValue = new QLabel(QT_VERSION_STR, this);
    QLabel* uptimeLabel = new QLabel("运行时间:", this);
    QLabel* uptimeValue = new QLabel("7天 3小时 25分钟", this);
    QLabel* dbLabel = new QLabel("数据库:", this);
    QLabel* dbValue = new QLabel("SQLite 3.45.0", this);
    
    resourceLayout->addWidget(osLabel, 0, 0);
    resourceLayout->addWidget(osValue, 0, 1);
    resourceLayout->addWidget(qtLabel, 1, 0);
    resourceLayout->addWidget(qtValue, 1, 1);
    resourceLayout->addWidget(uptimeLabel, 2, 0);
    resourceLayout->addWidget(uptimeValue, 2, 1);
    resourceLayout->addWidget(dbLabel, 3, 0);
    resourceLayout->addWidget(dbValue, 3, 1);
    
    m_systemStatsLayout->addWidget(m_performanceChart);
    m_systemStatsLayout->addWidget(m_resourceGroup);
}

void SystemStatsWidget::setupReportsTab()
{
    m_reportsTab = new QWidget;
    m_reportsLayout = new QVBoxLayout(m_reportsTab);
    
    // 报表类型选择
    QHBoxLayout* reportLayout = new QHBoxLayout;
    QLabel* typeLabel = new QLabel("报表类型:", this);
    m_reportType = new QComboBox;
    m_reportType->addItems({"用户活动报表", "对话统计报表", "系统性能报表", "错误日志报表"});
    
    reportLayout->addWidget(typeLabel);
    reportLayout->addWidget(m_reportType);
    reportLayout->addStretch();
    
    m_reportsLayout->addLayout(reportLayout);
    
    // 报表内容表格
    m_reportTable = new QTableWidget;
    m_reportTable->setColumnCount(5);
    QStringList headers = {"时间", "用户", "操作", "结果", "详情"};
    m_reportTable->setHorizontalHeaderLabels(headers);
    
    // 添加示例数据
    m_reportTable->setRowCount(10);
    for (int i = 0; i < 10; ++i) {
        m_reportTable->setItem(i, 0, new QTableWidgetItem(QDateTime::currentDateTime().addSecs(-i * 3600).toString("yyyy-MM-dd hh:mm")));
        m_reportTable->setItem(i, 1, new QTableWidgetItem(QString("用户%1").arg(i + 1)));
        m_reportTable->setItem(i, 2, new QTableWidgetItem("登录"));
        m_reportTable->setItem(i, 3, new QTableWidgetItem("成功"));
        m_reportTable->setItem(i, 4, new QTableWidgetItem("正常登录"));
    }
    
    m_reportTable->horizontalHeader()->setStretchLastSection(true);
    m_reportTable->setAlternatingRowColors(true);
    
    m_reportsLayout->addWidget(m_reportTable);
}

void SystemStatsWidget::updateOverviewStats()
{
    if (!m_dbManager) {
        // 如果没有数据库管理器，使用模拟数据
        static int counter = 0;
        counter++;
        
        int sysLoad = 30 + (counter % 40);
        int memUsage = 50 + (counter % 30);
        
        m_systemLoad->setValue(sysLoad);
        m_memoryUsage->setValue(memUsage);
        
        m_totalUsers->setText(QString("总用户数: <b>%1</b>").arg(1247 + counter % 10));
        m_activeUsers->setText(QString("活跃用户: <b>%1</b>").arg(892 + counter % 5));
        return;
    }
    
    // 获取真实的用户统计数据
    QList<UserInfo> allUsers = m_dbManager->getAllUsers();
    int totalUsers = allUsers.size();
    
    // 计算活跃用户（7天内登录的用户）
    QDateTime sevenDaysAgo = QDateTime::currentDateTime().addDays(-7);
    int activeUsers = 0;
    for (const UserInfo& user : allUsers) {
        if (user.lastLoginTime.isValid() && user.lastLoginTime > sevenDaysAgo) {
            activeUsers++;
        }
    }
    
    // 获取聊天会话总数
    QList<ChatSession> allSessions = m_dbManager->getActiveSessions();
    int totalChats = allSessions.size();
    
    // 更新显示
    m_totalUsers->setText(QString("总用户数: <b>%1</b>").arg(totalUsers));
    m_activeUsers->setText(QString("活跃用户: <b>%1</b>").arg(activeUsers));
    m_totalChats->setText(QString("总对话数: <b>%1</b>").arg(totalChats));
    
    // 模拟系统负载和内存使用的变化
    static int counter = 0;
    counter++;
    int sysLoad = 30 + (counter % 40);
    int memUsage = 50 + (counter % 30);
    
    m_systemLoad->setValue(sysLoad);
    m_memoryUsage->setValue(memUsage);
}

void SystemStatsWidget::updateUserStats()
{
    if (!m_dbManager) return;
    
    // 获取用户数据
    QList<UserInfo> allUsers = m_dbManager->getAllUsers();
    
    // 按角色分类统计
    QMap<QString, int> roleCounts;
    QMap<QString, int> activeRoleCounts;
    
    QDateTime sevenDaysAgo = QDateTime::currentDateTime().addDays(-7);
    
    for (const UserInfo& user : allUsers) {
        roleCounts[user.role]++;
        if (user.lastLoginTime.isValid() && user.lastLoginTime > sevenDaysAgo) {
            activeRoleCounts[user.role]++;
        }
    }
    
    int totalUsers = allUsers.size();
    
    // 更新饼图文本
    QString pieText = "📊 用户角色分布\n\n";
    QStringList roles = {"患者", "客服", "管理员"};
    
    for (const QString& role : roles) {
        int count = roleCounts.value(role, 0);
        int activeCount = activeRoleCounts.value(role, 0);
        double percentage = totalUsers > 0 ? (count * 100.0 / totalUsers) : 0;
        
        QString emoji = "👥";
        if (role == "客服") emoji = "🛎️";
        else if (role == "管理员") emoji = "👑";
        
        pieText += QString("%1 %2: %3 人 (%4%)\n").arg(emoji, role).arg(count).arg(percentage, 0, 'f', 1);
    }
    pieText += QString("\n总计: %1 人").arg(totalUsers);
    
    m_userPieChart->setText(pieText);
    
    // 更新表格
    m_userStatsTable->setRowCount(roles.size());
    
    for (int i = 0; i < roles.size(); ++i) {
        const QString& role = roles[i];
        int count = roleCounts.value(role, 0);
        int activeCount = activeRoleCounts.value(role, 0);
        double percentage = totalUsers > 0 ? (count * 100.0 / totalUsers) : 0;
        
        m_userStatsTable->setItem(i, 0, new QTableWidgetItem(role));
        m_userStatsTable->setItem(i, 1, new QTableWidgetItem(QString::number(count)));
        m_userStatsTable->setItem(i, 2, new QTableWidgetItem(QString::number(activeCount)));
        m_userStatsTable->setItem(i, 3, new QTableWidgetItem(QString("%1%").arg(percentage, 0, 'f', 1)));
    }
}

void SystemStatsWidget::updateSystemStats()
{
    // 更新系统性能数据
}

void SystemStatsWidget::createCharts()
{
    // 创建或更新图表
}

void SystemStatsWidget::onDateRangeChanged()
{
    // 日期范围改变时触发
    QDate start = m_startDate->date();
    QDate end = m_endDate->date();
    
    if (start > end) {
        QMessageBox::warning(this, "日期错误", "开始日期不能晚于结束日期！");
        m_startDate->setDate(end.addDays(-7));
    }
}

void SystemStatsWidget::onRefreshStats()
{
    // 刷新统计数据
    updateOverviewStats();
    updateUserStats();
    updateSystemStats();
    
    QMessageBox::information(this, "刷新", "统计数据已刷新！");
}

void SystemStatsWidget::onExportReport()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出统计报表",
        QString("stats_report_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV文件 (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // Qt5/Qt6兼容性：在Qt6中setCodec已移除，默认使用UTF-8
            #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out.setCodec("UTF-8");
            #endif
            
            // 写入CSV头部
            out << "时间,用户,操作,结果,详情\n";
            
            // 写入表格数据
            for (int row = 0; row < m_reportTable->rowCount(); ++row) {
                QStringList rowData;
                for (int col = 0; col < m_reportTable->columnCount(); ++col) {
                    QTableWidgetItem* item = m_reportTable->item(row, col);
                    rowData << (item ? item->text() : "");
                }
                out << rowData.join(",") << "\n";
            }
            
            QMessageBox::information(this, "导出成功", QString("报表已导出到: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, "导出失败", "无法创建文件！");
        }
    }
}

void SystemStatsWidget::onTabChanged(int index)
{
    Q_UNUSED(index)
    // 切换选项卡时可以执行特定操作
} 