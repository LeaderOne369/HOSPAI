#include "PatientMainWidget.h"
#include <QDebug>

PatientMainWidget::PatientMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PatientMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新AI分诊组件的用户信息
    if (m_chatWidget) {
        m_chatWidget->setUserInfo(QString::number(user.id), user.realName.isEmpty() ? user.username : user.realName);
    }
    
    // 更新实时聊天组件的用户信息
    if (m_realChatWidget) {
        m_realChatWidget->setCurrentUser(user);
    }
}

void PatientMainWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    // 设置数据库管理器
    if (m_chatWidget) {
        m_chatWidget->setDatabaseManager(dbManager);
    }
    if (m_realChatWidget) {
        // m_realChatWidget->setDatabaseManager(dbManager); // TODO: 实现此方法
    }
}

void PatientMainWidget::onRequestHumanService(const QString& userId, const QString& userName, const QString& context)
{
    // 调试信息
    qDebug() << "接收到转人工信号！用户ID:" << userId << "用户名:" << userName;
    qDebug() << "上下文内容:" << context;
    
    // 切换到客服咨询选项卡
    m_tabWidget->setCurrentWidget(m_realChatWidget);
    qDebug() << "已切换到客服咨询选项卡";
    
    // 如果有上下文，可以将其传递给客服聊天界面
    if (m_realChatWidget && !context.isEmpty()) {
        // 可以在这里处理对话上下文的传递
        // 例如显示一个系统消息告知客服之前的AI对话内容
        qDebug() << "准备传递上下文给客服聊天界面";
    }
}

void PatientMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    
    // 创建各个功能页面
    m_chatWidget = new ChatWidget;          // AI智能分诊
    m_realChatWidget = new RealChatWidget;  // 真人客服
    m_faqWidget = new FAQWidget;
    m_appointmentWidget = new AppointmentWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_chatWidget, "🤖 智能分诊");
    m_tabWidget->addTab(m_realChatWidget, "💬 客服咨询");
    m_tabWidget->addTab(m_faqWidget, "❓ 常见问题");
    m_tabWidget->addTab(m_appointmentWidget, "📅 预约挂号");
    
    // 连接转人工信号
    connect(m_chatWidget, &ChatWidget::requestHumanService,
            this, &PatientMainWidget::onRequestHumanService);
    
    m_mainLayout->addWidget(m_tabWidget);
} 