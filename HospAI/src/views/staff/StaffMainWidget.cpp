#include "StaffMainWidget.h"

StaffMainWidget::StaffMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StaffMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新聊天管理器的用户信息（如果已经创建）
    if (m_chatManager) {
        m_chatManager->setCurrentUser(user);
    }
    // 如果还没创建，user信息已保存在m_currentUser中，会在initializeComponents中设置
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
    
    // 延迟创建各个功能页面，避免在数据库未完全初始化时创建
    m_chatManager = nullptr;
    m_consultationWidget = nullptr;
    m_statsWidget = nullptr;
    m_knowledgeBaseWidget = nullptr;
    
    // 创建一个占位符页面
    QWidget* placeholderWidget = new QWidget;
    QVBoxLayout* placeholderLayout = new QVBoxLayout(placeholderWidget);
    QLabel* loadingLabel = new QLabel("正在初始化客服系统...", placeholderWidget);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("font-size: 16px; color: #666;");
    placeholderLayout->addWidget(loadingLabel);
    
    m_tabWidget->addTab(placeholderWidget, "💬 客服聊天");
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // 延迟初始化真正的组件
    QTimer::singleShot(100, this, &StaffMainWidget::initializeComponents);
}

void StaffMainWidget::initializeComponents()
{
    // 移除占位符页面
    m_tabWidget->clear();
    
    // 现在创建真正的功能页面
    m_chatManager = new StaffChatManager;
    m_consultationWidget = new ConsultationWidget;
    m_statsWidget = new StatsWidget;
    m_knowledgeBaseWidget = new KnowledgeBaseWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_chatManager, "💬 客服聊天");
    m_tabWidget->addTab(m_consultationWidget, "📋 咨询管理");
    m_tabWidget->addTab(m_statsWidget, "📊 工作统计");
    m_tabWidget->addTab(m_knowledgeBaseWidget, "📚 知识库");
    
    // 如果已经设置了用户信息，传递给组件
    if (!m_currentUser.username.isEmpty() && m_chatManager) {
        m_chatManager->setCurrentUser(m_currentUser);
    }
    
    // 如果已经设置了数据库管理器，传递给组件
    if (m_dbManager && m_chatManager) {
        // 传递数据库管理器给聊天管理器
        // m_chatManager->setDatabaseManager(m_dbManager); // TODO: 实现此方法
    }
} 