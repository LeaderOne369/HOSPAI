#include "StaffMainWidget.h"

StaffMainWidget::StaffMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StaffMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_consultationWidget = new ConsultationWidget;
    m_statsWidget = new StatsWidget;
    m_knowledgeBaseWidget = new KnowledgeBaseWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_consultationWidget, "💬 在线咨询");
    m_tabWidget->addTab(m_statsWidget, "📊 工作统计");
    m_tabWidget->addTab(m_knowledgeBaseWidget, "📚 知识库");
    
    m_mainLayout->addWidget(m_tabWidget);
} 