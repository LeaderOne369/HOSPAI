#include "StaffMainWidget.h"

StaffMainWidget::StaffMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StaffMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新聊天管理器的用户信息
    if (m_chatManager) {
        m_chatManager->setCurrentUser(user);
    }
}

void StaffMainWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    // 传递数据库管理器给聊天管理器
    if (m_chatManager) {
        // m_chatManager->setDatabaseManager(dbManager); // TODO: 实现此方法
    }
}

void StaffMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_chatManager = new StaffChatManager;        // 新的实时聊天管理器
    m_consultationWidget = new ConsultationWidget;
    m_statsWidget = new StatsWidget;
    m_knowledgeBaseWidget = new KnowledgeBaseWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_chatManager, "💬 客服聊天");
    m_tabWidget->addTab(m_consultationWidget, "📋 咨询管理");
    m_tabWidget->addTab(m_statsWidget, "📊 工作统计");
    m_tabWidget->addTab(m_knowledgeBaseWidget, "📚 知识库");
    
    m_mainLayout->addWidget(m_tabWidget);
} 