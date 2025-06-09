#include "PatientMainWidget.h"

PatientMainWidget::PatientMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PatientMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_chatWidget = new ChatWidget;
    m_faqWidget = new FAQWidget;
    m_appointmentWidget = new AppointmentWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_chatWidget, "💬 智能客服");
    m_tabWidget->addTab(m_faqWidget, "❓ 常见问题");
    m_tabWidget->addTab(m_appointmentWidget, "📅 预约挂号");
    
    m_mainLayout->addWidget(m_tabWidget);
} 