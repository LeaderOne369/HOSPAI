#include "AdminMainWidget.h"

AdminMainWidget::AdminMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
{
    setupUI();
}

void AdminMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    // 管理员界面可以在这里根据需要更新组件状态
}

void AdminMainWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    // 传递数据库管理器给各个子组件
    if (m_userManageWidget) {
        m_userManageWidget->setDatabaseManager(dbManager);
    }
    if (m_patientManageWidget) {
        m_patientManageWidget->setDatabaseManager(dbManager);
    }
    if (m_quickReplyManageWidget) {
        m_quickReplyManageWidget->setDatabaseManager(dbManager);
    }
    if (m_staffRatingWidget) {
        m_staffRatingWidget->setDatabaseManager(dbManager);
    }
    if (m_systemStatsWidget) {
        m_systemStatsWidget->setDatabaseManager(dbManager);
    }
    if (m_auditLogWidget) {
        m_auditLogWidget->setDatabaseManager(dbManager);
    }
}

void AdminMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_userManageWidget = new UserManageWidget;
    m_patientManageWidget = new PatientManageWidget;
    m_quickReplyManageWidget = new QuickReplyManageWidget;
    m_staffRatingWidget = new StaffRatingWidget;
    m_systemStatsWidget = new SystemStatsWidget;
    m_systemConfigWidget = new SystemConfigWidget;
    m_auditLogWidget = new AuditLogWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_userManageWidget, "👥 用户管理");
    m_tabWidget->addTab(m_patientManageWidget, "🏥 患者管理");
    m_tabWidget->addTab(m_quickReplyManageWidget, "💬 快捷回复");
    m_tabWidget->addTab(m_staffRatingWidget, "⭐ 满意度评价");
    m_tabWidget->addTab(m_systemStatsWidget, "📊 系统统计");
    m_tabWidget->addTab(m_systemConfigWidget, "⚙️ 系统配置");
    m_tabWidget->addTab(m_auditLogWidget, "📋 审计日志");
    
    m_mainLayout->addWidget(m_tabWidget);
} 