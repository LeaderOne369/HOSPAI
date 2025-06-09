#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>

#include "src/core/DatabaseManager.h"
#include "src/views/patient/RealChatWidget.h"
#include "src/views/staff/StaffChatManager.h"
#include "src/views/common/UIStyleManager.h"

class ChatDemo : public QMainWindow
{
    Q_OBJECT

public:
    ChatDemo(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupDatabase();
        createTestUsers();
    }

private slots:
    void onLoginAsPatient()
    {
        // 模拟患者登录
        UserInfo patient;
        patient.id = 3; // 假设患者ID是3
        patient.username = "patient1";
        patient.realName = "张三";
        patient.role = "患者";
        
        if (m_patientWidget) {
            m_patientWidget->setCurrentUser(patient);
            m_tabWidget->setCurrentWidget(m_patientWidget);
        }
    }
    
    void onLoginAsStaff()
    {
        // 模拟客服登录
        UserInfo staff;
        staff.id = 2; // staff1的ID
        staff.username = "staff1";
        staff.realName = "客服小王";
        staff.role = "客服";
        
        if (m_staffWidget) {
            m_staffWidget->setCurrentUser(staff);
            m_tabWidget->setCurrentWidget(m_staffWidget);
        }
    }

private:
    void setupUI()
    {
        setWindowTitle("HospAI 聊天演示");
        resize(1200, 800);
        
        QWidget* central = new QWidget;
        setCentralWidget(central);
        
        QVBoxLayout* layout = new QVBoxLayout(central);
        
        // 标题
        QLabel* title = new QLabel("HospAI 实时聊天演示", this);
        UIStyleManager::applyLabelStyle(title, "title");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        
        // 登录按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        QPushButton* patientBtn = new QPushButton("模拟患者登录", this);
        QPushButton* staffBtn = new QPushButton("模拟客服登录", this);
        
        UIStyleManager::applyButtonStyle(patientBtn, "primary");
        UIStyleManager::applyButtonStyle(staffBtn, "success");
        
        connect(patientBtn, &QPushButton::clicked, this, &ChatDemo::onLoginAsPatient);
        connect(staffBtn, &QPushButton::clicked, this, &ChatDemo::onLoginAsStaff);
        
        buttonLayout->addWidget(patientBtn);
        buttonLayout->addWidget(staffBtn);
        layout->addLayout(buttonLayout);
        
        // 选项卡界面
        m_tabWidget = new QTabWidget;
        
        // 患者界面
        m_patientWidget = new RealChatWidget;
        m_tabWidget->addTab(m_patientWidget, "患者聊天界面");
        
        // 客服界面
        m_staffWidget = new StaffChatManager;
        m_tabWidget->addTab(m_staffWidget, "客服管理界面");
        
        layout->addWidget(m_tabWidget);
        
        // 说明文字
        QLabel* instruction = new QLabel(
            "使用说明：\n"
            "1. 点击「模拟患者登录」切换到患者界面，可以发起咨询\n"
            "2. 点击「模拟客服登录」切换到客服界面，可以看到等待接入的会话\n"
            "3. 客服双击等待列表中的会话即可接入对话\n"
            "4. 可以开启多个进程模拟多个用户同时在线", this);
        instruction->setWordWrap(true);
        instruction->setStyleSheet("color: #666; padding: 10px; background: #f5f5f5; border-radius: 5px;");
        layout->addWidget(instruction);
    }
    
    void setupDatabase()
    {
        m_dbManager = DatabaseManager::instance();
        if (!m_dbManager->initDatabase()) {
            QMessageBox::critical(this, "错误", "数据库初始化失败！");
            return;
        }
    }
    
    void createTestUsers()
    {
        // 创建测试患者账户
        m_dbManager->registerUser("patient1", "123456", "patient1@test.com", "13800138001", "患者", "张三");
        m_dbManager->registerUser("patient2", "123456", "patient2@test.com", "13800138002", "患者", "李四");
        m_dbManager->registerUser("patient3", "123456", "patient3@test.com", "13800138003", "患者", "王五");
    }

private:
    QTabWidget* m_tabWidget;
    RealChatWidget* m_patientWidget;
    StaffChatManager* m_staffWidget;
    DatabaseManager* m_dbManager;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 应用样式
    UIStyleManager::initializeStyles();
    
    ChatDemo demo;
    demo.show();
    
    return app.exec();
}

#include "chat_demo.moc" 