#include "src/views/common/RoleSelector.h"
#include "src/views/patient/PatientWindow.h"
#include "src/views/staff/StaffWindow.h"
#include "src/views/admin/AdminWindow.h"
#include "mainwindow.h"
#include "src/views/common/ExampleUsageWidget.h"
#include "src/views/common/UIStyleManager.h"
#include "src/core/DatabaseManager.h"
#include "src/views/common/LoginDialog.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QSysInfo>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>

// 创建一个启动选择对话框
class StartupDialog : public QDialog
{
    Q_OBJECT
public:
    enum StartupMode {
        OriginalApp,
        DemoApp
    };

    explicit StartupDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("🏥 HospAI 启动选择");
        setFixedSize(400, 300);
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        UIStyleManager::applyContainerSpacing(this);
        
        // 标题
        QLabel *title = new QLabel("欢迎使用 HospAI 系统", this);
        UIStyleManager::applyLabelStyle(title, "title");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        
        // 说明文字
        QLabel *desc = new QLabel("请选择您要启动的模式：", this);
        UIStyleManager::applyLabelStyle(desc, "normal");
        desc->setAlignment(Qt::AlignCenter);
        layout->addWidget(desc);
        
        layout->addStretch();
        
        // 原始应用按钮
        QPushButton *originalBtn = new QPushButton("🏥 医院智慧客服系统\n（原始功能）", this);
        UIStyleManager::applyButtonStyle(originalBtn, "primary");
        originalBtn->setMinimumHeight(60);
        connect(originalBtn, &QPushButton::clicked, [this]() {
            m_selectedMode = OriginalApp;
            accept();
        });
        layout->addWidget(originalBtn);
        
        // 功能演示按钮
        QPushButton *demoBtn = new QPushButton("🎨 新功能演示\n（导航系统 + 聊天存储 + 样式管理）", this);
        UIStyleManager::applyButtonStyle(demoBtn, "secondary");
        demoBtn->setMinimumHeight(60);
        connect(demoBtn, &QPushButton::clicked, [this]() {
            m_selectedMode = DemoApp;
            accept();
        });
        layout->addWidget(demoBtn);
        
        layout->addStretch();
        
        // 退出按钮
        QPushButton *exitBtn = new QPushButton("❌ 退出", this);
        UIStyleManager::applyButtonStyle(exitBtn, "error");
        connect(exitBtn, &QPushButton::clicked, this, &QDialog::reject);
        layout->addWidget(exitBtn);
    }
    
    StartupMode getSelectedMode() const { return m_selectedMode; }

private:
    StartupMode m_selectedMode = OriginalApp;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("HospAI");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("HospAI Team");
    a.setOrganizationDomain("hospai.com");
    
    // 跨平台字体设置，会在 UIStyleManager 中进一步优化
    UIStyleManager::setupFonts();
    
    // 应用跨平台优化的全局样式
    UIStyleManager::applyGlobalStyleSheet(&a);
    
    // 初始化数据库
    DatabaseManager* dbManager = DatabaseManager::instance();
    if (!dbManager->initDatabase()) {
        QMessageBox::critical(nullptr, "数据库错误", 
                            "无法初始化数据库！\n应用程序将退出。");
        return -1;
    }
    
    qDebug() << "数据库初始化成功";
    
    // 显示登录对话框
    LoginDialog loginDialog;
    
    // 如果登录失败，退出应用程序
    if (loginDialog.exec() != QDialog::Accepted) {
        qDebug() << "用户取消登录，应用程序退出";
        return 0;
    }
    
    // 获取登录用户信息
    UserInfo currentUser = loginDialog.getLoggedInUser();
    qDebug() << "用户登录成功:" << currentUser.username << "角色:" << currentUser.role;
    
    // 根据用户角色创建相应的主窗口
    MainWindow w;
    w.setDatabaseManager(dbManager);
    w.setCurrentUser(currentUser);
    w.show();
    
    return a.exec();
}

#include "main.moc"
