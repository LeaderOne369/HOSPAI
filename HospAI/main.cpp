#include "src/views/common/RoleSelector.h"
#include "src/views/patient/PatientWindow.h"
#include "src/views/staff/StaffWindow.h"
#include "src/views/admin/AdminWindow.h"
#include "mainwindow.h"
#include "src/views/common/ExampleUsageWidget.h"
#include "src/views/common/UIStyleManager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QSysInfo>

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
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("HospAI");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Hospital AI Team");
    
    // 应用全局样式
    UIStyleManager::applyGlobalStyleSheet(&app);
    
    // 显示启动选择对话框
    StartupDialog startupDialog;
    if (startupDialog.exec() != QDialog::Accepted) {
        return 0; // 用户选择退出
    }
    
    if (startupDialog.getSelectedMode() == StartupDialog::DemoApp) {
        // 启动功能演示程序
        ExampleUsageWidget *example = new ExampleUsageWidget();
        example->setWindowTitle("🏥 HospAI - 新功能演示");
        example->resize(1200, 800);
        example->show();
        
        return app.exec();
    } else {
        // 启动原始的医院智慧客服系统
        app.setApplicationName("医院智慧客服系统");
        app.setApplicationVersion("1.0.0");
        app.setOrganizationName("医院信息科");
        
        // 显示角色选择器
        RoleSelector roleSelector;
        if (roleSelector.exec() != QDialog::Accepted) {
            return 0; // 用户取消选择
        }
        
        UserRole selectedRole = roleSelector.getSelectedRole();
        
        // 根据选择的角色创建相应的窗口
        BaseWindow* mainWindow = nullptr;
        
        switch (selectedRole) {
        case UserRole::Patient:
            mainWindow = new PatientWindow;
            break;
        case UserRole::Staff:
            mainWindow = new StaffWindow;
            break;
        case UserRole::Admin:
            mainWindow = new AdminWindow;
            break;
        }
        
        if (mainWindow) {
            mainWindow->show();
            
            // 连接退出登录信号
            QObject::connect(mainWindow, &BaseWindow::logoutRequested, [&]() {
                mainWindow->close();
                app.quit();
            });
            
            int result = app.exec();
            delete mainWindow;
            return result;
        }
    }
    
    return 0;
}

#include "main.moc"
