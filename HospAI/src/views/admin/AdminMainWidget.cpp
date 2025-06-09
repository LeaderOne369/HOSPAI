#include "AdminMainWidget.h"

AdminMainWidget::AdminMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdminMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_userManageWidget = new UserManageWidget;
    m_systemStatsWidget = new SystemStatsWidget;
    m_systemConfigWidget = new SystemConfigWidget;
    m_auditLogWidget = new AuditLogWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_userManageWidget, "👥 用户管理");
    m_tabWidget->addTab(m_systemStatsWidget, "📊 系统统计");
    m_tabWidget->addTab(m_systemConfigWidget, "⚙️ 系统配置");
    m_tabWidget->addTab(m_auditLogWidget, "📋 审计日志");
    
    m_mainLayout->addWidget(m_tabWidget);
} 